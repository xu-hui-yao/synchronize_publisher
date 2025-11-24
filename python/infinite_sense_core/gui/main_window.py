"""
主窗口模块

SimpleSensorSync多传感器同步监控主界面
提供设备状态监控、触发信号可视化、配置管理等功能
"""

import sys
from typing import Optional

from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QMenuBar, QStatusBar, QSplitter, QApplication
)
from PyQt6.QtCore import QTimer, pyqtSignal, Qt
from PyQt6.QtGui import QAction, QIcon

from ..data import get_trigger_processor, TriggerEvent
from ..infinit_sense import Synchronizer
from .widgets.device_panel import DevicePanel
from .widgets.signal_chart import SignalChart
from .dialogs.config_dialog import ConfigDialog
from .dialogs.about_dialog import AboutDialog


class MainWindow(QMainWindow):
    """SimpleSensorSync主窗口"""

    # 信号定义
    trigger_received = pyqtSignal(object)  # TriggerEvent信号
    connection_status_changed = pyqtSignal(bool)  # 连接状态变化

    def __init__(self):
        super().__init__()
        self.synchronizer: Optional[Synchronizer] = None
        self._is_monitoring = False

        self.init_ui()
        self.init_connections()
        self.init_trigger_processor()

    def init_ui(self):
        """初始化用户界面"""
        self.setWindowTitle("SimpleSensorSync")
        self.setGeometry(100, 100, 1200, 800)

        # 创建菜单栏
        self.setup_menu_bar()

        # 创建中央部件
        self.setup_central_widget()

        # 创建状态栏
        self.setup_status_bar()

    def setup_menu_bar(self):
        """设置菜单栏"""
        menubar = self.menuBar()

        # 文件菜单
        file_menu = menubar.addMenu('设置(&F)')

        # 连接配置
        config_action = QAction('连接配置(&C)', self)
        config_action.setShortcut('Ctrl+C')
        config_action.triggered.connect(self.show_config_dialog)
        file_menu.addAction(config_action)

        file_menu.addSeparator()

        # 退出
        exit_action = QAction('退出(&X)', self)
        exit_action.setShortcut('Ctrl+Q')
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)

        # 监控菜单
        monitor_menu = menubar.addMenu('监控(&M)')

        self.start_action = QAction('开始监控(&S)', self)
        self.start_action.setShortcut('F5')
        self.start_action.triggered.connect(self.start_monitoring)
        monitor_menu.addAction(self.start_action)

        self.stop_action = QAction('停止监控(&T)', self)
        self.stop_action.setShortcut('F6')
        self.stop_action.setEnabled(False)
        self.stop_action.triggered.connect(self.stop_monitoring)
        monitor_menu.addAction(self.stop_action)

        # 帮助菜单
        help_menu = menubar.addMenu('帮助(&H)')

        about_action = QAction('关于(&A)', self)
        about_action.triggered.connect(self.show_about_dialog)
        help_menu.addAction(about_action)

    def setup_central_widget(self):
        """设置中央部件"""
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # 主布局
        main_layout = QHBoxLayout(central_widget)

        # 创建分割器
        splitter = QSplitter(Qt.Orientation.Horizontal)

        # 左侧设备面板
        self.device_panel = DevicePanel()
        splitter.addWidget(self.device_panel)

        # 右侧信号图表
        self.signal_chart = SignalChart()
        splitter.addWidget(self.signal_chart)

        # 设置分割比例
        splitter.setStretchFactor(0, 1)  # 设备面板
        splitter.setStretchFactor(1, 2)  # 信号图表

        main_layout.addWidget(splitter)

    def setup_status_bar(self):
        """设置状态栏"""
        self.status_bar = self.statusBar()
        self.status_bar.showMessage("就绪 - 等待连接配置")

    def init_connections(self):
        """初始化信号连接"""
        # 连接内部信号
        self.trigger_received.connect(self.on_trigger_received)
        self.connection_status_changed.connect(self.on_connection_status_changed)

        # 连接组件信号
        self.device_panel.device_selected.connect(self.signal_chart.highlight_device)

    def init_trigger_processor(self):
        """初始化触发处理器"""
        trigger_processor = get_trigger_processor()
        trigger_processor.add_callback(self.handle_trigger_event)

    def handle_trigger_event(self, event: TriggerEvent):
        """处理触发事件"""
        # 通过信号发送到主线程
        self.trigger_received.emit(event)

    def on_trigger_received(self, event: TriggerEvent):
        """触发事件处理槽函数"""
        # 更新设备面板
        self.device_panel.update_trigger_event(event)

        # 更新信号图表
        self.signal_chart.add_trigger_event(event)

        # 更新状态栏
        devices = ", ".join(event.triggered_devices)
        self.status_bar.showMessage(f"触发设备: {devices} | 时间: {event.timestamp_str}")

    def on_connection_status_changed(self, connected: bool):
        """连接状态变化处理"""
        if connected:
            self.status_bar.showMessage("已连接 - 监控中")
            self.start_action.setEnabled(False)
            self.stop_action.setEnabled(True)
        else:
            self.status_bar.showMessage("已断开连接")
            self.start_action.setEnabled(True)
            self.stop_action.setEnabled(False)

    def show_config_dialog(self):
        """显示配置对话框"""
        dialog = ConfigDialog(self)
        if dialog.exec() == ConfigDialog.DialogCode.Accepted:
            config = dialog.get_config()
            self.apply_config(config)

    def show_about_dialog(self):
        """显示关于对话框"""
        dialog = AboutDialog(self)
        dialog.exec()

    def apply_config(self, config: dict):
        """应用配置"""
        self.synchronizer = Synchronizer()

        if config.get('connection_type') == 'network':
            self.synchronizer.set_net_link(config['net_ip'], config['net_port'])
        elif config.get('connection_type') == 'serial':
            self.synchronizer.set_usb_link(config['serial_device'], config['serial_baudrate'])

        self.status_bar.showMessage("配置已应用 - 可以开始监控")
        self.start_action.setEnabled(True)

    def start_monitoring(self):
        """开始监控"""
        if not self.synchronizer:
            self.status_bar.showMessage("请先配置连接参数")
            return

        self.synchronizer.start()
        self._is_monitoring = True
        self.connection_status_changed.emit(True)

    def stop_monitoring(self):
        """停止监控"""
        if self.synchronizer and self._is_monitoring:
            self.synchronizer.stop()
            self._is_monitoring = False
            self.connection_status_changed.emit(False)

    def closeEvent(self, event):
        """窗口关闭事件"""
        if self._is_monitoring:
            self.stop_monitoring()
        event.accept()


def main():
    """主函数 - 启动应用程序"""
    app = QApplication(sys.argv)

    # 设置应用程序信息
    app.setApplicationName("SimpleSensorSync")
    app.setApplicationVersion("0.2.0")
    app.setOrganizationName("SimpleSensorSync Team")

    # 创建并显示主窗口
    window = MainWindow()
    window.show()

    # 运行应用程序
    sys.exit(app.exec())


if __name__ == "__main__":
    main()