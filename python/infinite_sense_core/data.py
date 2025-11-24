import json
from typing import Dict, Callable, Optional, List
from dataclasses import dataclass
from datetime import datetime
import threading

from .message import Messenger

@dataclass
class TriggerEvent:
    """触发事件数据结构"""
    timestamp_us: int
    status: int  # 8位状态掩码
    device_mask: int = 0xFF  # 设备掩码，标识哪些设备触发

    @property
    def triggered_devices(self) -> List[str]:
        """获取触发的设备列表"""
        devices = []
        device_names = ["IMU_1", "IMU_2", "CAM_1", "CAM_2", "CAM_3", "CAM_4", "LASER", "GPS"]
        for i, name in enumerate(device_names):
            if self.status & (1 << i):
                devices.append(name)
        return devices

    @property
    def timestamp_str(self) -> str:
        """获取可读的时间戳字符串"""
        return datetime.fromtimestamp(self.timestamp_us / 1_000_000).strftime("%H:%M:%S.%f")

class TriggerProcessor:
    """触发信号处理器，提供实时监控和分析功能"""

    def __init__(self):
        self._callbacks: List[Callable[[TriggerEvent], None]] = []
        self._lock = threading.Lock()
        self._last_trigger_time = {}  # 每个设备的最后触发时间
        self._trigger_counts = {}     # 每个设备的触发计数

    def add_callback(self, callback: Callable[[TriggerEvent], None]):
        """添加触发事件回调函数"""
        with self._lock:
            self._callbacks.append(callback)

    def remove_callback(self, callback: Callable[[TriggerEvent], None]):
        """移除触发事件回调函数"""
        with self._lock:
            if callback in self._callbacks:
                self._callbacks.remove(callback)

    def process_trigger_data(self, data: Dict):
        """处理触发数据"""
        if data.get("f") != "t":
            return

        trigger_event = TriggerEvent(
            timestamp_us=data["t"],
            status=data["s"]
        )

        # 更新统计信息
        self._update_statistics(trigger_event)

        # 调用所有回调函数
        with self._lock:
            for callback in self._callbacks:
                callback(trigger_event)

    def _update_statistics(self, event: TriggerEvent):
        """更新触发统计信息"""
        with self._lock:
            for device_name in event.triggered_devices:
                self._last_trigger_time[device_name] = event.timestamp_us
                self._trigger_counts[device_name] = self._trigger_counts.get(device_name, 0) + 1

    def get_device_stats(self, device_name: str) -> dict:
        """获取设备统计信息"""
        with self._lock:
            return {
                "last_trigger_time": self._last_trigger_time.get(device_name, 0),
                "trigger_count": self._trigger_counts.get(device_name, 0)
            }

# 全局触发处理器实例
_trigger_processor = TriggerProcessor()

def get_trigger_processor() -> TriggerProcessor:
    """获取全局触发处理器实例"""
    return _trigger_processor

def process_trigger_data(data: Dict):
    """处理触发数据（兼容原有接口）"""
    _trigger_processor.process_trigger_data(data)


def process_imu_data(data: Dict):
    if data.get("f") != "imu":
        return
    imu = {
        "time_stamp_us": data["t"],
        "a": data["d"][:3],
        "g": data["d"][3:6],
        "temperature": data["d"][6],
        "q": data["q"][:4]
    }
    Messenger.get_instance().pub("imu_1", json.dumps(imu))


def process_gps_data(data: Dict):
    if data.get("f") != "GNGGA":
        return
    gps = {
        "data": data["d"],
        "trigger_time_us": data["pps"],
        "time_stamp_us": data["t"]
    }
    Messenger.pub("gps", json.dumps(gps))

def process_log_data(data: Dict):
    if data.get("f") != "log":
        return
