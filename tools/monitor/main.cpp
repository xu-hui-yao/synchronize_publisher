#include "infinite_sense.h"
#include "monitor.h"
using namespace infinite_sense;
int main() {
  TopicMonitor::GetInstance().Start();
  std::this_thread::sleep_for(std::chrono::milliseconds{1001});
  TopicMonitor::GetInstance().Stop();
  LOG(INFO) << TopicMonitor::GetInstance();
  return 0;
}