/*
  =====================================================================
  nRF24L01 Serial Chat / Tester  --  beginner friendly
  =====================================================================
  Flash this SAME sketch onto TWO boards. Open a serial monitor on each
  at 115200 baud. Whatever you type on one board is radioed to the other
  and printed. This is the quickest way to prove your nRF24L01 modules
  and wiring actually work before you build anything bigger (a drone,
  a remote, a sensor link, ...).

  WHAT IT DOES
    - On boot it tells you clearly if the nRF24 chip is detected (YES/NO).
    - You pick a role: A or B (one board each). They auto-talk to each other.
    - Type a line + Enter  -> it is sent; you see [TX ok] or [TX FAIL].
    - Anything received is printed as [RX] ...
    - Handy commands (type them and press Enter):
        /help          show the command list
        /ping          auto-send a counter every second (range/link test)
        /stop          stop /ping
        /test          fire 20 packets, print the success rate
        /stats         show ok/fail/received counters
        /role a|b      switch this board's role on the fly
        /chan 76       change RF channel 0..125 (must match the other board)

  Works on: Arduino Uno/Nano, classic ESP32, ESP32-C3 Supermini.
  Libraries needed (Library Manager): "RF24" by TMRh20.

  Source code is free / open (MIT). See LICENSE.
  =====================================================================
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// ----------------------------------------------------------------------
// 1) PINS  --  picked automatically for your board.
//    If your wiring is different, edit the numbers for your board below.
// ----------------------------------------------------------------------
#if defined(CONFIG_IDF_TARGET_ESP32C3)
  // ----- ESP32-C3 Supermini (custom SPI pins) -----
  #define PIN_SCK   5
  #define PIN_MISO  6
  #define PIN_MOSI  7
  #define PIN_CE    10
  #define PIN_CSN   20
  #define USE_CUSTOM_SPI 1
  #define BOARD_NAME "ESP32-C3"
#elif defined(ESP32)
  // ----- Classic ESP32 (default VSPI: SCK=18 MISO=19 MOSI=23) -----
  #define PIN_CE    4
  #define PIN_CSN   5
  #define BOARD_NAME "ESP32"
#elif defined(ARDUINO_ARCH_AVR)
  // ----- Arduino Uno / Nano (hardware SPI: SCK=13 MISO=12 MOSI=11) -----
  #define PIN_CE    9
  #define PIN_CSN   10
  #define BOARD_NAME "Arduino AVR"
#else
  // ----- Fallback (CE=9, CSN=10) -----
  #define PIN_CE    9
  #define PIN_CSN   10
  #define BOARD_NAME "generic"
#endif

RF24 radio(PIN_CE, PIN_CSN);

// ----------------------------------------------------------------------
// 2) RADIO SETTINGS  --  these MUST be identical on both boards.
// ----------------------------------------------------------------------
const uint8_t       CHANNEL   = 76;             // 0..125
rf24_pa_dbm_e       PA_LEVEL  = RF24_PA_MIN;    // MIN is safest for cheap modules
const rf24_datarate_e DATARATE = RF24_250KBPS;  // 250kbps = best range/reliability

// Two pipe addresses. Node A and Node B swap which one they write vs read.
const uint8_t address[][6] = { "1Node", "2Node" };

// ----------------------------------------------------------------------
// 3) State
// ----------------------------------------------------------------------
const uint8_t MSG_LEN = 32;        // fixed packet size (max 32 on nRF24)
char txBuf[MSG_LEN];
char rxBuf[MSG_LEN];

bool isNodeA = true;               // role; chosen at boot
uint8_t rfChannel = CHANNEL;

unsigned long okCount = 0, failCount = 0, rxCount = 0;
bool pingMode = false;
unsigned long lastPing = 0;
unsigned long pingNum = 0;

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------
void applyRole() {
  // Node A writes to "2Node" and listens on "1Node"; Node B is the mirror.
  uint8_t w = isNodeA ? 1 : 0;
  uint8_t r = isNodeA ? 0 : 1;
  radio.stopListening();
  radio.openWritingPipe(address[w]);
  radio.openReadingPipe(1, address[r]);
  radio.startListening();
}

bool initRadio() {
#ifdef USE_CUSTOM_SPI
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CSN);
#endif
  if (!radio.begin()) return false;          // false = chip not responding
  radio.setChannel(rfChannel);
  radio.setDataRate(DATARATE);
  radio.setPALevel(PA_LEVEL);
  radio.setRetries(5, 15);                    // auto-retry: 5*250us gap, 15 tries
  radio.setPayloadSize(MSG_LEN);
  applyRole();
  return true;
}

void sendMsg(const char* m) {
  radio.stopListening();
  bool ok = radio.write(m, MSG_LEN);          // true only if the other end ACKs
  radio.startListening();
  if (ok) { okCount++;  Serial.print(F("[TX ok]   ")); }
  else    { failCount++; Serial.print(F("[TX FAIL] ")); }
  Serial.println(m);
}

void printBanner() {
  Serial.println();
  Serial.println(F("==================================================="));
  Serial.println(F("        nRF24L01 Serial Chat / Tester"));
  Serial.println(F("==================================================="));
  Serial.print  (F("Board: "));   Serial.println(F(BOARD_NAME));
  Serial.print  (F("CE pin="));   Serial.print(PIN_CE);
  Serial.print  (F("  CSN pin=")); Serial.println(PIN_CSN);
#ifdef USE_CUSTOM_SPI
  Serial.print  (F("SPI: SCK=")); Serial.print(PIN_SCK);
  Serial.print  (F(" MISO="));    Serial.print(PIN_MISO);
  Serial.print  (F(" MOSI="));    Serial.println(PIN_MOSI);
#endif
}

void printTroubleshooting() {
  Serial.println();
  Serial.println(F("nRF24 NOT detected. Check these (most common first):"));
  Serial.println(F("  1) Power the module from 3.3V ONLY (5V kills it)."));
  Serial.println(F("  2) Solder a 10uF (or 1-100uF) capacitor across the"));
  Serial.println(F("     module's + (VCC) and - (GND) pins. This is the"));
  Serial.println(F("     single biggest fix for flaky/undetected modules."));
  Serial.println(F("  3) Re-check SPI wiring: CE, CSN, SCK, MOSI, MISO, GND."));
  Serial.println(F("  4) Use short, good jumper wires; reseat the module."));
  Serial.println(F("  5) For the PA+LNA (antenna) version use a separate 3.3V"));
  Serial.println(F("     supply - the board's regulator often can't feed it."));
  Serial.println();
}

void printHelp() {
  Serial.println();
  Serial.println(F("Commands:  /help  /ping  /stop  /test  /stats  /role a|b  /chan N"));
  Serial.println(F("Or just type a message and press Enter to send it."));
  Serial.println(F("---------------------------------------------------"));
}

void chooseRole() {
  Serial.println();
  Serial.println(F("Pick a role for THIS board:"));
  Serial.println(F("  Type 'A' (default) or 'B', then Enter."));
  Serial.println(F("  -> Flash one board as A and the other as B."));
  Serial.print  (F("Waiting 6s... "));

  unsigned long start = millis();
  while (millis() - start < 6000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'A' || c == 'a') { isNodeA = true;  break; }
      if (c == 'B' || c == 'b') { isNodeA = false; break; }
    }
  }
  // drain any leftover newline characters
  while (Serial.available()) Serial.read();

  Serial.println();
  Serial.print(F("This board is NODE "));
  Serial.println(isNodeA ? F("A") : F("B"));
}

// returns true if the line was a command (already handled)
bool handleCommand(String line) {
  if (!line.startsWith("/")) return false;
  line.toLowerCase();

  if (line == "/help")  { printHelp(); return true; }
  if (line == "/ping")  { pingMode = true;  Serial.println(F("[ping] auto-send started")); return true; }
  if (line == "/stop")  { pingMode = false; Serial.println(F("[ping] stopped")); return true; }
  if (line == "/stats") {
    Serial.print(F("[stats] TX ok="));   Serial.print(okCount);
    Serial.print(F("  TX fail="));        Serial.print(failCount);
    Serial.print(F("  RX received="));    Serial.println(rxCount);
    return true;
  }
  if (line == "/test") {
    Serial.println(F("[test] sending 20 packets..."));
    unsigned long good = 0;
    for (int i = 1; i <= 20; i++) {
      snprintf(txBuf, MSG_LEN, "TEST %d/20", i);
      radio.stopListening();
      bool ok = radio.write(txBuf, MSG_LEN);
      radio.startListening();
      if (ok) { good++; okCount++; } else { failCount++; }
      delay(50);
    }
    Serial.print(F("[test] success "));
    Serial.print(good); Serial.print(F("/20  ("));
    Serial.print(good * 5); Serial.println(F("%)"));
    if (good == 0) Serial.println(F("       0% -> the OTHER board is off, not in range, or on a different channel/role."));
    return true;
  }
  if (line.startsWith("/role")) {
    if (line.indexOf('b') > 0) isNodeA = false;
    else                       isNodeA = true;
    applyRole();
    Serial.print(F("[role] now NODE "));
    Serial.println(isNodeA ? F("A") : F("B"));
    return true;
  }
  if (line.startsWith("/chan")) {
    int sp = line.indexOf(' ');
    if (sp > 0) {
      int ch = line.substring(sp + 1).toInt();
      if (ch >= 0 && ch <= 125) {
        rfChannel = ch;
        radio.setChannel(rfChannel);
        Serial.print(F("[chan] now ")); Serial.println(rfChannel);
        Serial.println(F("       set the SAME channel on the other board!"));
      } else {
        Serial.println(F("[chan] value must be 0..125"));
      }
    }
    return true;
  }
  Serial.println(F("[?] unknown command - type /help"));
  return true;
}

// ----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000) { /* wait for USB serial */ }
  delay(300);

  printBanner();

  bool ok = initRadio();
  bool detected = ok && radio.isChipConnected();
  Serial.print(F("nRF24 detected: "));
  Serial.println(detected ? F("YES") : F("NO  <-- fix wiring/power first"));
  if (!detected) printTroubleshooting();

  Serial.print(F("Channel="));   Serial.print(rfChannel);
  Serial.print(F("  Rate=250kbps  Power=MIN  PayloadSize="));
  Serial.println(MSG_LEN);

  chooseRole();
  applyRole();
  printHelp();
}

void loop() {
  // 1) Receive anything waiting.
  if (radio.available()) {
    radio.read(rxBuf, MSG_LEN);
    rxBuf[MSG_LEN - 1] = '\0';
    rxCount++;
    Serial.print(F("[RX] "));
    Serial.println(rxBuf);
  }

  // 2) Auto-ping (if enabled).
  if (pingMode && millis() - lastPing >= 1000) {
    lastPing = millis();
    snprintf(txBuf, MSG_LEN, "PING #%lu", ++pingNum);
    sendMsg(txBuf);
  }

  // 3) Serial input -> command or message.
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;
    if (handleCommand(line)) return;
    line.toCharArray(txBuf, MSG_LEN);
    sendMsg(txBuf);
  }
}
