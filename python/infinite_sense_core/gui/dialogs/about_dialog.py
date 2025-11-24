"""
关于对话框模块

显示应用程序信息、版本、作者等
"""

from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QLabel,
    QPushButton, QTextEdit, QTabWidget, QWidget
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont, QPixmap


class AboutDialog(QDialog):
    """关于对话框"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("关于 SimpleSensorSync")
        self.setModal(True)
        self.setFixedSize(500, 400)

        self.init_ui()

    def init_ui(self):
        """初始化界面"""
        layout = QVBoxLayout(self)

        # 创建选项卡
        tab_widget = QTabWidget()

        # 关于标签页
        about_tab = self.create_about_tab()
        tab_widget.addTab(about_tab, "关于")

        # 系统信息标签页
        system_tab = self.create_system_tab()
        tab_widget.addTab(system_tab, "系统信息")

        # 许可证标签页
        license_tab = self.create_license_tab()
        tab_widget.addTab(license_tab, "许可证")

        layout.addWidget(tab_widget)

        # 确定按钮
        button_layout = QHBoxLayout()
        button_layout.addStretch()

        ok_button = QPushButton("确定")
        ok_button.clicked.connect(self.accept)
        ok_button.setDefault(True)

        button_layout.addWidget(ok_button)
        layout.addLayout(button_layout)

    def create_about_tab(self) -> QWidget:
        """创建关于标签页"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # 应用程序图标和名称
        title_layout = QHBoxLayout()

        # 这里可以添加应用程序图标
        # icon_label = QLabel()
        # icon_label.setPixmap(QPixmap(":/icons/app_icon.png"))
        # title_layout.addWidget(icon_label)

        title_info_layout = QVBoxLayout()

        app_name_label = QLabel("SimpleSensorSync")
        app_name_font = QFont()
        app_name_font.setBold(True)
        app_name_font.setPointSize(16)
        app_name_label.setFont(app_name_font)
        title_info_layout.addWidget(app_name_label)

        version_label = QLabel("版本 0.2.0")
        version_label.setStyleSheet("color: gray;")
        title_info_layout.addWidget(version_label)

        title_layout.addLayout(title_info_layout)
        title_layout.addStretch()

        layout.addLayout(title_layout)

        # 描述信息
        description = QLabel("""
SimpleSensorSync 是一个为机器人和传感器融合系统设计的
多传感器同步解决方案。

主要特性：
• 支持8个传感器设备的实时同步监控
• 基于ZeroMQ的高性能数据通信
• 精确的PTP网络时间同步
• PWM/PPS硬件触发信号支持
• 直观的PyQt6图形界面
• 实时触发信号可视化

适用于相机、激光雷达、IMU、GPS等各种传感器的
精确时间协调和数据同步。
        """)
        description.setWordWrap(True)
        description.setAlignment(Qt.AlignmentFlag.AlignTop)
        layout.addWidget(description)

        # 作者信息
        author_label = QLabel("开发团队: SimpleSensorSync Team")
        author_label.setStyleSheet("font-style: italic; color: #666;")
        layout.addWidget(author_label)

        layout.addStretch()
        return widget

    def create_system_tab(self) -> QWidget:
        """创建系统信息标签页"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        system_info = self.get_system_info()

        info_text = QTextEdit()
        info_text.setPlainText(system_info)
        info_text.setReadOnly(True)
        info_text.setFont(QFont("Consolas", 9))

        layout.addWidget(info_text)
        return widget

    def create_license_tab(self) -> QWidget:
        """创建许可证标签页"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        license_text = QTextEdit()
        license_text.setPlainText(self.get_license_text())
        license_text.setReadOnly(True)

        layout.addWidget(license_text)
        return widget

    def get_system_info(self) -> str:
        """获取系统信息"""
        import sys
        import platform
        try:
            import PyQt6
            pyqt_version = PyQt6.QtCore.PYQT_VERSION_STR
        except:
            pyqt_version = "未知"

        try:
            import zmq
            zmq_version = zmq.zmq_version()
        except:
            zmq_version = "未知"

        try:
            import numpy
            numpy_version = numpy.__version__
        except:
            numpy_version = "未知"

        try:
            import pyqtgraph
            pyqtgraph_version = pyqtgraph.__version__
        except:
            pyqtgraph_version = "未知"

        info = f"""SimpleSensorSync 系统信息

应用程序信息:
  版本: 0.2.0
  构建日期: 2025-09-17

Python 环境:
  Python 版本: {sys.version}
  Python 路径: {sys.executable}

系统信息:
  操作系统: {platform.system()} {platform.release()}
  架构: {platform.machine()}
  处理器: {platform.processor()}

依赖库版本:
  PyQt6: {pyqt_version}
  ZeroMQ: {zmq_version}
  NumPy: {numpy_version}
  PyQtGraph: {pyqtgraph_version}

硬件支持:
  同步板版本: V3/V4/MINI
  支持传感器: 相机、激光雷达、IMU、GPS
  通信协议: ZeroMQ, PTP, PWM/PPS
"""
        return info

    def get_license_text(self) -> str:
        """获取许可证文本"""
        return """MIT License

Copyright (c) 2025 SimpleSensorSync Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

第三方库许可证:

PyQt6: GPL v3 License
ZeroMQ: LGPL License
NumPy: BSD License
PyQtGraph: MIT License

本软件使用的所有第三方库均遵循其各自的开源许可证。
"""