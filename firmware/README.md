# 433 MHz Replay Firmware

## Purpose

Capture and analyze 433 MHz signals; demo is SIMULATION ONLY. Proof-for-study: no live replay without lab authorization.

## Board

- **Board**: ESP32 NodeMCU + HC-12 433 MHz transceiver
- **FQBN**: `esp32:esp32:esp32`
- **Sketch**: `h5_433_replay/h5_433_replay.ino`

## Wiring

```
HC-12: TXD -> RX2 (GPIO16), RXD -> TX2 (GPIO17), VCC -> 5V, GND -> GND, SET -> GND (AT mode) or floating
```

## Build

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h5_433_replay
# upload (example, ESP32-C6):
# arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyACM0 firmware/h5_433_replay
```

## Runtime

See the root README "IMPORTANT" section before powering on. This firmware is
for authorized own-lab study. Serial console exposes the interactive command
set described in the root README. All identifiers in the sketch are
placeholders (`lab-*` SSIDs, `00:11:22:33:44:55`, RFC 5737 / example.com).
