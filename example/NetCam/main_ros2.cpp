#include "infinite_sense.h"
#include "mv_cam.h"

#include "rclcpp/rclcpp.hpp"

#include <image_transport/image_transport.hpp>
#include <cv_bridge/cv_bridge.h>

#include <sensor_msgs/msg/imu.hpp>

#define QUEUE_LENGTH 10
#define SCALE_FACTOR 0.5

struct cam_config {
    bool onboard_imu = true;// 使用内部IMU
    bool extern_imu = false;// 使用外部IMU,TODO 
    std::string imu_name = "onboard_imu";

    // 相机名称 + 设备ID
    std::pair<std::string, int> CAM1 = {"cam_1", 1};
    std::pair<std::string, int> CAM2 = {"cam_2", 1};
    std::pair<std::string, int> CAM3 = {"cam_3", 1};
    std::pair<std::string, int> CAM4 = {"cam_4", -1}; // -1 代表没有

    // 通信方式
    int type = 0; // 0 --> 串口通信  1 --> 网口通信
    std::string uart_dev = "/dev/ttyACM0";
    std::string net_ip = "192.168.1.188";
    int net_port = 8888;
};
cam_config cfg;

using namespace infinite_sense;

class CamDriver final : public rclcpp::Node {
 public:
  CamDriver()
      : Node("ros2_cam_driver"), node_handle_(std::shared_ptr<CamDriver>(this, [](auto *) {})), transport_(node_handle_) {

    if (cfg.type == 0) {
      synchronizer_.SetUsbLink(cfg.uart_dev, 921600);
    } else if (cfg.type == 1) {
      synchronizer_.SetNetLink(cfg.net_ip, cfg.net_port);
    }

    mv_cam_ = std::make_shared<infinite_sense::MvCam>();

    // if (cfg.CAM1.second >= 0) mv_cam_->SetParams({{cfg.CAM1.first, CAM_1}});
    // if (cfg.CAM2.second >= 0) mv_cam_->SetParams({{cfg.CAM2.first, CAM_2}});
    // if (cfg.CAM3.second >= 0) mv_cam_->SetParams({{cfg.CAM3.first, CAM_3}});
    // if (cfg.CAM4.second >= 0) mv_cam_->SetParams({{cfg.CAM4.first, CAM_4}});

    if (cfg.CAM1.second >= 0) mv_cam_->SetParams({{cfg.CAM1.first, CAM_1}, {cfg.CAM2.first, CAM_2}, {cfg.CAM3.first, CAM_3}});

    // 显示节点中
    rclcpp::QoS qos_profile(QUEUE_LENGTH); // 匹配发布端的队列长度
    qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE);
    qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
    qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);

    if (cfg.CAM1.second >= 0) image_pubs_[cfg.CAM1.first] = transport_.advertise(cfg.CAM1.first, qos_profile.get_rmw_qos_profile());
    if (cfg.CAM2.second >= 0) image_pubs_[cfg.CAM2.first] = transport_.advertise(cfg.CAM2.first, qos_profile.get_rmw_qos_profile());
    if (cfg.CAM3.second >= 0) image_pubs_[cfg.CAM3.first] = transport_.advertise(cfg.CAM3.first, qos_profile.get_rmw_qos_profile());
    if (cfg.CAM4.second >= 0) image_pubs_[cfg.CAM4.first] = transport_.advertise(cfg.CAM4.first, qos_profile.get_rmw_qos_profile());

    synchronizer_.UseSensor(mv_cam_);
    synchronizer_.Start();

    if (cfg.onboard_imu) {
      imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>(cfg.imu_name, 10);
      Messenger::GetInstance().SubStruct(
          cfg.imu_name, std::bind(&CamDriver::ImuCallback, this, std::placeholders::_1, std::placeholders::_2));
    }

    {
      using namespace std::placeholders;
      if (cfg.onboard_imu) {
          Messenger::GetInstance().SubStruct(
          cfg.imu_name, std::bind(&CamDriver::ImuCallback, this, std::placeholders::_1, std::placeholders::_2));
      }

      if (cfg.CAM1.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM1.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
      if (cfg.CAM2.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM2.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
      if (cfg.CAM3.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM3.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
      if (cfg.CAM4.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM4.first, std::bind(&CamDriver::ImageCallback, this, std::placeholders::_1, std::placeholders::_2));
    }
  }

  void ImuCallback(const void *msg, size_t) const {
    const auto *imu_data = static_cast<const infinite_sense::ImuData *>(msg);
    sensor_msgs::msg::Imu imu_msg;
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

  void ImageCallback(const void *msg, size_t) {
    const auto *cam_data = static_cast<const infinite_sense::CamData *>(msg);
    std_msgs::msg::Header header;
    header.stamp = rclcpp::Time(cam_data->time_stamp_us);
    header.frame_id = "map";
    // const cv::Mat image_mat(cam_data->image.rows, cam_data->image.cols, CV_8UC1, cam_data->image.data);
    // const sensor_msgs::msg::Image::SharedPtr image_msg = cv_bridge::CvImage(header, "mono8", image_mat).toImageMsg();
    // const cv::Mat image_mat(cam_data->image.rows, cam_data->image.cols, CV_8UC3, cam_data->image.data);
    // const sensor_msgs::msg::Image::SharedPtr image_msg = cv_bridge::CvImage(header, "bgr8", image_mat).toImageMsg();
    // 原始图像
    cv::Mat original_image(cam_data->image.rows, cam_data->image.cols, CV_8UC3, cam_data->image.data);
    // 创建下采样图像容器
    cv::Mat resized_image;
    cv::resize(original_image, resized_image, 
               cv::Size(original_image.cols * SCALE_FACTOR, 
                        original_image.rows * SCALE_FACTOR));
    // 转换为ROS消息（注意：使用与原始图像相同的编码）
    const sensor_msgs::msg::Image::SharedPtr image_msg = 
        cv_bridge::CvImage(header, "bgr8", resized_image).toImageMsg();
    image_pubs_[cam_data->name].publish(image_msg);
  }


 private:
  infinite_sense::Synchronizer synchronizer_;
  SharedPtr node_handle_;
  image_transport::ImageTransport transport_;
  std::shared_ptr<MvCam> mv_cam_;
  std::unordered_map<std::string, image_transport::Publisher> image_pubs_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
};

int main(const int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CamDriver>());
  return 0;
}
