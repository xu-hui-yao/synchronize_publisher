#include "mv_cam.h"
#include "infinite_sense.h"
#include "MvCameraControl.h"
#include <opencv2/opencv.hpp>
#include <sys/mman.h>   // mmap, PROT_READ, PROT_WRITE, MAP_SHARED 等
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      // open, O_RDWR, O_CREAT 等
#include <unistd.h>     // close, ftruncate
#include "nlohmann/json.hpp"
#include <fstream>

namespace infinite_sense {

// TODO1: 定义时间戳数据，文件开头调用
struct time_stamp {
  int64_t high;
  int64_t low;
};
time_stamp *pointt;

bool IsColor(const MvGvspPixelType type) {
  switch (type) {
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed:
      return true;
    default:
      return false;
  }
}
bool IsMono(const MvGvspPixelType type) {
  switch (type) {
    case PixelType_Gvsp_Mono8:
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_Mono10_Packed:
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_Mono12_Packed:
      return true;
    default:
      return false;
  }
}
bool IsBayerGB8(const MvGvspPixelType type) {
  switch (type) {
    case PixelType_Gvsp_BayerGB8:
      return true;
    default:
      return false;
  }
}
bool IsBayerRG8(const MvGvspPixelType type) {
  switch (type) {
    case PixelType_Gvsp_BayerRG8:
      return true;
    default:
      return false;
  }
}


bool PrintDeviceInfo(const MV_CC_DEVICE_INFO *info) {
  if (info == nullptr) {
    LOG(WARNING) << "[WARNING] Failed to get camera details. Skipping...";
    return false;
  }
  LOG(INFO) << "-------------------------------------------------------------";
  LOG(INFO) << "----------------------Camera Device Info---------------------";
  if (info->nTLayerType == MV_GIGE_DEVICE) {
    const unsigned int n_ip1 = ((info->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
    const unsigned int n_ip2 = ((info->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
    const unsigned int n_ip3 = ((info->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
    const unsigned int n_ip4 = (info->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
    LOG(INFO) << "  Device Model Name    : " << info->SpecialInfo.stGigEInfo.chModelName;
    LOG(INFO) << "  Current IP Address   : " << n_ip1 << "." << n_ip2 << "." << n_ip3 << "." << n_ip4;
    LOG(INFO) << "  User Defined Name    : " << info->SpecialInfo.stGigEInfo.chUserDefinedName;
  } else if (info->nTLayerType == MV_USB_DEVICE) {
    LOG(INFO) << "  Device Model Name    : " << info->SpecialInfo.stUsb3VInfo.chModelName;
    LOG(INFO) << "  User Defined Name    : " << info->SpecialInfo.stUsb3VInfo.chUserDefinedName;
  } else {
    LOG(ERROR) << "[ERROR] Unsupported camera type!";
    return false;
  }
  LOG(INFO) << "-------------------------------------------------------------";
  return true;
}

MvCam::~MvCam() { Stop(); }

// bool MvCam::Initialization() {

//   // TODO2: 获取共享内存文件指针，相机类初始化时调用
// 	const char *user_name = getlogin();
// 	std::string path_for_time_stamp = "/home/" + std::string(user_name) + "/timeshare";
// 	const char *shared_file_name = path_for_time_stamp.c_str();

// 	int fd = open(shared_file_name, O_RDWR);

// 	pointt = (time_stamp *)mmap(NULL, sizeof(time_stamp), PROT_READ | PROT_WRITE,
// 	                            MAP_SHARED, fd, 0);

//   int n_ret = MV_OK;
//   MV_CC_DEVICE_INFO_LIST st_device_list{};
//   memset(&st_device_list, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
//   n_ret = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &st_device_list);
//   if (MV_OK != n_ret) {
//     LOG(ERROR) << "MV_CC_EnumDevices fail! n_ret [" << n_ret << "]";
//   }
//   if (st_device_list.nDeviceNum > 0) {
//     for (size_t i = 0; i < st_device_list.nDeviceNum; i++) {
//       MV_CC_DEVICE_INFO *p_device_info = st_device_list.pDeviceInfo[i];
//       if (nullptr == p_device_info) {
//         continue;
//       }
//       PrintDeviceInfo(p_device_info);
//     }
//   } else {
//     LOG(ERROR) << "Not find GIGE or USB Cam Devices!";
//     return false;
//   }
//   LOG(INFO) << "Number of cameras detected : " << st_device_list.nDeviceNum;
//   for (size_t i = 0; i < st_device_list.nDeviceNum; i++) {
//     handles_.emplace_back(nullptr);
//     rets_.push_back(MV_OK);
//   }
//   // 一共找到多少个设备,分别对所有的设备进行初始化
//   for (unsigned int i = 0; i < st_device_list.nDeviceNum; ++i) {
//     // 检测到有多少个相机要全部进行初始化。
//     // 选择设备并创建句柄
//     rets_[i] = MV_CC_CreateHandleWithoutLog(&handles_[i], st_device_list.pDeviceInfo[i]);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_CreateHandle fail! n_ret [" << rets_[i] << "]";
//     }
//     // open device
//     rets_[i] = MV_CC_OpenDevice(handles_[i]);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_OpenDevice fail! n_ret [" << rets_[i] << "]";
//     }
//     // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal
//     if (st_device_list.pDeviceInfo[i]->nTLayerType == MV_GIGE_DEVICE) {
//       int n_packet_size = MV_CC_GetOptimalPacketSize(handles_[i]);
//       if (n_packet_size > 0) {
//         rets_[i] = MV_CC_SetIntValue(handles_[i], "GevSCPSPacketSize", n_packet_size);
//         if (rets_[i] != MV_OK) {
//           LOG(WARNING) << "Set Packet Size fail n_ret [0x" << std::hex << rets_[i] << "]";
//         }
//       } else {
//         LOG(WARNING) << "Get Packet Size fail n_ret [0x" << std::hex << n_packet_size << "]";
//       }
//     }
//     // 设置触发模式为off
//     // rets_[i] = MV_CC_SetEnumValue(handles_[i], "TriggerMode", 1);
//     // if (MV_OK != rets_[i]) {
//     //   printf("MV_CC_SetTriggerMode fail! n_ret [%x]\n", rets_[i]);
//     // }
//     // 设置SDK内部图像缓存节点个数，大于等于1，在抓图前调用 1-30个缓存
//     rets_[i] = MV_CC_SetImageNodeNum(handles_[i], 100);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetImageNodeNum fail! n_ret [" << rets_[i] << "]";
//     }
//     // start grab image
//     rets_[i] = MV_CC_StartGrabbing(handles_[i]);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_StartGrabbing fail! n_ret [" << rets_[i] << "]";
//     }
    
//     int exposure_mode = 0;
//     float exposure_time = 10000;
//     int gain_mode = 0;
//     float gain = 6.0;
//     int balance_mode = 0;
//     int balance_ratio_red = 1427;
//     int balance_ratio_green = 1024;
//     int balance_ratio_blue = 1404;
//     // int saturation = 128;
//     float gamma = 0.7;

//     rets_[i] = MV_CC_SetExposureAutoMode(handles_[i], exposure_mode);  //0：off 1：once 2：Continuous
//     printf("Set Auto Exposure Mode to 0\n");
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetExposureAutoMode fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetExposureTime(handles_[i], exposure_time);
//     printf("Set Auto Exposure Time to %f\n", exposure_time);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetExposureTime fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetGainMode(handles_[i], gain_mode);
//     printf("Set Gain Mode to %d\n", gain_mode);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetGainMode fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetGain(handles_[i], gain);
//     printf("Set Gain to %f\n", gain);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetGain fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetBalanceWhiteAuto(handles_[i], balance_mode);
//     printf("Set Balance White Auto to %d\n", balance_mode);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetBalanceWhiteAuto fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetBalanceRatioRed(handles_[i], balance_ratio_red);
//     printf("Set Balance Ratio Red to %d\n", balance_ratio_red);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetBalanceRatioRed fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetBalanceRatioGreen(handles_[i], balance_ratio_green);
//     printf("Set Balance Ratio Green to %d\n", balance_ratio_green);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetBalanceRatioGreen fail! n_ret [" << rets_[i] << "]";
//     }

//     rets_[i] = MV_CC_SetBalanceRatioBlue(handles_[i], balance_ratio_blue);
//     printf("Set Balance Ratio Blue to %d\n", balance_ratio_blue);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetBalanceRatioBlue fail! n_ret [" << rets_[i] << "]";
//     }

//     // rets_[i] = MV_CC_SetSaturation(handles_[i], saturation);
//     // printf("Set Saturation to %d\n", saturation);
//     // if (MV_OK != rets_[i]) {
//     //   LOG(ERROR) << "MV_CC_SetSaturation fail! n_ret [" << rets_[i] << "]";
//     // }

//     rets_[i] = MV_CC_SetGamma(handles_[i], gamma);
//     printf("Set Gamma to %f\n", gamma);
//     if (MV_OK != rets_[i]) {
//       LOG(ERROR) << "MV_CC_SetGamma fail! n_ret [" << rets_[i] << "]";
//     }

//     LOG(INFO) << "Camera " << i << " opens to completion.";
//   }
//   if (0 == st_device_list.nDeviceNum) {
//     return true;
//   } else {
//     return false;
//   }
// }

bool MvCam::Initialization() {
    // === Step 1: 加载配置文件 ===
    std::string config_path = "/home/xuhuiyao/Desktop/workfield/sync/SimpleSensorSync/camera_config.json";
    
    CameraParams params;
    try {
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            LOG(ERROR) << "Cannot open camera config file: " << config_path;
            return false;
        }
        nlohmann::json j;
        config_file >> j;

        // 从 JSON 读取参数，使用 value() 提供默认值（兼容性好）
        params.exposure_mode = j.value("exposure_mode", 0);
        params.exposure_time = j.value("exposure_time", 10000.0f);
        params.gain_mode = j.value("gain_mode", 0);
        params.gain = j.value("gain", 6.0f);
        params.balance_mode = j.value("balance_mode", 0);
        params.balance_ratio_red = j.value("balance_ratio_red", 1427);
        params.balance_ratio_green = j.value("balance_ratio_green", 1024);
        params.balance_ratio_blue = j.value("balance_ratio_blue", 1404);
        params.gamma = j.value("gamma", 0.7f);
        // params.saturation = j.value("saturation", 128);

        LOG(INFO) << "Loaded camera parameters from: " << config_path;
    } catch (const std::exception& e) {
        LOG(ERROR) << "Failed to parse config file: " << e.what();
        return false;
    }

    // === Step 2: 共享内存初始化（原逻辑不变）===
    const char *user_name = getlogin();
    std::string path_for_time_stamp = "/home/" + std::string(user_name) + "/timeshare";
    int fd = open(path_for_time_stamp.c_str(), O_RDWR);
    if (fd == -1) {
        LOG(ERROR) << "Failed to open shared memory file";
        return false;
    }
    pointt = (time_stamp*)mmap(NULL, sizeof(time_stamp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pointt == MAP_FAILED) {
        LOG(ERROR) << "mmap failed";
        close(fd);
        return false;
    }

    // === Step 3: 相机枚举与初始化（原逻辑）===
    int n_ret = MV_OK;
    MV_CC_DEVICE_INFO_LIST st_device_list{};
    memset(&st_device_list, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    n_ret = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &st_device_list);
    if (MV_OK != n_ret) {
        LOG(ERROR) << "MV_CC_EnumDevices fail! n_ret [" << n_ret << "]";
        return false;
    }
    if (st_device_list.nDeviceNum == 0) {
        LOG(ERROR) << "No GIGE or USB Cam Devices found!";
        return false;
    }

    LOG(INFO) << "Number of cameras detected: " << st_device_list.nDeviceNum;
    handles_.resize(st_device_list.nDeviceNum, nullptr);
    rets_.resize(st_device_list.nDeviceNum, MV_OK);

    // === Step 4: 初始化每个相机并应用参数 ===
    for (unsigned int i = 0; i < st_device_list.nDeviceNum; ++i) {
        auto& handle = handles_[i];
        auto& ret = rets_[i];

        ret = MV_CC_CreateHandleWithoutLog(&handle, st_device_list.pDeviceInfo[i]);
        if (MV_OK != ret) {
            LOG(ERROR) << "MV_CC_CreateHandle fail! n_ret [" << ret << "]";
            continue;
        }

        ret = MV_CC_OpenDevice(handle);
        if (MV_OK != ret) {
            LOG(ERROR) << "MV_CC_OpenDevice fail! n_ret [" << ret << "]";
            continue;
        }

        // GigE: 设置最佳包大小
        if (st_device_list.pDeviceInfo[i]->nTLayerType == MV_GIGE_DEVICE) {
            int n_packet_size = MV_CC_GetOptimalPacketSize(handle);
            if (n_packet_size > 0) {
                ret = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", n_packet_size);
                if (ret != MV_OK) {
                    LOG(WARNING) << "Set Packet Size fail n_ret [0x" << std::hex << ret << "]";
                }
            } else {
                LOG(WARNING) << "Get Packet Size fail n_ret [0x" << std::hex << n_packet_size << "]";
            }
        }

        // 设置图像缓存
        ret = MV_CC_SetImageNodeNum(handle, 100);
        if (MV_OK != ret) {
            LOG(ERROR) << "MV_CC_SetImageNodeNum fail! n_ret [" << ret << "]";
        }

        // === 应用从配置文件读取的参数 ===
        ret = MV_CC_SetExposureAutoMode(handle, params.exposure_mode);
        if (MV_OK != ret) LOG(ERROR) << "SetExposureAutoMode fail! n_ret [" << ret << "]";

        // 如果启用了自动曝光（mode == 2），则不要手动设置曝光时间，
        // 避免覆盖相机自动计算的值。保留配置中的 exposure_time 作为回退。
        if (params.exposure_mode != 2) {
          ret = MV_CC_SetExposureTime(handle, params.exposure_time);
          if (MV_OK != ret) LOG(ERROR) << "SetExposureTime fail! n_ret [" << ret << "]";
        } else {
          LOG(INFO) << "Auto exposure enabled (mode=2); skipping explicit exposure time.";
        }

        ret = MV_CC_SetGainMode(handle, params.gain_mode);
        if (MV_OK != ret) LOG(ERROR) << "SetGainMode fail! n_ret [" << ret << "]";

        ret = MV_CC_SetGain(handle, params.gain);
        if (MV_OK != ret) LOG(ERROR) << "SetGain fail! n_ret [" << ret << "]";

        // ret = MV_CC_SetBalanceWhiteAuto(handle, params.balance_mode);
        // if (MV_OK != ret) LOG(ERROR) << "SetBalanceWhiteAuto fail! n_ret [" << ret << "]";

        // ret = MV_CC_SetBalanceRatioRed(handle, params.balance_ratio_red);
        // if (MV_OK != ret) LOG(ERROR) << "SetBalanceRatioRed fail! n_ret [" << ret << "]";

        // ret = MV_CC_SetBalanceRatioGreen(handle, params.balance_ratio_green);
        // if (MV_OK != ret) LOG(ERROR) << "SetBalanceRatioGreen fail! n_ret [" << ret << "]";

        // ret = MV_CC_SetBalanceRatioBlue(handle, params.balance_ratio_blue);
        // if (MV_OK != ret) LOG(ERROR) << "SetBalanceRatioBlue fail! n_ret [" << ret << "]";

        // ret = MV_CC_SetGamma(handle, params.gamma);
        // if (MV_OK != ret) LOG(ERROR) << "SetGamma fail! n_ret [" << ret << "]";

        // 启动抓图
        ret = MV_CC_StartGrabbing(handle);
        if (MV_OK != ret) {
            LOG(ERROR) << "MV_CC_StartGrabbing fail! n_ret [" << ret << "]";
        }

        LOG(INFO) << "Camera " << i << " initialized with config.";
    }

    // 注意：原代码此处逻辑有误 —— 检测到设备应返回 true，但写反了
    // 修正如下：
    return st_device_list.nDeviceNum > 0;
}

void MvCam::Stop() {
  Disable();
  std::this_thread::sleep_for(std::chrono::milliseconds{500});
  for (auto &cam_thread : cam_threads) {
    while (cam_thread.joinable()) {
      cam_thread.join();
    }
  }
  cam_threads.clear();
  cam_threads.shrink_to_fit();
  for (size_t i = 0; i < handles_.size(); ++i) {
    int n_ret = MV_OK;
    n_ret = MV_CC_StopGrabbing(handles_[i]);
    if (MV_OK != n_ret) {
      LOG(ERROR) << "MV_CC_StopGrabbing fail! n_ret [" << n_ret << "]";
    }
    n_ret = MV_CC_CloseDevice(handles_[i]);
    if (MV_OK != n_ret) {
      LOG(ERROR) << "MV_CC_CloseDevice fail! n_ret [" << n_ret << "]";
    }
    if (n_ret != MV_OK) {
      if (!handles_[i]) {
        MV_CC_DestroyHandle(handles_[i]);
        handles_[i] = nullptr;
      }
    }
    LOG(INFO) << "Exit  " << i << "  cam ";
  }
}
void MvCam::Receive(void *handle, const std::string &name) {
  unsigned int last_count = 0;
  MV_FRAME_OUT st_out_frame;
  CamData cam_data;
  Messenger &messenger = Messenger::GetInstance();
  while (is_running) {
    memset(&st_out_frame, 0, sizeof(MV_FRAME_OUT));
    int n_ret = MV_CC_GetImageBuffer(handle, &st_out_frame, 10);
    if (n_ret == MV_OK) {
      MVCC_FLOATVALUE expose_time;
      n_ret = MV_CC_GetExposureTime(handle, &expose_time);
      if (n_ret != MV_OK) {
        LOG(ERROR) << "Get ExposureTime fail! n_ret [0x" << std::hex << n_ret << "]";
      }
      // 将相机报告的曝光时间保存（单位为微秒），并在计算时间戳时使用曝光时间的一半
      cam_data.exposure_time_us = static_cast<uint64_t>(expose_time.fCurValue);
      // 这里的time_stamp_us是相机触发时间，需要加上曝光时间的一半，以获得相机拍摄的时间
      if (params.find(name) == params.end()) {
        LOG(ERROR) << "cam: " << name << " not found!";
      } else {
        if (uint64_t time; GET_LAST_TRIGGER_STATUS(params[name], time)) {
          cam_data.time_stamp_us = time + static_cast<uint64_t>(expose_time.fCurValue / 2.);
        } else {
          LOG(ERROR) << "Trigger cam: " << name << " not found!";
        }
      }
      
      // TODO3: 获取当前时间戳信息
		  if(pointt != MAP_FAILED && pointt->low != 0)
		  {
			  // 赋值共享内存中的时间戳给相机帧
		    cam_data.time_stamp_us = pointt->low;
        // LOG(INFO) << "time_stamp_s: " << cam_data.time_stamp_us/1000000 << " not found!";
		  }

      MvGvspPixelType en_dst_pixel_type = PixelType_Gvsp_Undefined;
      unsigned int n_channel_num = 0;
      // 如果是彩色则转成RGB8
      if (IsBayerRG8(st_out_frame.stFrameInfo.enPixelType)) {
        n_channel_num = 3;
        en_dst_pixel_type = PixelType_Gvsp_BGR8_Packed;
      }
      else if (IsColor(st_out_frame.stFrameInfo.enPixelType)) {
        n_channel_num = 3;
        en_dst_pixel_type = PixelType_Gvsp_RGB8_Packed;
      }
      // 如果是黑白则转换成Mono8
      else if (IsMono(st_out_frame.stFrameInfo.enPixelType)) {
        n_channel_num = 1;
        en_dst_pixel_type = PixelType_Gvsp_Mono8;
      }
      else if (IsBayerGB8(st_out_frame.stFrameInfo.enPixelType)) {
        n_channel_num = 3;
        en_dst_pixel_type = PixelType_Gvsp_BGR8_Packed;
      }
      if (n_channel_num != 0) {
        cam_data.name = name;
        if (en_dst_pixel_type == PixelType_Gvsp_BGR8_Packed) {
          // cv::Mat out_image;
          // cv::Mat cv_image = cv::Mat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,CV_8UC1, st_out_frame.pBufAddr);
          // // cv::cvtColor(cv_image, out_image, cv::COLOR_BayerBG2GRAY);
          // // cam_data.image = GMat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,CV_8UC1, out_image.data);
          // cv::cvtColor(cv_image, out_image, cv::COLOR_BayerBG2BGR);
          // cam_data.image = GMat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,CV_8UC3, out_image.data);
          // // cam_data.image = GMat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,
          // //                       GMatType<uint8_t, 3>::Type, st_out_frame.pBufAddr)
          // //                      .Clone();
          
#define MAX_IMAGE_DATA_SIZE (3 * 4000 * 4000)
          unsigned char *m_pBufForSaveImage = (unsigned char *) malloc(MAX_IMAGE_DATA_SIZE);
          MV_CC_PIXEL_CONVERT_PARAM stConvertParam{};
          stConvertParam.nWidth      = st_out_frame.stFrameInfo.nWidth;            // ch:图像宽 | en:image width
          stConvertParam.nHeight     = st_out_frame.stFrameInfo.nHeight;           // ch:图像高 | en:image height
          stConvertParam.pSrcData    = st_out_frame.pBufAddr;               // ch:输入数据缓存 | en:input data buffer
          stConvertParam.nSrcDataLen = st_out_frame.stFrameInfo.nFrameLen;         // ch:输入数据大小 | en:input data size
          stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; // ch:输出像素格式 | en:output pixel format //!
                                                                      // 输出格式 RGB
          stConvertParam.pDstBuffer     = m_pBufForSaveImage;  // ch:输出数据缓存 | en:output data buffer
          stConvertParam.nDstBufferSize = MAX_IMAGE_DATA_SIZE; // ch:输出缓存大小 | en:output buffer size
          stConvertParam.enSrcPixelType = st_out_frame.stFrameInfo.enPixelType; // ch:输入像素格式 | en:input pixel format
          auto nRet = MV_CC_ConvertPixelType(handle, &stConvertParam);
          if (nRet != MV_OK) {
              printf("Pixel conversion failed! Error: 0x%x\n", nRet);
              continue;
          }
          cam_data.image = GMat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,
                                GMatType<uint8_t, 3>::Type, m_pBufForSaveImage)
                               .Clone();
          free(m_pBufForSaveImage);
        }
        if (en_dst_pixel_type == PixelType_Gvsp_Mono8) {
          cam_data.image = GMat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,
                                GMatType<uint8_t, 1>::Type, st_out_frame.pBufAddr)
                               .Clone();
        }
        if (en_dst_pixel_type == PixelType_Gvsp_RGB8_Packed) {
          cam_data.image = GMat(st_out_frame.stFrameInfo.nHeight, st_out_frame.stFrameInfo.nWidth,
                                GMatType<uint8_t, 3>::Type, st_out_frame.pBufAddr)
                               .Clone();
        }
        messenger.PubStruct(name, &cam_data, sizeof(cam_data));
        if (last_count == 0) {
          last_count = st_out_frame.stFrameInfo.nFrameNum;
        } else {
          last_count++;
          if (last_count != st_out_frame.stFrameInfo.nFrameNum) {
            LOG(WARNING) << "Loss of data from cam connection : " << last_count;
            last_count = 0;
          }
        }
      }
    }
    if (nullptr != st_out_frame.pBufAddr) {
      n_ret = MV_CC_FreeImageBuffer(handle, &st_out_frame);
      if (n_ret != MV_OK) {
        LOG(ERROR) << "Free Image Buffer fail! n_ret [0x" << std::hex << n_ret << "]";
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{2});
  }
}
void MvCam::Start() {
  for (const auto &handle : handles_) {
    int n_ret = MV_OK;
    MVCC_STRINGVALUE pst_value;
    n_ret = MV_CC_GetDeviceUserID(handle, &pst_value);
    if (n_ret != MV_OK) {
      LOG(ERROR) << "Get DeviceUserID fail! n_ret [0x" << std::hex << n_ret << "]";
    }
    Enable();
    auto name = std::string(pst_value.chCurValue);
    if (name.empty()) {
      // 相机名称默认从1开始
      static int cam_index{1};
      name = "cam_" + std::to_string(cam_index++);
      LOG(WARNING) << "Camera name is empty,Create new name: " << name;
    } else {
      LOG(INFO) << "Camera name is " << name;
    }
    cam_threads.emplace_back(&MvCam::Receive, this, handle, name);
    LOG(INFO) << "Camera " << name << " start.";
  }
}
}  // namespace infinite_sense
