"""
对话框模块

包含配置和设置对话框：
- ConfigDialog: 连接配置对话框
- AboutDialog: 关于对话框
"""

from .config_dialog import ConfigDialog
from .about_dialog import AboutDialog

__all__ = ["ConfigDialog", "AboutDialog"]