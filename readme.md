1. 运行雷达驱动

进入：/home/xuhuiyao/Desktop/workfield/sync/mid360 目录

环境：source install/setup.bash

运行驱动：ros2 launch livox_ros_driver2 rviz_MID360_launch.py


2. 运行SDK程序

sudo chmod 777 /dev/ttyACM0

如果没有检查接线是否正确

进入目录：/home/xuhuiyao/Desktop/workfield/sync/SimpleSensorSync/build

编译： make -j

运行：./example/NetCam/net_cam_ros2


正常运行输出：
infinite_sense.cpp:11 
  ▗▄▄▄▖▗▖  ▗▖▗▄▄▄▖▗▄▄▄▖▗▖  ▗▖▗▄▄▄▖▗▄▄▄▖▗▄▄▄▖ ▗▄▄▖▗▄▄▄▖▗▖  ▗▖ ▗▄▄▖▗▄▄▄▖
    █  ▐▛▚▖▐▌▐▌     █  ▐▛▚▖▐▌  █    █  ▐▌   ▐▌   ▐▌   ▐▛▚▖▐▌▐▌   ▐▌   
    █  ▐▌ ▝▜▌▐▛▀▀▘  █  ▐▌ ▝▜▌  █    █  ▐▛▀▀▘ ▝▀▚▖▐▛▀▀▘▐▌ ▝▜▌ ▝▀▚▖▐▛▀▀▘
  ▗▄█▄▖▐▌  ▐▌▐▌   ▗▄█▄▖▐▌  ▐▌▗▄█▄▖  █  ▐▙▄▄▖▗▄▄▞▘▐▙▄▄▖▐▌  ▐▌▗▄▄▞▘▐▙▄▄▖
infinite_sense.cpp:18 ZeroMQ version: 4.3.4
usb.cpp:18 Serial port /dev/ttyACM0 opened successfully.
usb.cpp:45 USB manager started.
usb.cpp:80 Received malformed JSON: t":0,"c":0}

usb.cpp:80 Received malformed JSON: {"f":"t","s":0,"t"f":"t","s":0,"t":0,"c":0}

messenger.cpp:11 Using new ZMQ version: 4.3.4
messenger.cpp:20 Link Net: tcp://127.0.0.1:4565
mv_cam.cpp:77 -------------------------------------------------------------
mv_cam.cpp:78 ----------------------Camera Device Info---------------------
mv_cam.cpp:88   Device Model Name    : MV-CU013-A0UC
mv_cam.cpp:89   User Defined Name    : cam_1
mv_cam.cpp:94 -------------------------------------------------------------
mv_cam.cpp:77 -------------------------------------------------------------
mv_cam.cpp:78 ----------------------Camera Device Info---------------------
mv_cam.cpp:88   Device Model Name    : MV-CU013-A0UC
mv_cam.cpp:89   User Defined Name    : cam_3
mv_cam.cpp:94 -------------------------------------------------------------
mv_cam.cpp:77 -------------------------------------------------------------
mv_cam.cpp:78 ----------------------Camera Device Info---------------------
mv_cam.cpp:88   Device Model Name    : MV-CU013-A0UC
mv_cam.cpp:89   User Defined Name    : cam_2
mv_cam.cpp:94 -------------------------------------------------------------
mv_cam.cpp:131 Number of cameras detected : 3
mv_cam.cpp:176 Camera 0 opens to completion.
mv_cam.cpp:176 Camera 1 opens to completion.
mv_cam.cpp:176 Camera 2 opens to completion.
mv_cam.cpp:318 Camera name is cam_1
mv_cam.cpp:321 Camera cam_1 start.
mv_cam.cpp:318 Camera name is cam_3
mv_cam.cpp:321 Camera cam_3 start.
mv_cam.cpp:318 Camera name is cam_2
mv_cam.cpp:321 Camera cam_2 start.
infinite_sense.cpp:46 Synchronizer Started.


3. 时间同步验证
ros2 topic echo /cam_1 |grep sec
ros2 topic echo /livox/lidar |grep sec


4. 相机topic 名称在 /home/xuhuiyao/Desktop/workfield/sync/SimpleSensorSync/example/NetCam/main_ros2.cpp 文件的，17-20行修改

相机频率配置：

打开串口工具，发送下面数据，修改hz_cam_1等后面对应的数字，然后重新上电即可。

{"f":"cfg","port":8888,"ip":[192,168,1,188],"subnet":[255,255,255,0],"hz_cam_1":10,"hz_cam_2":10,"hz_cam_3":10,"hz_cam_4":10,"hz_imu_2":10,"xtal_diff":0,"uart_0_baud_rate":921600,"uart_1_baud_rate":9600,"uart_2_baud_rate":115200,"use_gps":true,"use_pps":true,"version":400}\n





