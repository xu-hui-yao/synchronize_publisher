// 加入SDK头文件
#include "infinite_sense.h"
// 加入工业相机头文件
#include "video_cam.h"
using namespace infinite_sense;

// TODO: 请按需要修改如下配置
struct video_config {
    bool onboard_imu = true;

    // 相机名称 + 设备ID
    std::pair<std::string, int> CAM3_Video = {"left_cam", 0};
    std::pair<std::string, int> CAM4_Video = {"right_cam", 1};
    std::pair<std::string, int> CAM1_Video = {"front_cam", 2};
    std::pair<std::string, int> CAM2_Video = {"rear_cam", -1}; // -1 代表没有
};


// 自定义回调函数
void ImuCallback(const void *msg, size_t) {
  const auto *imu_data = static_cast<const ImuData *>(msg);
  LOG(INFO) << imu_data->time_stamp_us << " " << "Accel: " << imu_data->a[0] << " " << imu_data->a[1] << " "
            << imu_data->a[2] << " Gyro: " << imu_data->g[0] << " " << imu_data->g[1] << " " << imu_data->g[2]
            << " Temp: " << imu_data->temperature;
  // 处理IMU数据
}

// 自定义回调函数
void ImageCallback(const void *msg, size_t) {
  const auto *cam_data = static_cast<const CamData *>(msg);
  // 处理图像数据
}

int main(int argc, char* argv[]) {
  // 1.创建同步器
  Synchronizer synchronizer;

  if (argc < 2) {
    // 默认使用串口
    std::cout << "未传入参数，默认使用 USB 串口 /dev/ttyACM0" << std::endl;
    synchronizer.SetUsbLink("/dev/ttyACM0", 921600);
  } else {
    std::string mode = argv[1];
    if (mode == "uart") {
      std::cout << "参数为 uart，使用 USB 串口 /dev/ttyACM0" << std::endl;
      synchronizer.SetUsbLink("/dev/ttyACM0", 921600);
    } else if (mode == "net") {
      std::cout << "参数为 net，使用网口 192.168.1.188:8888" << std::endl;
      synchronizer.SetNetLink("192.168.1.188", 8888);
    } else {
      std::cerr << "未知参数: " << mode 
                << "，请使用 uart / net 或不传参数" << std::endl;
      return -1;
    }
  }

  video_config cfg;
  // 把配置转成 cam_list，过滤掉 device_id = -1 的
  std::vector<std::pair<std::string, int>> cam_list;
  if (cfg.CAM1_Video.second >= 0) cam_list.push_back(cfg.CAM1_Video);
  if (cfg.CAM2_Video.second >= 0) cam_list.push_back(cfg.CAM2_Video);
  if (cfg.CAM3_Video.second >= 0) cam_list.push_back(cfg.CAM3_Video);
  if (cfg.CAM4_Video.second >= 0) cam_list.push_back(cfg.CAM4_Video);

  // 2.配置同步接口
  auto video_cam = std::make_shared<VideoCam>(cam_list);
  // 设置触发参数（如果有的话）
  // 这里用相机名作为 key
  if (cfg.CAM1_Video.second >= 0) video_cam->SetParams({{cfg.CAM1_Video.first, CAM_1}});
  if (cfg.CAM2_Video.second >= 0) video_cam->SetParams({{cfg.CAM2_Video.first, CAM_2}});
  if (cfg.CAM3_Video.second >= 0) video_cam->SetParams({{cfg.CAM3_Video.first, CAM_3}});
  if (cfg.CAM4_Video.second >= 0) video_cam->SetParams({{cfg.CAM4_Video.first, CAM_4}});

  synchronizer.UseSensor(video_cam);

  // 3.开启同步
  synchronizer.Start();

  // 4.接收数据
  if (cfg.onboard_imu) {
    Messenger::GetInstance().SubStruct("imu_1", ImuCallback);
  }
  
  if (cfg.CAM1_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM1_Video.first, ImageCallback);
  if (cfg.CAM2_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM2_Video.first, ImageCallback);
  if (cfg.CAM3_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM3_Video.first, ImageCallback);
  if (cfg.CAM4_Video.second >= 0) Messenger::GetInstance().SubStruct(cfg.CAM4_Video.first, ImageCallback);

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  // 5.停止同步
  synchronizer.Stop();
  return 0;
}
