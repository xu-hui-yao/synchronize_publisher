<p align="center">
<img  style="width:50%;"  alt="Logo" src="assets/picture/main_logo.png">
<br>
<em>Stable · Easy to Use · Precise</em>
<br>
</p>
<p align="center">
<a href="README_EN.md">English</a>
</p>

---

# 🚀 [A Simple and Easy-to-Use Multi-Sensor Synchronization Solution](https://github.com/InfiniteSenseLab/SimpleSensorSync/wiki)！  
   Multi-sensor time synchronization is a critical issue, especially for sensor fusion systems. Incorrect time alignment can cause data fusion errors and negatively affect system performance. For most researchers, this is a low-level and complex issue that is not their research focus. More attention should be paid to designing sensor fusion algorithms rather than struggling with time synchronization. That’s why we designed this system — to make time synchronization no longer a hassle.  
   
---
✨ Minimal dependencies – Reduce compilation overhead for faster builds.  
🤖 ROS2 & Python support – Easily integrate into modern robotics and scripting workflows.  
⏱ More accurate synchronization mechanism – Provides higher precision time coordination.  
📡 Transparent data protocol (JSON) – Clearer and more flexible communication.  
⚙️ Simpler configuration – Easy to get started, more convenient customization. See [Quick Start & System Guide](https://github.com/InfiniteSenseLab/SimpleSensorSync/wiki).  
📜 Enhanced logging – More comprehensive records, more efficient debugging.  
🌐 Flexible multi-platform deployment – (ZeroMQ) supports embedded/desktop/cloud deployments.  
🔗 Support for multiple cameras 📷, LiDARs ⦿, IMUs 🧭 and GPS 🛰 for mixed signal coordination.  
🔄 [Supports multiple sync boards](assets/doc/board_introduction.md) - V3/V4/MINI.  
🛡️ Safe and reliable – Safer power and wiring design 🚫.

# News

>1. Latest hardware now supports the RealSense camera series.  
>2. Complete [User Manual & System Guide](https://github.com/InfiniteSenseLab/SimpleSensorSync/wiki) released.

<p align="center">
  <img alt="Image 1" src="assets/picture/v4_board.png" width="45%">
  &nbsp;&nbsp;&nbsp;
  <img alt="Image 2" src="assets/picture/link/all_sensor.png" width="45%">
</p>

# Supported Devices

>| Device Type      | Brand(s)                          | Sync Method |
>|------------------|-----------------------------------|-------------|
>| Depth Camera     | RealSense Series                  | PWM         |
>| Industrial Camera (Ethernet) | Hikvision/Dahua/Daheng/Jinghang/... | PWM         |
>| Industrial Camera (USB)      | Hikvision/Dahua/Daheng/Jinghang/... | PWM         |
>| Third-party IMU   | Xsens Series/...                  | PWM         |
>| 3D LiDAR          | Mid360/Mid70/RoboSense/Tele-15/Horizon/... | PPS         |
>| RTK/GPS           | All devices supporting NMEA0183   | NMEA        |
>| Host (ARM/X86)    | Intel/AMD/Jetson/RockChip/...     | PTP         |

# Contact

[【Taobao】Access denied MF3543 「Multi-camera IMU sync board with Ethernet/Serial support for industrial cameras and 6-axis pose」  
Click the link to open directly or search on Taobao](https://item.taobao.com/item.htm?ft=t&id=832624497202)

# Thanks  

Our sync board has already been used by more and more users 🎉  
We know it's not perfect yet 🙇‍♂️  
You're welcome to give suggestions and create Issues 🛠️📝  
Let’s make it better together! If you find it helpful, don’t forget to leave us a ⭐ to support us ❤️  
