import json

from .times import get_current_time_us

class Ptp:
    FUNC_NAME = "f"
    FUNC_TYPE_A = "a"
    FUNC_TYPE_B = "b"

    def __init__(self):
        self.serial_ptr = None
        self.net_ptr = None
        self.target_ip = None
        self.port = None

        self.time_t1 = 0
        self.time_t2 = 0
        self.updated_t1_t2 = False

    def set_usb_ptr(self, serial_ptr):
        self.serial_ptr = serial_ptr

    def set_net_ptr(self, net_ptr, target_ip: str, port: int):
        self.net_ptr = net_ptr
        self.target_ip = target_ip
        self.port = port

    def receive_ptp_data(self, data: dict):
        try:
            func = data[self.FUNC_NAME]

            if func == self.FUNC_TYPE_A:
                self.handle_time_sync_request(data)
            elif func == self.FUNC_TYPE_B:
                self.handle_time_sync_response(data)
        except Exception as e:
            print(f"[ERROR] PTP JSON parse error: {e}")

    def handle_time_sync_request(self, data: dict):
        try:
            self.time_t1 = data[self.FUNC_TYPE_A]
            self.time_t2 = data[self.FUNC_TYPE_B]
            self.updated_t1_t2 = True
        except Exception as e:
            print(f"[ERROR] Invalid 'a' message format: {e}")

    def handle_time_sync_response(self, data: dict):
        try:
            t3 = data[self.FUNC_TYPE_A]
            t4 = get_current_time_us()

            if self.updated_t1_t2:
                delay = int((t4 - t3 + self.time_t2 - self.time_t1) / 2)
                offset = int((self.time_t2 - self.time_t1 - t4 + t3) / 2)

                response = {
                    self.FUNC_NAME: self.FUNC_TYPE_B,
                    self.FUNC_TYPE_A: delay,
                    self.FUNC_TYPE_B: offset
                }

                self.send_json(response)
                self.updated_t1_t2 = False
        except Exception as e:
            print(f"[ERROR] Invalid 'b' message format: {e}")

    def send_ptp_data(self):
        mark = get_current_time_us()

        data = {
            self.FUNC_NAME: self.FUNC_TYPE_A,
            self.FUNC_TYPE_A: mark
        }
        self.send_json(data)

    def send_json(self, data: dict):
        out = json.dumps(data) + "\n"
        if self.net_ptr:
            self.net_ptr.sendto(out.encode(), (self.target_ip, self.port))
        elif self.serial_ptr:
            self.serial_ptr.write(out.encode())
        else:
            print("[WARNING] No valid output interface for PTP message.")
