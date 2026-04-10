# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SimpleSensorSync (infinite_sense) is a multi-sensor time synchronization framework combining a C++ core library with a Python SDK and GUI. It synchronizes cameras, IMUs, LiDARs, and GPS across embedded, desktop, and cloud platforms using either UDP network or serial (USB) communication, plus PTP (Precision Time Protocol) for clock alignment.

## Build Commands

### C++ Library & Examples

```bash
# Install ZeroMQ dependency (Ubuntu)
sudo apt-get install -y libzmq3-dev

# Configure and build
cmake -B build
cmake --build build

# Build specific target
cmake --build build --target video_cam
```

### Python SDK

```bash
cd python/

# Install with uv (preferred)
uv sync

# Run demo
python main.py

# Run GUI
python gui_main.py
```

### Python Code Quality

```bash
cd python/
ruff check .          # lint
black .               # format
mypy .                # type check
pytest                # tests
```

## Architecture

### C++ Core (`infinite_sense_core/`)

The library is built around a central `Synchronizer` class that owns and coordinates:

- **NetManager / UsbManager** — UDP and serial transport layers; each runs dedicated RX/TX threads
- **PTP** — four-message handshake (t1–t4) over either transport to compute clock offset and one-way delay
- **TriggerManager** (singleton) — tracks multi-device trigger events using an 8-bit device mask; broadcasts trigger readiness to subscribers
- **Messenger** (singleton, ZeroMQ PUB) — publishes all sensor data and triggers on `tcp://127.0.0.1:4565`; messages are `[topic] [json_payload]`
- **Sensor** (abstract base) — concrete camera/sensor implementations provide `Initialization()`, `Start()`, `Stop()`, `Receive()` and run in their own threads

### Python SDK (`python/infinite_sense_core/`)

Mirrors the C++ structure in pure Python:

| Module | Role |
|---|---|
| `infinit_sense.py` | `Synchronizer` wrapper — top-level entry point |
| `net.py` / `usb.py` | Network and serial managers |
| `ptp.py` | PTP protocol implementation |
| `data.py` | Data processing and trigger logic |
| `message.py` | ZeroMQ SUB subscriber |
| `times.py` | Timing utilities |
| `gui/` | PyQt6 + pyqtgraph visualization application |

### Data Flow

```
Device (hardware sync board)
  └─ UDP / Serial
      └─ NetManager / UsbManager
          ├─ PTP (clock offset correction)
          └─ TriggerManager
              └─ Messenger (ZeroMQ PUB tcp://127.0.0.1:4565)
                  └─ Subscribers (user app / Python GUI)
```

### JSON Protocol

All messages (sensor data, triggers, PTP) are JSON over the transport channel:

```json
{ "f": "t",   "t": <us>, "s": <8-bit mask> }          // trigger
{ "f": "imu", "t": <us>, "d": [ax,ay,az,gx,gy,gz,T], "q": [q0,q1,q2,q3] }
{ "f": "a",   "a": <us> }                               // PTP request
{ "f": "b",   "a": <us> }                               // PTP response
```

ZeroMQ topic == sensor name (e.g. `imu_1`, `cam_1`, `gps`).

### Example Implementations (`example/`)

- **VideoCam** — standard USB/V4L camera; supports `uart` or `net` mode at launch
- **NetCam** — GigE industrial cameras (Hikvision MV SDK bundled in `mvcam/`)
- **VideoCam_LVI** — camera + IMU fusion pattern
- **CustomCam** — template for adding a new sensor type

Each example has its own `CMakeLists.txt` and can be taken as the starting point when integrating a new camera SDK.

## Key Types

```cpp
// infinite_sense_core/include/data.h
struct ImuData  { uint64_t time_stamp_us; float a[3], g[3], q[4], temperature; };
struct CamData  { uint64_t time_stamp_us; GMat image; uint64_t exposure_time_us; };

enum TriggerDevice { IMU_1, IMU_2, CAM_1, CAM_2, CAM_3, CAM_4, LASER, GPS };
```

## CI

- `.github/workflows/cmake-single-platform.yml` — builds C++ on Ubuntu on every push
- `.github/workflows/publish-release.yml` — packages and publishes releases
