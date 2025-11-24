"""
UI工具模块

提供通用的UI组件和工具函数
支持布局、对话框、消息提示等功能
"""

from PyQt6.QtWidgets import (
    QWidget, QMessageBox, QFileDialog, QProgressDialog,
    QHBoxLayout, QVBoxLayout, QLabel, QPushButton,
    QFrame, QSizePolicy, QApplication
)
from PyQt6.QtCore import Qt, QTimer, QThread, pyqtSignal
from PyQt6.QtGui import QFont, QPixmap, QIcon
from typing import Optional, Callable, Any
import os


class UIUtils:
    """UI工具类"""

    @staticmethod
    def show_info_message(parent: QWidget, title: str, message: str):
        """显示信息消息框"""
        QMessageBox.information(parent, title, message)

    @staticmethod
    def show_warning_message(parent: QWidget, title: str, message: str):
        """显示警告消息框"""
        QMessageBox.warning(parent, title, message)

    @staticmethod
    def show_error_message(parent: QWidget, title: str, message: str):
        """显示错误消息框"""
        QMessageBox.critical(parent, title, message)

    @staticmethod
    def show_question_dialog(parent: QWidget, title: str, message: str) -> bool:
        """显示确认对话框"""
        reply = QMessageBox.question(
            parent, title, message,
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No
        )
        return reply == QMessageBox.StandardButton.Yes

    @staticmethod
    def select_file_dialog(
        parent: QWidget,
        title: str = "选择文件",
        filter_str: str = "所有文件 (*.*)",
        default_dir: str = ""
    ) -> Optional[str]:
        """文件选择对话框"""
        filename, _ = QFileDialog.getOpenFileName(
            parent, title, default_dir, filter_str
        )
        return filename if filename else None

    @staticmethod
    def save_file_dialog(
        parent: QWidget,
        title: str = "保存文件",
        filter_str: str = "所有文件 (*.*)",
        default_name: str = ""
    ) -> Optional[str]:
        """文件保存对话框"""
        filename, _ = QFileDialog.getSaveFileName(
            parent, title, default_name, filter_str
        )
        return filename if filename else None

    @staticmethod
    def select_directory_dialog(
        parent: QWidget,
        title: str = "选择目录",
        default_dir: str = ""
    ) -> Optional[str]:
        """目录选择对话框"""
        directory = QFileDialog.getExistingDirectory(
            parent, title, default_dir
        )
        return directory if directory else None

    @staticmethod
    def create_horizontal_line() -> QFrame:
        """创建水平分割线"""
        line = QFrame()
        line.setFrameShape(QFrame.Shape.HLine)
        line.setFrameShadow(QFrame.Shadow.Sunken)
        return line

    @staticmethod
    def create_vertical_line() -> QFrame:
        """创建垂直分割线"""
        line = QFrame()
        line.setFrameShape(QFrame.Shape.VLine)
        line.setFrameShadow(QFrame.Shadow.Sunken)
        return line

    @staticmethod
    def create_spacer(width: int = 0, height: int = 0) -> QWidget:
        """创建空白间距组件"""
        spacer = QWidget()
        if width > 0:
            spacer.setFixedWidth(width)
        if height > 0:
            spacer.setFixedHeight(height)
        spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        return spacer

    @staticmethod
    def create_title_label(text: str, size: int = 12, bold: bool = True) -> QLabel:
        """创建标题标签"""
        label = QLabel(text)
        font = QFont()
        font.setPointSize(size)
        font.setBold(bold)
        label.setFont(font)
        return label

    @staticmethod
    def create_button_with_icon(
        text: str,
        icon_path: str = "",
        size: tuple = (100, 30)
    ) -> QPushButton:
        """创建带图标的按钮"""
        button = QPushButton(text)
        if icon_path and os.path.exists(icon_path):
            button.setIcon(QIcon(icon_path))
        if size:
            button.setFixedSize(*size)
        return button

    @staticmethod
    def set_widget_style(widget: QWidget, style_dict: dict):
        """设置组件样式"""
        style_parts = []
        for property_name, value in style_dict.items():
            style_parts.append(f"{property_name}: {value};")

        widget.setStyleSheet(" ".join(style_parts))

    @staticmethod
    def center_widget(widget: QWidget, parent: Optional[QWidget] = None):
        """居中显示组件"""
        if parent:
            # 相对于父组件居中
            parent_rect = parent.geometry()
            widget_rect = widget.geometry()
            x = parent_rect.x() + (parent_rect.width() - widget_rect.width()) // 2
            y = parent_rect.y() + (parent_rect.height() - widget_rect.height()) // 2
            widget.move(x, y)
        else:
            # 相对于屏幕居中
            screen = QApplication.primaryScreen().geometry()
            widget_rect = widget.geometry()
            x = (screen.width() - widget_rect.width()) // 2
            y = (screen.height() - widget_rect.height()) // 2
            widget.move(x, y)

    @staticmethod
    def animate_widget_opacity(
        widget: QWidget,
        start_opacity: float,
        end_opacity: float,
        duration: int = 300
    ):
        """组件透明度动画"""
        from PyQt6.QtCore import QPropertyAnimation
        from PyQt6.QtWidgets import QGraphicsOpacityEffect

        effect = QGraphicsOpacityEffect()
        widget.setGraphicsEffect(effect)

        animation = QPropertyAnimation(effect, b"opacity")
        animation.setDuration(duration)
        animation.setStartValue(start_opacity)
        animation.setEndValue(end_opacity)
        animation.start()

        return animation

    @staticmethod
    def create_loading_dialog(parent: QWidget, title: str = "正在处理...") -> QProgressDialog:
        """创建加载对话框"""
        progress = QProgressDialog(parent)
        progress.setWindowTitle(title)
        progress.setLabelText("请稍候...")
        progress.setRange(0, 0)  # 不确定进度
        progress.setModal(True)
        progress.setCancelButton(None)  # 不显示取消按钮
        return progress


class StatusIndicator(QWidget):
    """状态指示器组件"""

    def __init__(self, size: int = 16):
        super().__init__()
        self.size = size
        self.status = "unknown"  # unknown, ok, warning, error
        self.setFixedSize(size, size)

    def set_status(self, status: str):
        """设置状态"""
        self.status = status
        self.update()

    def paintEvent(self, event):
        """绘制事件"""
        from PyQt6.QtGui import QPainter, QBrush, QColor

        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # 根据状态选择颜色
        colors = {
            "ok": QColor(0, 255, 0),      # 绿色
            "warning": QColor(255, 165, 0),  # 橙色
            "error": QColor(255, 0, 0),    # 红色
            "unknown": QColor(128, 128, 128)  # 灰色
        }

        color = colors.get(self.status, colors["unknown"])
        brush = QBrush(color)
        painter.setBrush(brush)
        painter.setPen(color.darker())

        # 绘制圆形
        painter.drawEllipse(2, 2, self.size - 4, self.size - 4)


class AsyncWorker(QThread):
    """异步工作线程"""

    finished = pyqtSignal(object)  # 完成信号
    error = pyqtSignal(str)        # 错误信号
    progress = pyqtSignal(int)     # 进度信号

    def __init__(self, func: Callable, *args, **kwargs):
        super().__init__()
        self.func = func
        self.args = args
        self.kwargs = kwargs

    def run(self):
        """运行线程"""
        try:
            result = self.func(*self.args, **self.kwargs)
            self.finished.emit(result)
        except Exception as e:
            self.error.emit(str(e))


class NotificationWidget(QWidget):
    """通知组件"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.init_ui()
        self.hide()

    def init_ui(self):
        """初始化界面"""
        layout = QHBoxLayout(self)
        layout.setContentsMargins(10, 5, 10, 5)

        self.icon_label = QLabel()
        self.message_label = QLabel()
        self.close_button = QPushButton("×")
        self.close_button.setFixedSize(20, 20)
        self.close_button.clicked.connect(self.hide)

        layout.addWidget(self.icon_label)
        layout.addWidget(self.message_label, 1)
        layout.addWidget(self.close_button)

        # 设置样式
        self.setStyleSheet("""
            NotificationWidget {
                background-color: #f0f8ff;
                border: 1px solid #0078d4;
                border-radius: 5px;
            }
        """)

    def show_notification(
        self,
        message: str,
        notification_type: str = "info",
        duration: int = 3000
    ):
        """显示通知"""
        self.message_label.setText(message)

        # 设置图标和样式
        type_styles = {
            "info": {"color": "#0078d4", "icon": "ℹ"},
            "success": {"color": "#00a86b", "icon": "✓"},
            "warning": {"color": "#ff9500", "icon": "⚠"},
            "error": {"color": "#e74c3c", "icon": "✗"}
        }

        style_info = type_styles.get(notification_type, type_styles["info"])
        self.icon_label.setText(style_info["icon"])
        self.icon_label.setStyleSheet(f"color: {style_info['color']}; font-weight: bold;")

        self.show()

        # 自动隐藏
        if duration > 0:
            QTimer.singleShot(duration, self.hide)