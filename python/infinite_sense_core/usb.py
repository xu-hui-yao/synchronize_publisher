import json
import threading

import serial

from .data import process_trigger_data, process_imu_data, process_gps_data, process_log_data
from .ptp import Ptp
from .times import precise_sleep

class UsbManager:
    def __init__(self, port: str, baud_rate: int):
        self.port = port
        self.started = False
        self.serial_ptr = None
        self.ptp = None
        self.rx_thread = None
        self.tx_thread = None

        try:
            self.serial_ptr = serial.Serial(
                port=port,
                baudrate=baud_rate,
                timeout=1
            )
            if self.serial_ptr.is_open:
                self.serial_ptr.reset_input_buffer()
                self.serial_ptr.reset_output_buffer()
                print(f"[INFO] Serial port {port} opened successfully.")
            else:
                print(f"[ERROR] Failed to open serial port: {port}")
                return

            self.ptp = Ptp()
            self.ptp.set_usb_ptr(self.serial_ptr)

        except serial.SerialException as e:
            print(f"[ERROR] Serial exception on port {port}: {e}")
            if self.serial_ptr and self.serial_ptr.is_open:
                self.serial_ptr.close()

    def start(self):
        if not self.serial_ptr or not self.serial_ptr.is_open:
            print("[ERROR] Cannot start USB manager: Serial port not open.")
            return

        self.started = True
        self.rx_thread = threading.Thread(target=self.receive, daemon=True)
        self.tx_thread = threading.Thread(target=self.time_stamp_synchronization, daemon=True)
        self.rx_thread.start()
        self.tx_thread.start()
        print("[INFO] USB manager started.")

    def stop(self):
        if not self.started:
            return

        self.started = False
        if self.rx_thread and self.rx_thread.is_alive():
            self.rx_thread.join()
        if self.tx_thread and self.tx_thread.is_alive():
            self.tx_thread.join()
        if self.serial_ptr and self.serial_ptr.is_open:
            self.serial_ptr.close()
            print(f"[INFO] Serial port {self.port} closed.")
        print("[INFO] USB manager stopped.")

    def receive(self):
        while self.started:
            try:
                if not self.serial_ptr or not self.serial_ptr.is_open:
                    break

                if self.serial_ptr.in_waiting:
                    serial_recv = self.serial_ptr.readline().decode(errors="ignore").strip()
                    if not serial_recv:
                        print("[WARNING] Received empty string.")
                        continue

                    try:
                        json_data = json.loads(serial_recv)
                    except json.JSONDecodeError:
                        print(f"[WARNING] Received malformed JSON: {serial_recv}")
                        continue

                    self.ptp.receive_ptp_data(json_data)
                    process_trigger_data(json_data)
                    process_imu_data(json_data)
                    process_gps_data(json_data)
                    process_log_data(json_data)

                precise_sleep(0.001)  # 1ms
            except Exception as e:
                print(f"[ERROR] Receive thread exception: {e}")

    def time_stamp_synchronization(self):
        while self.started:
            try:
                self.ptp.send_ptp_data()
                precise_sleep(0.01)  # 10ms
            except Exception as e:
                print(f"[ERROR] Timestamp sync exception: {e}")
