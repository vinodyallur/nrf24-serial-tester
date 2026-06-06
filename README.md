# nRF24L01 Serial Chat / Tester

A dead-simple way for **beginners to test nRF24L01 modules**. Flash the same
sketch to two boards, open the serial monitor on each, and **type messages back
and forth over the radio**. If your typing shows up on the other board, your
modules and wiring work — now you can confidently build the bigger project
(drone, RC remote, sensor link, ...).

> This was extracted and simplified from a working ESP32 drone radio link so
> beginners don't get stuck on the nRF24 the way most people do.

- **Free / open source (MIT).** Source code, wiring, tips, and ready-to-flash
  `.bin` files are all in this repo.

## Features
- **Boot self-test:** clearly prints `nRF24 detected: YES / NO`.
- **Two-way chat:** type a line, it's radioed to the other board and printed.
- **`[TX ok]` / `[TX FAIL]`** feedback so you know if the other end received it.
- **Built-in test commands:** `/test` (success %), `/ping` (range test),
  `/stats`, `/role`, `/chan`.
- **One sketch, many boards:** Arduino Uno/Nano, classic ESP32, ESP32-C3 Supermini
  (pins auto-selected).

## Quick start

1. **Wire two modules** — see [docs/WIRING.md](docs/WIRING.md). Add the capacitor
   (see tips), power from **3.3 V only**.
2. **Get the library:** Arduino IDE → Library Manager → install **"RF24" by TMRh20**.
3. **Flash** `nrf24_serial_chat/nrf24_serial_chat.ino` to **both** boards
   (or use a ready-made binary in [`bin/`](bin/) — see below).
4. **Open the serial monitor** on each at **115200 baud**.
5. On one board type **A** + Enter (Node A); on the other type **B** + Enter.
6. **Type a message** on either board and press Enter. It appears as `[RX] ...`
   on the other. Done — your nRF24 link works!

## Commands (type in the serial monitor)
| Command | What it does |
|---------|--------------|
| `/help` | show the command list |
| `/ping` | auto-send a counter every second (range / link test) |
| `/stop` | stop `/ping` |
| `/test` | fire 20 packets and print the success rate |
| `/stats`| show ok / fail / received counters |
| `/role a` or `/role b` | switch this board's role |
| `/chan 76` | change RF channel 0..125 (match on both boards) |
| *(any text)* | sent as a message to the other board |

## Ready-to-flash binaries
Pre-compiled merged binaries are in [`bin/`](bin/):

| File | Board | Flash command (esptool) |
|------|-------|--------------------------|
| `nrf24_serial_chat_esp32.bin`   | classic ESP32      | `esptool.py --chip esp32   -p COMx write_flash 0x0 nrf24_serial_chat_esp32.bin` |
| `nrf24_serial_chat_esp32c3.bin` | ESP32-C3 Supermini | `esptool.py --chip esp32c3 -p COMx write_flash 0x0 nrf24_serial_chat_esp32c3.bin` |

These are **merged** images (bootloader + partitions + app) written at offset
`0x0`. On Windows you can also use the included helper:

```powershell
# from this folder, with arduino-cli in tools\ (see flash.ps1 header)
./flash.ps1 esp32      -Port COM9
./flash.ps1 esp32c3    -Port COM36
```

## Build it yourself
Install the **RF24** library and your board's core, then in the Arduino IDE open
`nrf24_serial_chat/nrf24_serial_chat.ino`, select your board, and upload.

With `arduino-cli`:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32   nrf24_serial_chat
arduino-cli compile --fqbn esp32:esp32:esp32c3 nrf24_serial_chat
arduino-cli compile --fqbn arduino:avr:uno     nrf24_serial_chat
```

## Troubleshooting
- **`nRF24 detected: NO`** → it's wiring or power. Work through the checklist in
  [docs/WIRING.md](docs/WIRING.md#it-isnt-detected--connection-problem-checklist).
- **Detected but `/test` is 0%** → the other board is off / out of range / on a
  different channel or the same role. Set one to A and one to B.
- **Random drops** → add the capacitor and lower distance; see
  [docs/TIPS.md](docs/TIPS.md).

## The #1 tip
**Solder a 10 µF capacitor across the module's + (VCC) and − (GND) pins.** It
fixes the majority of "not detected" and "randomly drops" problems by smoothing
the current spikes the radio makes when transmitting. Full list in
[docs/TIPS.md](docs/TIPS.md).

## License
MIT — see [LICENSE](LICENSE). Use it freely.
