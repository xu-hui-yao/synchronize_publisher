#include "infinite_sense.h"
#include "mv_cam.h"

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Imu.h>


struct cam_config {
    bool onboard_imu = true;// 使用内部IMU
    bool extern_imu = false;// 使用外部IMU,TODO 
    std::string imu_name = "onboard_imu";

    // 相机名称 + 设备ID
    std::pair<std::string, int> CAM1 = {"cam_1", 1};
    std::pair<std::string, int> CAM2 = {"cam_2", -1};
    std::pair<std::string, int> CAM3 = {"cam_3", -1};
    std::pair<std::string, int> CAM4 = {"cam_4", -1}; // -1 代表没有

    // 通信方式
    int type = 0; // 0 --> 串口通信  1 --> 网口通信
    std::string uart_dev = "/dev/ttyACM0";
    std::string net_ip = "192.168.1.188";
    int net_port = 8888;
};
cam_config cfg;

using namespace infinite_sense;
inline ros::Time CreateRosTimestamp(const uint64_t mico_sec) {
  uint32_t nsec_per_second = 1e9;
  auto u64 = mico_sec * 1000;
  uint32_t sec = u64 / nsec_per_second;
  uint32_t nsec = u64 - (sec * nsec_per_second);
  return {sec, nsec};
}

class CamDriver {
 public:
  CamDriver(ros::NodeHandle &nh) : node_(nh), it_(node_), camera_name_("cam_1"), imu_name_("imu_1") {}
  void ImuCallback(const void *msg, size_t) {
    const auto *imu_data = static_cast<const ImuData *>(msg);
    sensor_msgs::Imu imu_msg_data;
    imu_msg_data.header.frame_id = "map";
    imu_msg_data.header.stamp = CreateRosTimestamp(imu_data->time_stamp_us);

    imu_msg_data.angular_velocity.x = imu_data->g[0];
    imu_msg_data.angular_velocity.y = imu_data->g[1];
    imu_msg_data.angular_velocity.z = imu_data->g[2];

    imu_msg_data.linear_acceleration.x = imu_data->a[0];
    imu_msg_data.linear_acceleration.y = imu_data->a[1];
    imu_msg_data.linear_acceleration.z = imu_data->a[2];

    imu_msg_data.orientation.w = imu_data->q[0];
    imu_msg_data.orientation.x = imu_data->q[1];
    imu_msg_data.orientation.y = imu_data->q[2];
    imu_msg_data.orientation.z = imu_data->q[3];
    imu_pub_.publish(imu_msg_data);
  }

  // 自定义回调函数
  void ImageCallback(const void *msg, size_t) {
    const auto *cam_data = static_cast<const CamData *>(msg);
    const cv::Mat image_mat(cam_data->image.rows, cam_data->image.cols, CV_8UC1, cam_data->image.data);
    sensor_msgs::ImagePtr image_msg =
        // mono8:灰度类型,bgr8:彩图，具体需要根据相机类型进行修改
        cv_bridge::CvImage(std_msgs::Header(), "mono8", image_mat).toImageMsg();
    image_msg->header.stamp = CreateRosTimestamp(cam_data->time_stamp_us);
    // 发布曝光时间到单独 topic
    if (exposure_pubs_.find(cam_data->name) != exposure_pubs_.end()) {
      std_msgs::UInt64 exp_msg;
      exp_msg.data = cam_data->exposure_time_us;
      exposure_pubs_[cam_data->name].publish(exp_msg);
    }
    auto it_img = image_pubs_.find(cam_data->name);
    if (it_img != image_pubs_.end()) {
      it_img->second.publish(image_msg);
    } else {
      ROS_WARN("No image publisher for camera '%s'", cam_data->name.c_str());
    }
  }
  void Init() {
    if (cfg.type == 0) {
      synchronizer_.SetUsbLink(cfg.uart_dev, 921600);
    } else if (cfg.type == 1) {
      synchronizer_.SetNetLink(cfg.net_ip, cfg.net_port);
    }
  
    mv_cam_ = std::make_shared<MvCam>();
    synchronizer_.UseSensor(mv_cam_);

    if (cfg.onboard_imu) {
      imu_pub_ = node_.advertise<sensor_msgs::Imu>(cfg.imu_name, 1000);
    }

    if (cfg.CAM1.second >= 0) mv_cam_->SetParams({{cfg.CAM1.first, CAM_1}});
    if (cfg.CAM2.second >= 0) mv_cam_->SetParams({{cfg.CAM2.first, CAM_2}});
    if (cfg.CAM3.second >= 0) mv_cam_->SetParams({{cfg.CAM3.first, CAM_3}});
    if (cfg.CAM4.second >= 0) mv_cam_->SetParams({{cfg.CAM4.first, CAM_4}});


    if (cfg.CAM1.second >= 0) image_pubs_[cfg.CAM1.first] = it_.advertise(cfg.CAM1.first, 30);
    if (cfg.CAM2.second >= 0) image_pubs_[cfg.CAM2.first] = it_.advertise(cfg.CAM2.first, 30);
    if (cfg.CAM3.second >= 0) image_pubs_[cfg.CAM3.first] = it_.advertise(cfg.CAM3.first, 30);
    if (cfg.CAM4.second >= 0) image_pubs_[cfg.CAM4.first] = it_.advertise(cfg.CAM4.first, 30);

    synchronizer_.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    if (cfg.onboard_imu) {
      Messenger::GetInstance().SubStruct(
          cfg.imu_name, std::bind(&CamDriver::ImuCallback, this, std::placeholders::_1, std::placeholders::_2));
    }

    if (cfg.CAM1.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM1.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
    if (cfg.CAM2.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM2.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
    if (cfg.CAM3.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM3.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
    if (cfg.CAM4.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM4.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
  }

  void Run() {
    ros::Rate loop_rate(1000);
    while (ros::ok()) {
      ros::spinOnce();
      loop_rate.sleep();
    }
    synchronizer_.Stop();
  }

 private:
  ros::NodeHandle &node_;
  ros::Publisher imu_pub_;
  image_transport::ImageTransport it_;
  std::unordered_map<std::string, image_transport::Publisher> image_pubs_;
  std::shared_ptr<MvCam> mv_cam_;
  Synchronizer synchronizer_;
  std::string camera_name_;
  std::string imu_name_;
};

int main(int argc, char **argv) {
  ros::init(argc, argv, "ros_demo", ros::init_options::NoSigintHandler);
  ros::NodeHandle node;
  CamDriver demo_node(node);
  demo_node.Init();
  demo_node.Run();

  return 0;
}