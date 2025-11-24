#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <vector>
#include <algorithm>
#include <zmq.hpp>
#include <thread>
#include <atomic>
class TopicMonitor {
 public:
  static TopicMonitor& GetInstance() {
    static TopicMonitor instance;
    return instance;
  }
  TopicMonitor(const TopicMonitor&) = delete;
  TopicMonitor(TopicMonitor&&) = delete;
  TopicMonitor& operator=(const TopicMonitor&) = delete;
  TopicMonitor& operator=(TopicMonitor&&) = delete;
  std::unordered_set<std::string> GetTopics() const;
  friend std::ostream& operator<<(std::ostream& os, const TopicMonitor& monitor) {
    std::lock_guard lock(monitor.topics_mutex_);
    os << "\n--- Topic Monitor ---\n";
    if (monitor.topic_frequencies_.empty()) {
      os << "  No active topics\n";
    } else {
      os << "  Active Topics (" << monitor.topic_frequencies_.size() << "):\n";
      std::vector<std::pair<std::string, size_t>> sorted_topics(monitor.topic_frequencies_.begin(),
                                                                monitor.topic_frequencies_.end());
      std::sort(sorted_topics.begin(), sorted_topics.end(),
                [](const auto& a, const auto& b) { return b.second < a.second; });

      for (const auto& [topic, count] : sorted_topics) {
        os << "    • " << topic << " (num: " << count << ")\n";
      }
    }
    return os << "---------------------\n";
  }
  // 启动监控线程
  void Start();
  // 停止监控
  void Stop();

 private:
  TopicMonitor();
  ~TopicMonitor();
  // 监控线程主循环
  void MonitorLoop();
  std::unordered_map<std::string, size_t> topic_frequencies_;  // 新增：topic频率统计
  zmq::context_t context_;
  zmq::socket_t subscriber_;
  std::unordered_set<std::string> topics_;
  mutable std::mutex topics_mutex_;
  std::thread monitor_thread_;
  std::atomic<bool> should_run_;
};