"""
信号图表模块

使用pyqtgraph实现实时触发信号可视化
支持多通道信号显示、时间轴缩放、设备高亮等功能
"""

import numpy as np
from collections import deque

import pyqtgraph as pg
from PyQt6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QCheckBox, QSpinBox
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont

from ...data import TriggerEvent


class SignalChart(QWidget):
    """触发信号图表组件"""

    def __init__(self, max_points: int = 10000):
        super().__init__()
        self.max_points = max_points
        self.device_names = ["IMU_1", "IMU_2", "CAM_1", "CAM_2", "CAM_3", "CAM_4", "LASER", "GPS"]
        self.colors = [
            '#FF6B6B',  # 红色 - IMU_1
            '#4ECDC4',  # 青色 - IMU_2
            '#45B7D1',  # 蓝色 - CAM_1
            '#96CEB4',  # 绿色 - CAM_2
            '#FECA57',  # 黄色 - CAM_3
            '#FF9FF3',  # 粉色 - CAM_4
            '#54A0FF',  # 深蓝 - LASER
            '#5F27CD'   # 紫色 - GPS
        ]

        # 数据存储
        self.time_data = deque(maxlen=max_points)
        self.signal_data = {device: deque(maxlen=max_points) for device in self.device_names}
        self.plot_items = {}

        self.highlighted_device = None

        self.init_ui()
        self.init_plot()

    def init_ui(self):
        """初始化界面"""
        layout = QVBoxLayout(self)

        # 标题和控制区
        self.create_header(layout)

        # 图表区域
        self.create_plot_area(layout)

        # 图例和设备选择
        self.create_legend_area(layout)

    def create_header(self, parent_layout):
        """创建标题和控制区"""
        header_layout = QHBoxLayout()

        # 标题
        title_label = QLabel("实时触发信号")
        font = QFont()
        font.setBold(True)
        font.setPointSize(12)
        title_label.setFont(font)
        header_layout.addWidget(title_label)

        header_layout.addStretch()

        # 时间窗口控制
        time_window_label = QLabel("时间窗口(秒):")
        self.time_window_spin = QSpinBox()
        self.time_window_spin.setRange(1, 600)
        self.time_window_spin.setValue(1)
        self.time_window_spin.valueChanged.connect(self.update_time_window)

        header_layout.addWidget(time_window_label)
        header_layout.addWidget(self.time_window_spin)

        parent_layout.addLayout(header_layout)

    def create_plot_area(self, parent_layout):
        """创建图表区域"""
        # 使用pyqtgraph的PlotWidget
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setBackground('w')  # 白色背景
        self.plot_widget.setLabel('left', '设备编号')
        self.plot_widget.setLabel('bottom', '时间')
        self.plot_widget.showGrid(x=True, y=True, alpha=0.3)

        # 设置Y轴范围和标签
        self.plot_widget.setYRange(-0.5, len(self.device_names) - 0.5)
        self.plot_widget.getAxis('left').setTicks([[(i, name) for i, name in enumerate(self.device_names)]])

        # 禁用自动范围调整，使用手动控制
        self.plot_widget.enableAutoRange(axis='x', enable=False)

        parent_layout.addWidget(self.plot_widget, 1)

    def create_legend_area(self, parent_layout):
        """创建图例和设备选择区域"""
        legend_layout = QHBoxLayout()

        # 设备显示控制复选框
        for i, (device_name, color) in enumerate(zip(self.device_names, self.colors)):
            checkbox = QCheckBox(device_name)
            checkbox.setChecked(True)
            checkbox.stateChanged.connect(lambda state, dev=device_name: self.toggle_device_visibility(dev, state))

            # 设置颜色样式
            checkbox.setStyleSheet(f"""
                QCheckBox::indicator:checked {{
                    background-color: {color};
                    border: 1px solid #555;
                }}
                QCheckBox::indicator:unchecked {{
                    background-color: white;
                    border: 1px solid #555;
                }}
            """)

            legend_layout.addWidget(checkbox)

        legend_layout.addStretch()
        parent_layout.addLayout(legend_layout)

    def init_plot(self):
        """初始化图表"""
        # 为每个设备创建散点图
        for i, (device_name, color) in enumerate(zip(self.device_names, self.colors)):
            scatter = pg.ScatterPlotItem(
                size=8,
                pen=pg.mkPen(color, width=2),
                brush=pg.mkBrush(color),
                symbol='o'
            )
            self.plot_widget.addItem(scatter)
            self.plot_items[device_name] = scatter

    def add_trigger_event(self, event: TriggerEvent):
        """添加触发事件到图表"""
        current_time = event.timestamp_us / 1_000_000  # 转换为秒
        self.time_data.append(current_time)

        # 为每个设备添加数据点
        for i, device_name in enumerate(self.device_names):
            if device_name in event.triggered_devices:
                # 设备触发，添加数据点
                self.signal_data[device_name].append((current_time, i))
            # 注意：不再为未触发的设备添加None值，这样可以避免数据同步问题

        self.update_plot()

    def update_plot(self):
        """更新图表显示"""
        if not self.time_data:
            return

        # 使用数据中的最新时间作为基准，而不是系统当前时间
        latest_time = max(self.time_data)
        time_window = self.time_window_spin.value()
        window_start = latest_time - time_window


        for device_name, scatter in self.plot_items.items():
            # 获取设备数据
            device_data = self.signal_data[device_name]

            # 过滤时间窗口内的数据
            valid_points = []
            for point in device_data:
                timestamp, y_value = point
                if timestamp >= window_start:
                    valid_points.append([timestamp, y_value])


            if valid_points:
                valid_points = np.array(valid_points)
                scatter.setData(valid_points[:, 0], valid_points[:, 1])
            else:
                scatter.setData([], [])

        # 更新X轴范围 - 使用数据时间基准
        self.plot_widget.setXRange(window_start, latest_time)

    def highlight_device(self, device_name: str):
        """高亮显示特定设备"""
        self.highlighted_device = device_name

        for name, scatter in self.plot_items.items():
            if name == device_name:
                # 高亮选中的设备
                scatter.setSize(12)
                scatter.setPen(pg.mkPen('black', width=3))
            else:
                # 其他设备恢复正常
                device_index = self.device_names.index(name)
                color = self.colors[device_index]
                scatter.setSize(8)
                scatter.setPen(pg.mkPen(color, width=2))

    def toggle_device_visibility(self, device_name: str, state: int):
        """切换设备可见性"""
        visible = state == Qt.CheckState.Checked.value
        self.plot_items[device_name].setVisible(visible)

    def update_time_window(self):
        """更新时间窗口"""
        self.update_plot()

    def clear_data(self):
        """清空数据"""
        self.time_data.clear()
        for device_data in self.signal_data.values():
            device_data.clear()

        for scatter in self.plot_items.values():
            scatter.setData([], [])


