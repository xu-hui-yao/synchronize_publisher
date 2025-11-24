"""
图表工具模块

提供pyqtgraph图表的基本工具函数
"""

import pyqtgraph as pg
from typing import List


class ChartUtils:
    """图表工具类"""

    @staticmethod
    def create_color_map(device_count: int) -> List[str]:
        """创建设备颜色映射"""
        colors = [
            '#FF6B6B',  # 红色
            '#4ECDC4',  # 青色
            '#45B7D1',  # 蓝色
            '#96CEB4',  # 绿色
            '#FECA57',  # 黄色
            '#FF9FF3',  # 粉色
            '#54A0FF',  # 深蓝
            '#5F27CD'   # 紫色
        ]
        return colors[:device_count]

    @staticmethod
    def create_scatter_plot_item(
        color: str,
        size: int = 8,
        symbol: str = 'o'
    ) -> pg.ScatterPlotItem:
        """创建散点图项目"""
        return pg.ScatterPlotItem(
            size=size,
            pen=pg.mkPen(color, width=2),
            brush=pg.mkBrush(color),
            symbol=symbol
        )