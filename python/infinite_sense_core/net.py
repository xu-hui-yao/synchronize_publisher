import json
import socket
import struct
import threading
import time

from .data import process_trigger_data, process_imu_data, process_gps_data, process_log_data
from .times import precise_sleep
from .ptp import Ptp

class NetManager:
    def __init__(self, target_ip: str, port: int):
        self.target_ip = target_ip
        self.port = port
        self.started = False

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setblocking(False)

        curr_time = int(time.time() * 1_000_000)
        self.sock.sendto(struct.pack("<Q", curr_time), (self.target_ip, self.port))

        self.ptp = Ptp()
        self.ptp.set_net_ptr(self.sock, self.target_ip, self.port)

        self.rx_thread = None
        self.tx_thread = None

    def start(self):
        if self.started:
            return
        self.started = True
        self.rx_thread = threading.Thread(target=self.receive, daemon=True)
        self.tx_thread = threading.Thread(target=self.timestamp_synchronization, daemon=True)
        self.rx_thread.start()
        self.tx_thread.start()
        print("[NetManager] Started")

    def stop(self):
        if not self.started:
            return
        self.started = False
        print("[NetManager] Stopped")

    def receive(self):
        while self.started:
            try:
                data, addr = self.sock.recvfrom(65536)
            except BlockingIOError:
                precise_sleep(0.001)
                continue

            try:
                recv_data = data.decode("utf-8")
                if not recv_data:
                    continue
                json_data = json.loads(recv_data)
            except Exception as e:
                print(f"[WARN] JSON parse error: {e}")
                continue
            # print(json_data)
            self.ptp.receive_ptp_data(json_data)
            process_trigger_data(json_data)
            process_imu_data(json_data)
            process_gps_data(json_data)
            process_log_data(json_data)

    def timestamp_synchronization(self):
        while self.started:
            try:
                self.ptp.send_ptp_data()
                precise_sleep(0.01)
            except Exception as e:
                print(f"[ERROR] Timestamp sync error: {e}")


if __name__ == "__main__":
    nm = NetManager("192.168.1.188", 8888)
    nm.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        nm.stop()
