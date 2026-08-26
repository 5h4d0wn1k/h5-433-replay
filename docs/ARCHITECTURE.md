# H5 — 433 MHz Replay Architecture

## System Overview

The H5 433 MHz Replay Tool captures and replays radio signals in the 433 MHz ISM band. This is commonly used for:
- Garage door openers
- Remote controls
- Wireless sensors
- Doorbells
- Power meters

## Attack Flow

```
┌─────────────────┐
│  433 MHz Signal │
│  (Remote/Door)  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  HC-12 Receiver │
│  (433 MHz)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  ESP32 Capture  │
│  (Raw Data)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Signal Analysis│
│  (HEX/BIN)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Replay Engine  │
│  (HC-12 TX)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Target Device  │
│  (Garage/Door)  │
└─────────────────┘
```

## 433 MHz Signal Characteristics

### Modulation

Most 433 MHz devices use:
- **OOK (On-Off Keying)**: Simple amplitude modulation
- **ASK (Amplitude Shift Keying)**: Two amplitude levels
- **PWM (Pulse Width Modulation)**: Timing-based encoding

### Common Encoding

```
Fixed Code:
  [Header] [Device ID] [Command] [Checksum]
  
Rolling Code:
  [Header] [Device ID] [Counter] [Encrypted]
```

## HC-12 Configuration

### AT Commands

| Command | Description | Example |
|---------|-------------|---------|
| AT | Test connection | AT → OK |
| AT+V | Firmware version | AT+V → OK+v2.6 |
| AT+B9600 | Set baud rate | AT+B9600 → OK+B9600 |
| AT+C001 | Set channel | AT+C001 → OK+C001 |
| AT+P4 | Set power | AT+P4 → OK+P4 |
| AT+FU3 | Set FIFO mode | AT+FU3 → OK+FU3 |
| AT+W1206 | Set data format | AT+W1206 → OK+W1206 |

### Channel Mapping

| Channel | Frequency | Note |
|---------|-----------|------|
| 001 | 433.4 MHz | Default |
| 002 | 433.6 MHz | |
| 003 | 433.8 MHz | |
| ... | ... | |
| 050 | 443.2 MHz | |

## Signal Capture

### Data Format

Captured signals are stored as raw bytes:
```c
struct Packet {
    uint8_t data[64];    // Raw signal data
    uint8_t length;      // Data length
    uint32_t timestamp;  // Capture time
    uint32_t frequency;  // Timing pattern
    bool captured;       // Valid capture
};
```

### Analysis Modes

1. **HEX Display**: Raw bytes in hexadecimal
2. **BIN Display**: Binary representation for pattern analysis
3. **Timing Analysis**: Interval between packets (for rolling codes)

## Replay Attack

### Fixed Code Replay

Simple replay works for fixed code devices:
1. Capture signal from remote
2. Replay same data
3. Device accepts command

### Rolling Code Challenges

Rolling code devices use:
- Counter-based encryption
- Each press generates unique code
- Replayed codes are rejected

**Bypass methods** (educational):
1. **Jam & Capture**: Jam signal, capture code, replay before counter updates
2. **Relay Attack**: Extend signal range in real-time
3. **Brute Force**: Try sequential codes (limited success)

## Device Identification

### Garage Door Opener

```
Typical format:
  24-bit fixed code
  [8-bit house code] [8-bit button] [8-bit checksum]
  
Examples:
  Chamberlain: 24-bit, dip switch encoding
  LiftMaster: 315 MHz or 390 MHz (not 433)
  Genie: 12-bit dip switch
```

### Car Key Fob

```
Typical format:
  40-80 bit rolling code
  [32-bit ID] [32-bit counter/encrypted]
  
Protocol:
  KeeLoq (Microchip)
  Hitag2
  Texas Instruments
```

### Wireless Sensor

```
Typical format:
  36-bit fixed code
  [4-bit type] [8-bit ID] [12-bit value] [8-bit checksum]
  
Example (Temperature):
  Type: 0100 (temp)
  ID: 0x1234
  Value: 0x01A0 (25.6°C)
```

## Defensive Countermeasures

1. **Rolling codes**: Prevent simple replay
2. **Challenge-response**: Require authentication
3. **Signal encryption**: Encrypt payloads
4. **Frequency hopping**: Spread spectrum
5. **Signal detection**: Monitor for replay attacks

## Integration with Other Projects

```
H5 (433 Replay) ──captures──▶ W2 (Protocol Fingerprint)
     │
     ▼
H12 (Dead-Drop) ◀──uses── HC-12
     │
     ▼
H15 (Spectrum War) ──includes── 433 MHz analysis
```

## References

- HC-12 Datasheet
- 433 MHz ISM Band Regulations
- Rolling Code Security Analysis
- OOK/ASK Modulation
