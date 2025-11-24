import time


def precise_sleep(target_delay_s):
    start = time.perf_counter()
    end = start + target_delay_s

    while True:
        now = time.perf_counter()
        remaining = end - now
        if remaining <= 0.0002:
            break
        time.sleep(remaining / 2)

    while time.perf_counter() < end:
        pass


def get_current_time_us() -> int:
    return int(time.time() * 1_000_000)


if __name__ == "__main__":
    delays = []
    for _ in range(10):
        t0 = time.perf_counter()
        precise_sleep(0.001)  # 1 ms
        t1 = time.perf_counter()
        delays.append((t1 - t0) * 1e6)  # 转为微秒

    for i, d in enumerate(delays):
        print(f"第{i + 1}次延时: {d:.1f} μs")
