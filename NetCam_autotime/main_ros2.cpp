#include "infinite_sense.h"
#include "mv_cam.h"

#include "rclcpp/rclcpp.hpp"

#include <image_transport/image_transport.hpp>
#include <cv_bridge/cv_bridge.h>

#include <sensor_msgs/msg/imu.hpp>
#include <std_msgs/msg/u_int64.hpp>
#include <opencv2/opencv.hpp>
#include "nlohmann/json.hpp"
#include <fstream>

#define QUEUE_LENGTH 50

// 返回平均亮度（0..255）
double compute_mean_brightness(const cv::Mat &bgr) {
  cv::Mat y;
  cv::cvtColor(bgr, y, cv::COLOR_BGR2GRAY);      // 单通道亮度近似
  return cv::mean(y)[0];
}

// 或者计算中位数（更抗噪）
double compute_median_brightness(const cv::Mat &bgr) {
  cv::Mat y; cv::cvtColor(bgr, y, cv::COLOR_BGR2GRAY);
  y = y.reshape(0,1); // 展开为一行
  std::vector<uchar> v; v.assign(y.datastart, y.dataend);
  std::nth_element(v.begin(), v.begin() + v.size()/2, v.end());
  return v[v.size()/2];
}

// 快速分块采样亮度计算：将图像划分为 grid_rows x grid_cols 个块，
// 每个块只采样一个像素（块中心位置），计算这些采样点的灰度平均值。
// 目的：在场景亮度剧变时减少计算量，避免全图遍历带来的性能下降。
double compute_block_mean_brightness(const cv::Mat &img, int grid_rows = 6, int grid_cols = 6) {
  if (img.empty()) return 0.0;

  const int h = img.rows;
  const int w = img.cols;
  if (h <= 0 || w <= 0) return 0.0;

  grid_rows = std::max(1, grid_rows);
  grid_cols = std::max(1, grid_cols);

  // 块大小（整除取地板），最后一行/列使用边界保护
  const int block_h = std::max(1, h / grid_rows);
  const int block_w = std::max(1, w / grid_cols);

  double sum = 0.0;
  int count = 0;

  const bool is_gray = (img.type() == CV_8UC1);

  for (int r = 0; r < grid_rows; ++r) {
    int y = r * block_h + block_h / 2;
    if (y >= h) y = h - 1;
    for (int c = 0; c < grid_cols; ++c) {
      int x = c * block_w + block_w / 2;
      if (x >= w) x = w - 1;

      double gray = 0.0;
      if (is_gray) {
        gray = static_cast<unsigned char>(img.at<unsigned char>(y, x));
      } else if (img.type() == CV_8UC3) {
        const cv::Vec3b &bgr = img.at<cv::Vec3b>(y, x);
        // BGR -> 灰度（常用加权）
        gray = 0.299 * bgr[2] + 0.587 * bgr[1] + 0.114 * bgr[0];
      } else {
        // 其它类型回退为按通道读取（安全保障）
        if (img.channels() >= 3) {
          cv::Vec3b v = img.at<cv::Vec3b>(std::min(y, img.rows-1), std::min(x, img.cols-1));
          gray = 0.299 * v[2] + 0.587 * v[1] + 0.114 * v[0];
        } else {
          gray = static_cast<unsigned char>(img.at<unsigned char>(std::min(y, img.rows-1), std::min(x, img.cols-1)));
        }
      }

      sum += gray;
      ++count;
    }
  }

  if (count == 0) return 0.0;
  return sum / static_cast<double>(count);
}

// 分块采样，返回每个块中心处的灰度值数组（按行优先，大小 = grid_rows * grid_cols）
std::vector<uint8_t> compute_block_samples(const cv::Mat &img, int grid_rows = 6, int grid_cols = 6) {
  std::vector<uint8_t> out;
  if (img.empty()) return out;
  const int h = img.rows;
  const int w = img.cols;
  if (h <= 0 || w <= 0) return out;
  grid_rows = std::max(1, grid_rows);
  grid_cols = std::max(1, grid_cols);
  const int block_h = std::max(1, h / grid_rows);
  const int block_w = std::max(1, w / grid_cols);
  out.reserve(grid_rows * grid_cols);
  const bool is_gray = (img.type() == CV_8UC1);
  for (int r = 0; r < grid_rows; ++r) {
    int y = r * block_h + block_h / 2;
    if (y >= h) y = h - 1;
    for (int c = 0; c < grid_cols; ++c) {
      int x = c * block_w + block_w / 2;
      if (x >= w) x = w - 1;
      uint8_t gray = 0;
      if (is_gray) {
        gray = static_cast<uint8_t>(img.at<unsigned char>(y, x));
      } else if (img.type() == CV_8UC3) {
        const cv::Vec3b &bgr = img.at<cv::Vec3b>(y, x);
        double g = 0.299 * bgr[2] + 0.587 * bgr[1] + 0.114 * bgr[0];
        gray = static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(g)), 0, 255));
      } else {
        if (img.channels() >= 3) {
          cv::Vec3b v = img.at<cv::Vec3b>(std::min(y, img.rows-1), std::min(x, img.cols-1));
          double g = 0.299 * v[2] + 0.587 * v[1] + 0.114 * v[0];
          gray = static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(g)), 0, 255));
        } else {
          gray = static_cast<uint8_t>(img.at<unsigned char>(std::min(y, img.rows-1), std::min(x, img.cols-1)));
        }
      }
      out.push_back(gray);
    }
  }
  return out;
}

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

    // Load AE config (ideal_brightness, alpha, initial exposure)
    std::string config_path = "/home/xuhuiyao/Desktop/workfield/sync/SimpleSensorSync/camera_config.json";
    try {
      std::ifstream ifs(config_path);
      if (ifs.is_open()) {
        nlohmann::json j; ifs >> j;
        ideal_brightness_ = j.value("ideal_brightness", ideal_brightness_);
        alpha_ = j.value("alpha", alpha_);
        default_exposure_time_ = j.value("exposure_time", default_exposure_time_);
      }
    } catch (const std::exception &e) {
      RCLCPP_WARN(this->get_logger(), "Failed to read camera_config.json: %s", e.what());
    }

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
    // rclcpp::QoS qos_profile(QUEUE_LENGTH); // 匹配发布端的队列长度
    // qos_profile.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
    // qos_profile.durability(RMW_QOS_POLICY_DURABILITY_VOLATILE);
    // qos_profile.history(RMW_QOS_POLICY_HISTORY_KEEP_LAST);

    // if (cfg.CAM1.second >= 0) image_pubs_[cfg.CAM1.first] = transport_.advertise(cfg.CAM1.first, qos_profile.get_rmw_qos_profile());
    // if (cfg.CAM2.second >= 0) image_pubs_[cfg.CAM2.first] = transport_.advertise(cfg.CAM2.first, qos_profile.get_rmw_qos_profile());
    // if (cfg.CAM3.second >= 0) image_pubs_[cfg.CAM3.first] = transport_.advertise(cfg.CAM3.first, qos_profile.get_rmw_qos_profile());
    // if (cfg.CAM4.second >= 0) image_pubs_[cfg.CAM4.first] = transport_.advertise(cfg.CAM4.first, qos_profile.get_rmw_qos_profile());

    if (cfg.CAM1.second >= 0) image_pubs_[cfg.CAM1.first] = transport_.advertise(cfg.CAM1.first, QUEUE_LENGTH);
    if (cfg.CAM2.second >= 0) image_pubs_[cfg.CAM2.first] = transport_.advertise(cfg.CAM2.first, QUEUE_LENGTH);
    if (cfg.CAM3.second >= 0) image_pubs_[cfg.CAM3.first] = transport_.advertise(cfg.CAM3.first, QUEUE_LENGTH);
    if (cfg.CAM4.second >= 0) image_pubs_[cfg.CAM4.first] = transport_.advertise(cfg.CAM4.first, QUEUE_LENGTH);

    // create exposure publishers for each camera: topic <camera_name>_exposure
    if (cfg.CAM1.second >= 0) exposure_pubs_[cfg.CAM1.first] = this->create_publisher<std_msgs::msg::UInt64>(cfg.CAM1.first + "_exposure", 10);
    if (cfg.CAM2.second >= 0) exposure_pubs_[cfg.CAM2.first] = this->create_publisher<std_msgs::msg::UInt64>(cfg.CAM2.first + "_exposure", 10);
    if (cfg.CAM3.second >= 0) exposure_pubs_[cfg.CAM3.first] = this->create_publisher<std_msgs::msg::UInt64>(cfg.CAM3.first + "_exposure", 10);
    if (cfg.CAM4.second >= 0) exposure_pubs_[cfg.CAM4.first] = this->create_publisher<std_msgs::msg::UInt64>(cfg.CAM4.first + "_exposure", 10);

    synchronizer_.UseSensor(mv_cam_);
    synchronizer_.Start();

    // initialize current exposure map
    if (cfg.CAM1.second >= 0) current_exposure_us_[cfg.CAM1.first] = default_exposure_time_;
    if (cfg.CAM2.second >= 0) current_exposure_us_[cfg.CAM2.first] = default_exposure_time_;
    if (cfg.CAM3.second >= 0) current_exposure_us_[cfg.CAM3.first] = default_exposure_time_;
    if (cfg.CAM4.second >= 0) current_exposure_us_[cfg.CAM4.first] = default_exposure_time_;

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
    // cv::Mat resized_image;
    // cv::resize(original_image, resized_image, 
    //            cv::Size(original_image.cols * SCALE_FACTOR, 
    //                     original_image.rows * SCALE_FACTOR), 0, 0, cv::INTER_LINEAR);
    // cv::Mat denoise_image;
    // cv::fastNlMeansDenoisingColored(resized_image, denoise_image, 3, 3, 5, 7);
    // 转换为ROS消息（注意：使用与原始图像相同的编码）
    const sensor_msgs::msg::Image::SharedPtr image_msg = 
        cv_bridge::CvImage(header, "bgr8", original_image).toImageMsg();
    // publish exposure time on separate topic
    if (exposure_pubs_.find(cam_data->name) != exposure_pubs_.end()) {
      std_msgs::msg::UInt64 exp_msg;
      exp_msg.data = cam_data->exposure_time_us;
      exposure_pubs_[cam_data->name]->publish(exp_msg);
    }
    // 方案 B：只采样并把小样本包发到后台线程，由后台进行亮度计算和曝光下发
    if (mv_cam_) {
      infinite_sense::MvCam::ExposureSample s;
      s.name = cam_data->name;
      s.time_stamp_us = cam_data->time_stamp_us;
      s.grid_rows = 6;
      s.grid_cols = 6;
      s.samples = compute_block_samples(original_image, s.grid_rows, s.grid_cols);
      mv_cam_->EnqueueSample(s);
    }
    auto it_img = image_pubs_.find(cam_data->name);
    if (it_img != image_pubs_.end()) {
      it_img->second.publish(image_msg);
    } else {
      RCLCPP_WARN(this->get_logger(), "No image publisher for camera '%s'", cam_data->name.c_str());
    }
  }


 private:
  infinite_sense::Synchronizer synchronizer_;
  SharedPtr node_handle_;
  image_transport::ImageTransport transport_;
  std::shared_ptr<MvCam> mv_cam_;
  std::unordered_map<std::string, image_transport::Publisher> image_pubs_;
  std::unordered_map<std::string, rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr> exposure_pubs_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  std::unordered_map<std::string, double> current_exposure_us_;
  double ideal_brightness_ = 120.0;
  double alpha_ = 0.3;
  double default_exposure_time_ = 30000.0;
};

int main(const int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CamDriver>());
  return 0;
}
