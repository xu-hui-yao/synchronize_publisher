#pragma once
#include "infinite_sense.h"
#include <unordered_map>
#include <string>
namespace infinite_sense {
class MvCam final : public Sensor {
 public:
  ~MvCam() override;

  bool Initialization() override;
  void Stop() override;
  void Start() override;

 private:
  void Receive(void* handle, const std::string&) override;
  std::vector<int> rets_;
  std::vector<void*> handles_;
  // Per-camera LSC calibration buffers, keyed by device user ID (camera name)
  std::unordered_map<std::string, std::vector<unsigned char>> calib_map_;
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
    // int saturation = 128; // 可选
};
}  // namespace infinite_sense
