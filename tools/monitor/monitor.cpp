#include "monitor.h"
#include "infinite_sense.h"
std::unordered_set<std::string> TopicMonitor::GetTopics() const {
  std::lock_guard lock(topics_mutex_);
  return topics_;
}
void TopicMonitor::Start() {
  if (monitor_thread_.joinable()) {
    return;
  }
  should_run_.store(true);
  monitor_thread_ = std::thread(&TopicMonitor::MonitorLoop, this);
}

void TopicMonitor::Stop() {
  if (!should_run_.load()) {
    return;
  }
  should_run_.store(false);
  if (monitor_thread_.joinable()) {
    monitor_thread_.join();
  }
}
TopicMonitor::TopicMonitor() : context_(1), subscriber_(context_, ZMQ_SUB), should_run_(false) {
  try {
    subscriber_.connect("tcp://127.0.0.1:4565");
    subscriber_.set(zmq::sockopt::subscribe, "");
  } catch (const zmq::error_t &e) {
    LOG(ERROR) << "[TopicMonitor] Initialization failed: " << e.what();
    throw;
  }
}
TopicMonitor::~TopicMonitor() {
  Stop();
  try {
    subscriber_.close();
    context_.close();
  } catch (const zmq::error_t &e) {
    LOG(ERROR) << "[TopicMonitor] Cleanup error: " << e.what();
  }
}
void TopicMonitor::MonitorLoop() {
  zmq::message_t msg;

  while (should_run_.load()) {
    try {
      if (subscriber_.recv(msg, zmq::recv_flags::dontwait)) {
        {
          std::string topic(static_cast<char *>(msg.data()), msg.size());
          std::lock_guard lock(topics_mutex_);
          topic_frequencies_[topic]++;
        }
        if (subscriber_.get(zmq::sockopt::rcvmore)) {
          zmq::message_t dummy;
          subscriber_.recv(dummy);
        }
      }
    } catch (const zmq::error_t &e) {
      if (e.num() != ETERM) {
      }
    }
  }
}
