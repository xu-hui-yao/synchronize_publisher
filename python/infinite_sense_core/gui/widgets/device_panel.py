"""
设备状态面板模块

显示8个触发设备的状态和统计信息
支持设备选择和实时状态更新
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QFrame, QScrollArea, QGroupBox
)
from PyQt6.QtCore import pyqtSignal, QTimer, Qt
from PyQt6.QtGui import QPalette, QFont

from ...data import TriggerEvent, get_trigger_processor
from .trigger_indicator import TriggerIndicator


class DeviceStatusWidget(QFrame):
    """单个设备状态组件"""

    clicked = pyqtSignal(str)  # 设备被点击信号

    def __init__(self, device_name: str):
        super().__init__()
        self.device_name = device_name
        self.trigger_count = 0
        self.last_trigger_time = 0
        self.is_selected = False

        self.init_ui()

    def init_ui(self):
        """初始化界面"""
        self.setFrameStyle(QFrame.Shape.StyledPanel)
        self.setMinimumHeight(80)
        self.setMaximumHeight(80)

        layout = QHBoxLayout(self)
        layout.setContentsMargins(8, 4, 8, 4)

        # 触发指示器
        self.indicator = TriggerIndicator()
        layout.addWidget(self.indicator)

        # 设备信息区域
        info_layout = QVBoxLayout()

        # 设备名称
        self.name_label = QLabel(self.device_name)
        font = QFont()
        font.setBold(True)
        font.setPointSize(10)
        self.name_label.setFont(font)
        info_layout.addWidget(self.name_label)

        # 统计信息
        self.stats_label = QLabel("计数: 0 | 最后触发: 无")
        self.stats_label.setStyleSheet("color: gray; font-size: 9px;")
        info_layout.addWidget(self.stats_label)

        layout.addLayout(info_layout, 1)

        # 设置点击事件
        self.mousePressEvent = self.on_clicked

    def on_clicked(self, event):
        """处理点击事件"""
        self.clicked.emit(self.device_name)

    def update_trigger_status(self, triggered: bool):
        """更新触发状态"""
        self.indicator.set_triggered(triggered)
        if triggered:
            self.trigger_count += 1
            self.update_stats_display()

    def update_stats_display(self):
        """更新统计信息显示"""
        stats = get_trigger_processor().get_device_stats(self.device_name)
        self.trigger_count = stats.get('trigger_count', 0)
        self.stats_label.setText(f"计数: {self.trigger_count}")

    def set_selected(self, selected: bool):
        """设置选中状态"""
        self.is_selected = selected
        if selected:
            self.setStyleSheet("QFrame { border: 2px solid #0078d4; background-color: #f0f8ff; }")
        else:
            self.setStyleSheet("QFrame { border: 1px solid #ccc; background-color: white; }")


class DevicePanel(QWidget):
    """设备面板主组件"""

    device_selected = pyqtSignal(str)  # 设备选择信号

    def __init__(self):
        super().__init__()
        self.device_widgets = {}
        self.selected_device = None

        self.init_ui()
        self.init_timer()

    def init_ui(self):
        """初始化界面"""
        layout = QVBoxLayout(self)

        # 标题
        title_label = QLabel("设备状态监控")
        font = QFont()
        font.setBold(True)
        font.setPointSize(12)
        title_label.setFont(font)
        layout.addWidget(title_label)

        # 创建滚动区域
        scroll_area = QScrollArea()
        scroll_widget = QWidget()
        scroll_layout = QVBoxLayout(scroll_widget)

        # 创建设备组件
        device_names = ["IMU_1", "IMU_2", "CAM_1", "CAM_2", "CAM_3", "CAM_4", "LASER", "GPS"]

        for device_name in device_names:
            device_widget = DeviceStatusWidget(device_name)
            device_widget.clicked.connect(self.on_device_selected)
            self.device_widgets[device_name] = device_widget
            scroll_layout.addWidget(device_widget)

        scroll_layout.addStretch()
        scroll_area.setWidget(scroll_widget)
        scroll_area.setWidgetResizable(True)
        scroll_area.setMinimumWidth(200)

        layout.addWidget(scroll_area)

        # 总体统计信息
        self.create_summary_group(layout)

    def create_summary_group(self, parent_layout):
        """创建总体统计信息组"""
        summary_group = QGroupBox("总体统计")
        summary_layout = QVBoxLayout(summary_group)

        self.total_triggers_label = QLabel("总触发次数: 0")
        self.active_devices_label = QLabel("活跃设备: 0")
        self.last_activity_label = QLabel("最后活动: 无")

        summary_layout.addWidget(self.total_triggers_label)
        summary_layout.addWidget(self.active_devices_label)
        summary_layout.addWidget(self.last_activity_label)

        parent_layout.addWidget(summary_group)

    def init_timer(self):
        """初始化定时器用于定期更新统计信息"""
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_summary_stats)
        self.update_timer.start(1000)  # 每秒更新一次

    def on_device_selected(self, device_name: str):
        """处理设备选择"""
        # 取消之前的选择
        if self.selected_device:
            self.device_widgets[self.selected_device].set_selected(False)

        # 设置新的选择
        self.selected_device = device_name
        self.device_widgets[device_name].set_selected(True)

        # 发送信号
        self.device_selected.emit(device_name)

    def update_trigger_event(self, event: TriggerEvent):
        """更新触发事件"""
        # 更新所有设备状态
        for device_name, widget in self.device_widgets.items():
            triggered = device_name in event.triggered_devices
            widget.update_trigger_status(triggered)

    def update_summary_stats(self):
        """更新总体统计信息"""
        trigger_processor = get_trigger_processor()
        total_triggers = 0
        active_devices = 0

        for device_name in self.device_widgets.keys():
            count = trigger_processor.get_device_stats(device_name).get('trigger_count', 0)
            total_triggers += count
            if count > 0:
                active_devices += 1

        self.total_triggers_label.setText(f"总触发次数: {total_triggers}")
        self.active_devices_label.setText(f"活跃设备: {active_devices}")
        self.last_activity_label.setText("最后活动: 实时更新")