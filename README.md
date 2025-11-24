<p align="center">
<img  style="width:50%;"  alt="Logo" src="assets/picture/main_logo.png">
<br>
<em>稳定 易用 精度</em>
<br>
</p>
<p align="center">
<a href="README_EN.md">English</a>
</p>

---

# 🚀 [一个简单易用的多传感器同步方案](https://github.com/InfiniteSenseLab/SimpleSensorSync/wiki)！  
   多传感器的时间同步是一个很重要的问题，尤其对多传感器融合系统。不正确的时间同步会导致数据融合错误，影响系统性能。 对大多数研究人员来说，这是一个底层又复杂的问题，却不是他们的研究方向。更多的精力应该放在设计传感器融合算法上，而不是在时间同步上。因此，我们设计了这样一个系统，让时间同步不再是一件难事。  
   
<p align="center">
  <img alt="Image 1" src="assets/picture/v4_board.png" width="45%">
  &nbsp;&nbsp;&nbsp;
  <img alt="Image 2" src="assets/picture/link/all_sensor.png" width="45%">
</p>   

---
✨ 精简依赖 – 降低编译开销，构建更快速。  
🤖 支持 ROS2 & Python – 轻松集成现代机器人与脚本化工作流。  
⏱ 更精准的同步机制 – 提供更高精度的时间协调。  
📡 数据协议更透明(JSON) – 通信更清晰、更灵活。  
⚙️ 配置更简单 – 轻松上手，自定义更便捷。  
📜 日志功能增强 – 记录更全面，调试更高效。   
🌐 多平台灵活部署 – (ZeroMQ)支持嵌入式/桌面/云端多场景部署。  
🔗 支持多相机 📷、多雷达⦿ 、IMU 🧭 与 GPS 🛰 的混合信号协同管理。  
🔄 支持多同步板 -V3/V4/MINI。  
🛡️ 安全可靠 – 更加安全的电源与接线🚫。

# News

>1. 最新硬件支持realsense系列相机。
>2. 完整的[使用说明与系统说明](https://github.com/InfiniteSenseLab/SimpleSensorSync/wiki)发布。
>3. Python-SDK发布，同步可视化工具发布。

<p align="center">
  <img alt="tool" src="https://github.com/user-attachments/assets/6787bf44-0433-4cee-9843-9e48ebab3e41" width="60%">
</p> 

# 支持设备

>| 设备类型        | 品牌                            |同步方式 |
>|-------------|-------------------------------|--------|
>| 深度相机    | RealSense系列              | PWM    |
>| 工业相机(网口)    | 海康/华睿/大恒/京航/PointGrey/Basler/...               | PWM    |
>| 工业相机(USB)   | 海康/华睿/大恒/京航/PointGrey/Basler/...               | PWM    |
>| 特殊相机   | OAK/...               | PWM    |
>| 第三方IMU      | Xsense系列/HiPNUC...                 | PWM    |
>| 3D激光        | Mid360/Mid70/RoboSense/Tele-15/Horizon系列/Ouster/...  | PPS   |
>| RTK/GPS/组合导航    | 所有支持NMEA0183设备                | NMEA   |
>| 主机(ARM/X86) | Intel/AMD/Jetson/RockChip/... | PTP    |


# 咨询

[【淘宝】Access denied MF3543 「多相机IMU同步板网口串口同步工业相机六轴姿态」
点击链接直接打开 或者 淘宝搜索直接打开](https://item.taobao.com/item.htm?ft=t&id=832624497202)

# 感谢  

我们的同步板已经被越来越多的小伙伴使用 🎉  
我们知道它还不完美 🙇‍♂️  
欢迎大家多提建议、创建 Issues 🛠️📝  
一起让它变得更好！如果觉得不错，别忘了点个 ⭐ 支持我们 ❤️  

