# H5 — 433 MHz Replay Attack Tool

Capture and replay 433 MHz signals for garage doors, remotes, and sensors.

## Overview

This project implements a 433 MHz signal capture and replay tool using the HC-12 transceiver. It can:
- Capture raw 433 MHz signals from remote controls
- Display captured data in HEX and BIN formats
- Replay captured signals to trigger devices
- Analyze timing patterns for rolling code detection

**WARNING: Educational use only. Test on your own devices.**

## Hardware

| Component | Connection | Role |
|-----------|------------|------|
| ESP32 NodeMCU | Main board | Control + serial interface |
| HC-12 | TX2→RX, RX2→TX | 433 MHz transceiver |

## HC-12 Wiring

```
HC-12 Pin → NodeMCU Pin
────────────────────────
TXD      → RX2 (GPIO16)
RXD      → TX2 (GPIO17)
VCC      → 5V rail
GND      → GND rail
SET      → GND (AT mode) or floating (normal)
```

## Serial Commands

```
capture     - Start capturing 433 MHz signals
stop        - Stop capture/replay
list        - List captured packets
select N    - Select packet N
replay      - Replay selected packet once
replay N    - Replay packet N once
replay N M  - Replay packet N M times
AT+XX       - Send AT command to HC-12
clear       - Clear captured packets
help        - Show commands
```

## Common 433 MHz Protocols

| Device | Protocol | Notes |
|--------|----------|-------|
| Garage door | Fixed code | 8-24 bit, simple replay works |
| Car key | Rolling code | 40-80 bit, changes each press |
| Weather sensor | Fixed code | 24-36 bit, periodic transmission |
| Doorbell | Fixed code | 12-24 bit, simple replay |
| Power meter | Custom | 36-48 bit, periodic |

## Example Session

```
=== H5 — 433 MHz Replay Tool ===
Capture mode ON. Listening for 433 MHz signals...

[CAPTURE] Packet #0 (4 bytes):
  HEX: AA 12 34 56
  BIN: 10101010 00010010 00110100 01010110
  Total captured: 1

[CAPTURE] Packet #1 (4 bytes):
  HEX: AA 12 34 56
  Total captured: 2

[REPLAY] Sending packet #0 (4 bytes)...
  HEX: AA 12 34 56
  Sent!
```

## Build & Flash

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 h5_433_replay
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 h5_433_replay
```

## Research Value

- **H5 — 433 MHz Replay**: This project
- **W2 — Protocol Fingerprinting**: Analyze captured signals
- **H12 — Wireless Dead-Drop**: Use HC-12 for covert communication

## Legal Disclaimer

This tool is for authorized security testing only. Unauthorized interception and replay of signals is illegal. Always obtain permission before testing on devices you don't own.

## License

MIT
