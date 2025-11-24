"""
触发状态指示器模块

LED风格的圆形指示器，显示设备触发状态
支持动画效果和自定义颜色
"""

from PyQt6.QtWidgets import QWidget
from PyQt6.QtCore import QTimer, Qt
from PyQt6.QtGui import QPainter, QColor, QBrush, QPen
from PyQt6.QtCore import QRectF


class TriggerIndicator(QWidget):
    """触发状态LED指示器"""

    def __init__(self, size: int = 24):
        super().__init__()
        self.size = size
        self.is_triggered = False
        self._opacity = 1.0

        # 颜色配置
        self.color_normal = QColor(128, 128, 128)      # 灰色 - 未触发
        self.color_triggered = QColor(0, 255, 0)       # 绿色 - 触发
        self.color_border = QColor(64, 64, 64)         # 边框颜色

        self.init_ui()
        self.init_animation()

    def init_ui(self):
        """初始化界面"""
        self.setFixedSize(self.size, self.size)
        self.setToolTip("设备触发状态指示器")

    def init_animation(self):
        """初始化动画"""
        # 闪烁动画
        self.blink_timer = QTimer()
        self.blink_timer.timeout.connect(self.reset_trigger_state)



    def set_triggered(self, triggered: bool):
        """设置触发状态"""
        if triggered and not self.is_triggered:
            self.is_triggered = True
            self.start_trigger_animation()
        elif not triggered:
            self.is_triggered = False
            self.update()

    def start_trigger_animation(self):
        """开始触发动画"""
        # 开始闪烁
        self.update()

        # 设置自动复位定时器
        self.blink_timer.start(300)  # 300ms后自动复位

    def reset_trigger_state(self):
        """重置触发状态"""
        self.blink_timer.stop()
        self.is_triggered = False
        self.update()

    def paintEvent(self, event):
        """绘制事件"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # 计算绘制区域
        rect = QRectF(2, 2, self.size - 4, self.size - 4)

        # 选择颜色
        if self.is_triggered:
            color = self.color_triggered
        else:
            color = self.color_normal

        # 应用透明度
        color.setAlphaF(self._opacity)

        # 绘制外边框
        pen = QPen(self.color_border, 1)
        painter.setPen(pen)

        # 绘制填充圆形
        brush = QBrush(color)
        painter.setBrush(brush)
        painter.drawEllipse(rect)

        # 如果是触发状态，添加高亮效果
        if self.is_triggered:
            # 内部高亮圆
            highlight_color = QColor(255, 255, 255, 100)
            highlight_brush = QBrush(highlight_color)
            painter.setBrush(highlight_brush)
            painter.setPen(Qt.PenStyle.NoPen)

            highlight_rect = QRectF(
                rect.x() + rect.width() * 0.2,
                rect.y() + rect.height() * 0.2,
                rect.width() * 0.6,
                rect.height() * 0.6
            )
            painter.drawEllipse(highlight_rect)

    def set_color_scheme(self, normal_color: QColor, triggered_color: QColor):
        """设置颜色方案"""
        self.color_normal = normal_color
        self.color_triggered = triggered_color
        self.update()

    def set_size(self, size: int):
        """设置指示器大小"""
        self.size = size
        self.setFixedSize(size, size)
        self.update()


