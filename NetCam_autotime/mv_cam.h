#pragma once
#include "infinite_sense.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <unordered_map>

namespace infinite_sense {
class MvCam final : public Sensor {
 public:
  // 对外的样本结构体（用于将少量像素采样从回调传到后台）
  struct ExposureSample {
    std::string name;
    std::vector<uint8_t> samples;
    uint64_t time_stamp_us = 0;
    int grid_rows = 6;
    int grid_cols = 6;
  };

  // 将来自 ImageCallback 的小型采样数据入队（公有接口）
  void EnqueueSample(const ExposureSample &s);
  ~MvCam() override;

  bool Initialization() override;
  void Stop() override;
  void Start() override;
  // Set exposure time (microseconds) for a camera by its user-defined name.
  bool SetExposureTimeByName(const std::string &name, float exposure_us);
  // 非阻塞：将曝光设置请求放入队列，由后台线程异步下发到相机
  void EnqueueExposure(const std::string &name, float exposure_us);

 private:
  void Receive(void* handle, const std::string&) override;
  std::vector<int> rets_;
  std::vector<void*> handles_;
  std::unordered_map<std::string, void*> name_to_handle_;

  // 异步曝光控制
  std::thread exposure_thread_;
  std::mutex exposure_mutex_;
  std::condition_variable exposure_cv_;
  std::unordered_map<std::string, float> pending_exposures_;
  std::atomic<bool> exposure_thread_running_{false};
  // 异步曝光线程入口（私有静态方法，便于访问私有成员）
  static void ExposureWorker(MvCam* self);

  // --- 方案 B: 样本队列，用于将采样（很小的数据量）从回调线程传到后台处理线程 ---
  std::queue<ExposureSample> sample_queue_;
  std::mutex sample_mutex_;
  std::condition_variable sample_cv_;

  // AE 参数与当前曝光状态（由后台线程维护）
  std::unordered_map<std::string, double> current_exposure_us_;
  double ae_ideal_brightness_ = 120.0;
  double ae_alpha_ = 0.3;
  double ae_default_exposure_time_ = 30000.0;
  double ae_t_min_us_ = 50.0;
  double ae_t_max_us_ = 10000000.0;
  int sample_grid_rows_ = 6;
  int sample_grid_cols_ = 6;

  
};

struct CameraParams {
    int exposure_mode = 0;
    float exposure_time = 10000.0f;
    int gain_mode = 0;
    float gain = 6.0f;
    int balance_mode = 0;
    int balance_ratio_red = 1427;
    int balance_ratio_green = 1024;
    int balance_ratio_blue = 1404;
    float gamma = 0.7f;
    // Trigger settings: 用于配置外部硬件触发（同步信号）
    // trigger_mode: 0 = Off, 1 = On (外部触发)
    int trigger_mode = 0;
    // trigger_source: 相机厂商定义的触发源编码（例如 Line0/Software 等），默认 0
    int trigger_source = 0;
    // 触发延时，单位微秒（可选）
    float trigger_delay_us = 0.0f;
    // int saturation = 128; // 可选
};
}  // namespace infinite_sense
