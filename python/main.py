import time
from infinite_sense_core.infinit_sense import Synchronizer
from infinite_sense_core.message import Messenger


class DemoSensor:
    def initialization(self):
        print("MockSensor initialized")

    def start(self):
        print("MockSensor started")

    def stop(self):
        print("MockSensor stopped")

def imu_callback(msg_bytes, size):
    print(f"Received IMU data of size {size}")

def image_callback(msg_bytes, size):
    print(f"Received image data of size {size}")

def main():
    synchronizer = Synchronizer()

    synchronizer.set_net_link("192.168.192.188", 8888)

    sensor = DemoSensor()

    synchronizer.use_sensor(sensor)

    synchronizer.start()

    messenger = Messenger.get_instance()
    messenger.sub("imu_1", imu_callback)
    messenger.sub("cam_1", image_callback)
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n检测到键盘中断，正在停止同步器...")

    synchronizer.stop()

if __name__ == "__main__":
    main()
