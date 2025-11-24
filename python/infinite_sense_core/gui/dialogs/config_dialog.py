"""
配置对话框模块

提供网络和串口连接参数配置界面
支持配置验证和参数保存功能
"""

from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QFormLayout,
    QLabel, QLineEdit, QSpinBox, QComboBox, QGroupBox,
    QPushButton, QRadioButton, QButtonGroup,
    QTabWidget, QWidget
)
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QFont
import serial.tools.list_ports


class NetworkConfigWidget(QWidget):
    """网络配置组件"""

    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        """初始化界面"""
        layout = QFormLayout(self)
        layout.setContentsMargins(15, 15, 15, 15)
        layout.setVerticalSpacing(12)
        layout.setHorizontalSpacing(10)
        layout.setLabelAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)

        # IP地址
        self.ip_edit = QLineEdit("192.168.1.188")
        self.ip_edit.setPlaceholderText("例如: 192.168.1.100")
        self.ip_edit.setMinimumHeight(28)
        layout.addRow("IP地址:", self.ip_edit)

        # 端口
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65535)
        self.port_spin.setValue(8888)
        self.port_spin.setMinimumHeight(28)
        layout.addRow("端口:", self.port_spin)

        # PTP配置
        self.ptp_interface_edit = QLineEdit("eth0")
        self.ptp_interface_edit.setPlaceholderText("网络接口名称")
        self.ptp_interface_edit.setMinimumHeight(28)
        layout.addRow("PTP接口:", self.ptp_interface_edit)

    def get_config(self) -> dict:
        """获取网络配置"""
        return {
            'net_ip': self.ip_edit.text(),
            'net_port': self.port_spin.value(),
            'ptp_interface': self.ptp_interface_edit.text()
        }

    def set_config(self, config: dict):
        """设置网络配置"""
        self.ip_edit.setText(config.get('net_ip', '192.168.1.188'))
        self.port_spin.setValue(config.get('net_port', 8888))
        self.ptp_interface_edit.setText(config.get('ptp_interface', 'eth0'))

    def validate(self) -> tuple[bool, str]:
        """验证配置"""
        if not self.ip_edit.text().strip():
            return False, "IP地址不能为空"
        return True, ""


class SerialConfigWidget(QWidget):
    """串口配置组件"""

    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        """初始化界面"""
        layout = QFormLayout(self)
        layout.setContentsMargins(15, 15, 15, 15)
        layout.setVerticalSpacing(12)
        layout.setHorizontalSpacing(10)
        layout.setLabelAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)

        # 串口设备和刷新按钮
        device_layout = QHBoxLayout()
        device_layout.setSpacing(8)

        self.device_combo = QComboBox()
        self.device_combo.setMinimumHeight(28)
        self.refresh_serial_ports()

        refresh_button = QPushButton("刷新")
        refresh_button.setMinimumSize(60, 28)
        refresh_button.clicked.connect(self.refresh_serial_ports)

        device_layout.addWidget(self.device_combo, 1)
        device_layout.addWidget(refresh_button)
        layout.addRow("串口设备:", device_layout)

        # 波特率
        self.baudrate_combo = QComboBox()
        self.baudrate_combo.addItems(['9600', '19200', '38400', '57600', '115200', '230400', '460800', '921600'])
        self.baudrate_combo.setCurrentText('115200')
        self.baudrate_combo.setMinimumHeight(28)
        layout.addRow("波特率:", self.baudrate_combo)

        # 高级设置区域
        advanced_layout = QHBoxLayout()
        advanced_layout.setSpacing(10)

        # 数据位
        self.databits_combo = QComboBox()
        self.databits_combo.addItems(['5', '6', '7', '8'])
        self.databits_combo.setCurrentText('8')
        self.databits_combo.setMinimumHeight(28)

        # 停止位
        self.stopbits_combo = QComboBox()
        self.stopbits_combo.addItems(['1', '1.5', '2'])
        self.stopbits_combo.setCurrentText('1')
        self.stopbits_combo.setMinimumHeight(28)

        # 校验位
        self.parity_combo = QComboBox()
        self.parity_combo.addItems(['None', 'Even', 'Odd', 'Mark', 'Space'])
        self.parity_combo.setCurrentText('None')
        self.parity_combo.setMinimumHeight(28)

        advanced_layout.addWidget(QLabel("数据位:"))
        advanced_layout.addWidget(self.databits_combo)
        advanced_layout.addWidget(QLabel("停止位:"))
        advanced_layout.addWidget(self.stopbits_combo)
        advanced_layout.addWidget(QLabel("校验位:"))
        advanced_layout.addWidget(self.parity_combo)

        layout.addRow("高级设置:", advanced_layout)

    def refresh_serial_ports(self):
        """刷新串口列表"""
        self.device_combo.clear()
        ports = serial.tools.list_ports.comports()

        for port in ports:
            self.device_combo.addItem(f"{port.device} - {port.description}", port.device)

        if self.device_combo.count() == 0:
            self.device_combo.addItem("未找到串口设备", "")

    def get_config(self) -> dict:
        """获取串口配置"""
        return {
            'serial_device': self.device_combo.currentData(),
            'serial_baudrate': int(self.baudrate_combo.currentText()),
            'serial_databits': int(self.databits_combo.currentText()),
            'serial_stopbits': float(self.stopbits_combo.currentText()),
            'serial_parity': self.parity_combo.currentText()
        }

    def set_config(self, config: dict):
        """设置串口配置"""
        device = config.get('serial_device', '')
        # 尝试找到对应的设备
        index = self.device_combo.findData(device)
        if index >= 0:
            self.device_combo.setCurrentIndex(index)

        self.baudrate_combo.setCurrentText(str(config.get('serial_baudrate', 115200)))
        self.databits_combo.setCurrentText(str(config.get('serial_databits', 8)))
        self.stopbits_combo.setCurrentText(str(config.get('serial_stopbits', 1)))
        self.parity_combo.setCurrentText(config.get('serial_parity', 'None'))

    def validate(self) -> tuple[bool, str]:
        """验证配置"""
        if not self.device_combo.currentData():
            return False, "请选择串口设备"
        return True, ""


class ConfigDialog(QDialog):
    """配置对话框主类"""

    config_applied = pyqtSignal(dict)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("连接配置")
        self.setModal(True)
        self.resize(400, 350)

        self.connection_type = 'network'  # 默认网络连接

        self.init_ui()

    def init_ui(self):
        """初始化界面"""
        layout = QVBoxLayout(self)
        # 连接类型选择
        self.create_connection_type_group(layout)

        # 配置选项卡
        self.create_config_tabs(layout)

        # 按钮区域
        self.create_button_area(layout)

    def create_connection_type_group(self, parent_layout):
        """创建连接类型选择组"""
        group = QGroupBox("连接类型")
        layout = QHBoxLayout(group)

        self.button_group = QButtonGroup()

        self.network_radio = QRadioButton("网络连接")
        self.network_radio.setChecked(True)
        self.network_radio.toggled.connect(self.on_connection_type_changed)

        self.serial_radio = QRadioButton("串口连接")
        self.serial_radio.toggled.connect(self.on_connection_type_changed)

        self.button_group.addButton(self.network_radio, 0)
        self.button_group.addButton(self.serial_radio, 1)

        layout.addWidget(self.network_radio)
        layout.addWidget(self.serial_radio)

        parent_layout.addWidget(group)

    def create_config_tabs(self, parent_layout):
        """创建配置选项卡"""
        self.tab_widget = QTabWidget()
        self.tab_widget.setObjectName("configTabWidget")
        self.tab_widget.setTabPosition(QTabWidget.TabPosition.North)
        self.tab_widget.setUsesScrollButtons(False)

        # 网络配置标签页
        self.network_config = NetworkConfigWidget()
        self.tab_widget.addTab(self.network_config, "网络设置")

        # 串口配置标签页
        self.serial_config = SerialConfigWidget()
        self.tab_widget.addTab(self.serial_config, "串口设置")

        parent_layout.addWidget(self.tab_widget)

    def create_button_area(self, parent_layout):
        """创建按钮区域"""
        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)

        # 确定按钮
        self.ok_button = QPushButton("确定")
        self.ok_button.clicked.connect(self.accept_config)
        self.ok_button.setDefault(True)
        self.ok_button.setMinimumSize(80, 32)
        self.ok_button.setObjectName("okButton")

        # 取消按钮
        self.cancel_button = QPushButton("取消")
        self.cancel_button.clicked.connect(self.reject)
        self.cancel_button.setMinimumSize(80, 32)
        self.cancel_button.setObjectName("cancelButton")

        button_layout.addStretch()
        button_layout.addWidget(self.ok_button)
        button_layout.addWidget(self.cancel_button)

        parent_layout.addLayout(button_layout)

    def apply_styles(self):
        """应用统一样式"""
        style = """
            /* 对话框样式 */
            QDialog {
                background-color: #f8f9fa;
                font-family: "Microsoft YaHei", "SimHei", Arial, sans-serif;
                font-size: 9pt;
            }

            /* 标题样式 */
            #titleLabel {
                color: #2c3e50;
                margin: 5px 0px 10px 0px;
                font-weight: bold;
            }

            /* 分组框样式 */
            #connectionTypeGroup {
                font-weight: bold;
                border: 1px solid #d1d5db;
                border-radius: 6px;
                margin: 5px 0px;
            }

            #connectionTypeGroup::title {
                color: #374151;
                subcontrol-origin: margin;
                left: 10px;
                padding: 0px 8px;
            }

            /* 单选按钮样式 */
            #networkRadio, #serialRadio {
                font-weight: normal;
                spacing: 8px;
            }

            #networkRadio::indicator, #serialRadio::indicator {
                width: 16px;
                height: 16px;
            }

            /* 选项卡样式 */
            #configTabWidget QTabBar::tab {
                background-color: #e5e7eb;
                border: 1px solid #d1d5db;
                border-bottom-color: transparent;
                border-top-left-radius: 6px;
                border-top-right-radius: 6px;
                padding: 8px 16px;
                margin-right: 2px;
                font-weight: normal;
            }

            #configTabWidget QTabBar::tab:selected {
                background-color: #ffffff;
                border-bottom-color: #ffffff;
                font-weight: bold;
            }

            #configTabWidget QTabBar::tab:hover {
                background-color: #f3f4f6;
            }

            #configTabWidget QTabWidget::pane {
                border: 1px solid #d1d5db;
                border-top-right-radius: 6px;
                border-bottom-left-radius: 6px;
                border-bottom-right-radius: 6px;
                background-color: #ffffff;
            }

            /* 输入控件样式 */
            QLineEdit, QSpinBox, QComboBox {
                border: 1px solid #d1d5db;
                border-radius: 4px;
                padding: 4px 8px;
                background-color: #ffffff;
                selection-background-color: #3b82f6;
            }

            QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
                border-color: #3b82f6;
                outline: none;
            }

            QComboBox::drop-down {
                border: none;
                width: 20px;
            }

            QComboBox::down-arrow {
                width: 12px;
                height: 12px;
            }

            /* 按钮样式 */
            #okButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 4px;
                font-weight: bold;
                padding: 6px 16px;
            }

            #okButton:hover {
                background-color: #2563eb;
            }

            #okButton:pressed {
                background-color: #1d4ed8;
            }

            #cancelButton {
                background-color: #6b7280;
                color: white;
                border: none;
                border-radius: 4px;
                font-weight: bold;
                padding: 6px 16px;
            }

            #cancelButton:hover {
                background-color: #4b5563;
            }

            #cancelButton:pressed {
                background-color: #374151;
            }

            QPushButton {
                background-color: #f3f4f6;
                border: 1px solid #d1d5db;
                border-radius: 4px;
                padding: 4px 12px;
                font-weight: normal;
            }

            QPushButton:hover {
                background-color: #e5e7eb;
                border-color: #9ca3af;
            }

            QPushButton:pressed {
                background-color: #d1d5db;
            }

            /* 标签样式 */
            QLabel {
                color: #374151;
                font-weight: normal;
            }
        """
        self.setStyleSheet(style)

    def on_connection_type_changed(self):
        """连接类型变化处理"""
        if self.network_radio.isChecked():
            self.connection_type = 'network'
            self.tab_widget.setCurrentIndex(0)
        else:
            self.connection_type = 'serial'
            self.tab_widget.setCurrentIndex(1)


    def accept_config(self):
        """接受配置"""
        self.accept()

    def get_config(self) -> dict:
        """获取完整配置"""
        config = {'connection_type': self.connection_type}

        if self.connection_type == 'network':
            config.update(self.network_config.get_config())
        else:
            config.update(self.serial_config.get_config())

        return config

    def set_config(self, config: dict):
        """设置配置"""
        connection_type = config.get('connection_type', 'network')

        if connection_type == 'network':
            self.network_radio.setChecked(True)
            self.network_config.set_config(config)
        else:
            self.serial_radio.setChecked(True)
            self.serial_config.set_config(config)