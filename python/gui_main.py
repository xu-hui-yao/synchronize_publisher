#!/usr/bin/env python3
"""
SimpleSensorSync GUI 启动脚本

启动多传感器同步监控的图形界面应用程序
"""

import sys
from pathlib import Path

# 添加项目路径
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))

from PyQt6.QtWidgets import QApplication
from PyQt6.QtGui import QIcon

from infinite_sense_core.gui import MainWindow


def create_application() -> QApplication:
    """创建QApplication实例"""
    app = QApplication(sys.argv)

    # 设置应用程序属性
    app.setApplicationName("SimpleSensorSync")
    app.setApplicationVersion("0.2.0")
    app.setApplicationDisplayName("SimpleSensorSync")
    app.setOrganizationName("InfiniteSenseLab")
    app.setOrganizationDomain("https://github.com/InfiniteSenseLab")

    # 设置应用程序图标（如果存在）
    icon_path = project_root / "infinite_sense_core" / "gui" / "resources" / "icons" / "app_icon.png"
    if icon_path.exists():
        app.setWindowIcon(QIcon(str(icon_path)))

    return app


def main():
    """主函数"""
    print("启动 SimpleSensorSync GUI...")

    try:
        # 创建应用程序
        app = create_application()

        # 创建主窗口
        main_window = MainWindow()

        # 显示主窗口
        main_window.show()
        print("主窗口已显示")

        # 运行应用程序
        print("应用程序开始运行")
        exit_code = app.exec()

        print(f"应用程序退出，退出码: {exit_code}")
        return exit_code

    except Exception as e:
        print(f"应用程序启动失败: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())