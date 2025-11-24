import zmq
import threading

class Messenger:
    _instance = None
    def __init__(self, endpoint="tcp://127.0.0.1:4565"):
        self.endpoint = endpoint
        self.context = zmq.Context()
        self.publisher = self.context.socket(zmq.PUB)
        self.sub_threads = []
        self.lock = threading.Lock()

        try:
            self.publisher.bind(self.endpoint)
            print(f"[INFO] Link Net: {self.endpoint}")
        except zmq.ZMQError as e:
            print(f"[ERROR] Net initialization error: {e}")
            self.cleanup()
        Messenger._instance = self

    @classmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = Messenger()
        return cls._instance

    def cleanup(self):
        try:
            self.publisher.close()
            self.context.term()
            for thread in self.sub_threads:
                if thread.is_alive():
                    thread.join()
            print("[INFO] Messenger clean up successful")
        except Exception:
            print("[ERROR] Messenger clean up error")

    def pub(self, topic: str, metadata: str):
        try:
            with self.lock:
                self.publisher.send_string(topic, zmq.SNDMORE)
                self.publisher.send_string(metadata, zmq.DONTWAIT)
        except zmq.ZMQError as e:
            print(f"[ERROR] Publish error: {e}")

    def sub(self, topic: str, callback):
        def run():
            try:
                subscriber = self.context.socket(zmq.SUB)
                subscriber.connect(self.endpoint)
                subscriber.setsockopt_string(zmq.SUBSCRIBE, topic)
                while True:
                    topic_msg = subscriber.recv_string()
                    data_msg = subscriber.recv_string()
                    if topic_msg != topic:
                        continue
                    callback(data_msg.encode(), len(data_msg))
            except zmq.ZMQError as e:
                print(f"[ERROR] Exception in Sub thread for topic [{topic}]: {e}")

        thread = threading.Thread(target=run, daemon=True)
        thread.start()
        self.sub_threads.append(thread)

