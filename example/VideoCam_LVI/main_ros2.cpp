#include "infinite_sense.h"
#include "video_cam.h"

#include "rclcpp/rclcpp.hpp"

#include <image_transport/image_transport.hpp>
#include <cv_bridge/cv_bridge.h>

#include <sensor_msgs/msg/imu.hpp>


struct video_config {
    bool onboard_imu = true;// 使用内部IMU
    bool extern_imu = false;// 使用外部IMU,TODO 
    std::string imu_name = "imu_1";

    // 相机名称 + 设备ID
    std::pair<std::string, int> CAM3_Video = {"left_cam",  0};
    std::pair<std::string, int> CAM4_Video = {"right_cam", -1};
    std::pair<std::string, int> CAM1_Video = {"front_cam", -1};
    std::pair<std::string, int> CAM2_Video = {"rear_cam", -1}; // -1 代表没有

    // 通信方式
    int type = 0; // 0 --> 串口通信  1 --> 网口通信
    std::string uart_dev = "/dev/ttyACM0";
    std::string net_ip = "192.168.1.188";
    int net_port = 8888;
};
video_config cfg;

using namespace infinite_sense;

// 继承自 rclcpp::Nod
class CamDriver final : public rclcpp::Node {
 public:
  CamDriver()
      : Node("ros2_video_demo"), node_handle_(std::shared_ptr<CamDriver>(this, [](auto *) {})), transport_(node_handle_){
    
    // 相机通信方式
    if (cfg.type == 0) {
      synchronizer_.SetUsbLink(cfg.uart_dev, 921600);
    } else if (cfg.type == 1) {
      synchronizer_.SetNetLink(cfg.net_ip, cfg.net_port);
    }

    std::vector<std::pair<std::string, int>> cam_list;
    if (cfg.CAM1_Video.second >= 0) cam_list.push_back(cfg.CAM1_Video);
    if (cfg.CAM2_Video.second >= 0) cam_list.push_back(cfg.CAM2_Video);
    if (cfg.CAM3_Video.second >= 0) cam_list.push_back(cfg.CAM3_Video);
    if (cfg.CAM4_Video.second >= 0) cam_list.push_back(cfg.CAM4_Video);

    video_cam_ = std::make_shared<VideoCam>(cam_list);
    synchronizer_.UseSensor(video_cam_);

    if (cfg.onboard_imu) {
      imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>(cfg.imu_name, 1000);
    }

    if (cfg.CAM1_Video.second >= 0) image_pubs_[cfg.CAM1_Video.first] = transport_.advertise(cfg.CAM1_Video.first, 30);
    if (cfg.CAM2_Video.second >= 0) image_pubs_[cfg.CAM2_Video.first] = transport_.advertise(cfg.CAM2_Video.first, 30);
    if (cfg.CAM3_Video.second >= 0) image_pubs_[cfg.CAM3_Video.first] = transport_.advertise(cfg.CAM3_Video.first, 30);
    if (cfg.CAM4_Video.second >= 0) image_pubs_[cfg.CAM4_Video.first] = transport_.advertise(cfg.CAM4_Video.first, 30);
    
    // 设置触发参数（如果有的话）
    // 这里用相机名作为 key
    if (cfg.CAM1_Video.second >= 0) video_cam_->SetParams({{cfg.CAM1_Video.first, CAM_1}});
    if (cfg.CAM2_Video.second >= 0) video_cam_->SetParams({{cfg.CAM2_Video.first, CAM_2}});
    if (cfg.CAM3_Video.second >= 0) video_cam_->SetParams({{cfg.CAM3_Video.first, CAM_3}});
    if (cfg.CAM4_Video.second >= 0) video_cam_->SetParams({{cfg.CAM4_Video.first, CAM_4}});

    // 逐个调用开始函数 
    synchronizer_.Start();


    // SDK 获取IMU和图像信息之后的回调函数 
    {
      Messenger::GetInstance().SubStruct(
        cfg.imu_name, std::bind(&CamDriver::ImuCallback, this, std::placeholders::_1, std::placeholders::_2));

      if (cfg.CAM1_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM1_Video.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
      if (cfg.CAM2_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM2_Video.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
      if (cfg.CAM3_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM3_Video.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
      if (cfg.CAM4_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM4_Video.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
    }
  }

  // SDK 获取IMU信息后的回调函数 
  void ImuCallback(const void *msg, size_t) const {
    // 按照ROS2的格式发布的IMU数据
    const auto *imu_data = static_cast<const infinite_sense::ImuData *>(msg);
    sensor_msgs::msg::Imu imu_msg;
    // 得到的时间单位us,需要转换为ms
    imu_msg.header.stamp = rclcpp::Time(imu_data->time_stamp_us * 1000);
    imu_msg.header.frame_id = "map";
    imu_msg.linear_acceleration.x = imu_data->a[0];
    imu_msg.linear_acceleration.y = imu_data->a[1];
    imu_msg.linear_acceleration.z = imu_data->a[2];
    imu_msg.angular_velocity.x = imu_data->g[0];
    imu_msg.angular_velocity.y = imu_data->g[1];
    imu_msg.angular_velocity.z = imu_data->g[2];
    imu_msg.orientation.w = imu_data->q[0];
    imu_msg.orientation.x = imu_data->q[1];
    imu_msg.orientation.y = imu_data->q[2];
    imu_msg.orientation.z = imu_data->q[3];
    imu_pub_->publish(imu_msg);
  }

  // SDK 获取图像信息后的回调函数 
  void ImageCallback(const void *msg, size_t) {
    // 原始的图像数据
    const auto *cam_data = static_cast<const infinite_sense::CamData *>(msg);
    std_msgs::msg::Header header;
    // 得到的时间单位us,需要转换为ms
    header.stamp = rclcpp::Time(cam_data->time_stamp_us * 1000);
    header.frame_id = "map";
    // 构造opencv图像，这里是彩色图片因此是 CV_8UC3 和  bgr8
    const cv::Mat image_mat(cam_data->image.rows, cam_data->image.cols, CV_8UC3, cam_data->image.data);
    sensor_msgs::msg::Image::SharedPtr image_msg = cv_bridge::CvImage(header, "bgr8", image_mat).toImageMsg();
    image_pubs_[cam_data->name].publish(image_msg);
  }


 private:
  infinite_sense::Synchronizer synchronizer_;
  SharedPtr node_handle_;
  image_transport::ImageTransport transport_;
  std::unordered_map<std::string, image_transport::Publisher> image_pubs_;
  std::shared_ptr<VideoCam> video_cam_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
};

// main 函数 
int main(const int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  // 构造 CamDriver 类，用于读取相机、IMU等信息
  rclcpp::spin(std::make_shared<CamDriver>());
  return 0;
}
