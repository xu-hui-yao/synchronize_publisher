#include "infinite_sense.h"

#include "trigger.h"
#include "net.h"
#include "usb.h"
#include "messenger.h"
#include "log.h"

namespace infinite_sense {
Synchronizer::Synchronizer() {
  LOG(INFO) << "\n"
            << "  ▗▄▄▄▖▗▖  ▗▖▗▄▄▄▖▗▄▄▄▖▗▖  ▗▖▗▄▄▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▄▖▗▄▄▄▖▗▖  ▗▖ ▗▄▄▖▗▄▄▄▖" << "\n"
            << "    █  ▐▛▚▖▐▌▐▌     █  ▐▛▚▖▐▌  █    █  ▐▌   ▐▌   ▐▌   ▐▛▚▖▐▌▐▌   ▐▌   " << "\n"
            << "    █  ▐▌ ▝▜▌▐▛▀▀▘  █  ▐▌ ▝▜▌  █    █  ▐▛▀▀▘ ▝▀▚▖▐▛▀▀▘▐▌ ▝▜▌ ▝▀▚▖▐▛▀▀▘" << "\n"
            << "  ▗▄█▄▖▐▌  ▐▌▐▌   ▗▄█▄▖▐▌  ▐▌▗▄█▄▖  █  ▐▙▄▄▖▗▄▄▞▘▐▙▄▄▖▐▌  ▐▌▗▄▄▞▘▐▙▄▄▖";
  int major, minor, patch;
  zmq_version(&major, &minor, &patch);
  LOG(INFO) << "ZeroMQ version: " << major << "." << minor << "." << patch;
};
void Synchronizer::SetLogPath(const std::string& path) { SetLogDestination(INFO, path.c_str()); }
void Synchronizer::SetNetLink(std::string net_dev, const unsigned int port) {
  net_ip_ = std::move(net_dev);
  net_port_ = port;
  net_manager_ = std::make_shared<NetManager>(net_ip_, net_port_);
}
void Synchronizer::SetUsbLink(std::string serial_dev, const int serial_baud_rate) {
  serial_dev_ = std::move(serial_dev);
  serial_baud_rate_ = serial_baud_rate;
  serial_manager_ = std::make_shared<UsbManager>(serial_dev_, serial_baud_rate_);
  net_manager_ = nullptr;
}
void Synchronizer::UseSensor(const std::shared_ptr<Sensor>& sensor) { sensor_manager_ = sensor; }

void Synchronizer::Start() const {
  if (net_manager_) {
    net_manager_->Start();
  }
  if (serial_manager_) {
    serial_manager_->Start();
  }
  if (sensor_manager_) {
    std::this_thread::sleep_for(std::chrono::milliseconds{2000});
    sensor_manager_->Initialization();
    sensor_manager_->Start();
  }
  LOG(INFO) << "Synchronizer Started.";
}
void Synchronizer::Stop() const {
  if (net_manager_) {
    net_manager_->Stop();
  }
  if (serial_manager_) {
    serial_manager_->Stop();
  }
  if (sensor_manager_) {
    sensor_manager_->Stop();
  }
  LOG(INFO) << "Synchronizer Stopped";
}

}  // namespace infinite_sense
