# Wiring / Connections

The nRF24L01 is a **3.3 V** SPI device. Get these 6 logic wires + power right and
it works; get power wrong and it either isn't detected or drops packets randomly.

## The nRF24L01 pinout (top view, pins facing down)

```
   +-----------+
   | 1  2      |   1 GND     2 VCC (3.3V)
   | 3  4      |   3 CE      4 CSN
   | 5  6      |   5 SCK     6 MOSI
   | 7  8      |   7 MISO    8 IRQ (not used here)
   +-----------+
```

- **VCC = 3.3 V only.** 5 V will damage the module.
- The **logic pins (CE, CSN, SCK, MOSI, MISO) are 5 V tolerant**, so they connect
  directly to an Arduino Uno/Nano's 5 V pins without level shifters.
- **IRQ is not used** by this sketch — leave it unconnected.

## Per-board connections

### Arduino Uno / Nano
| nRF24 pin | Connect to |
|-----------|-----------|
| GND  | GND |
| VCC  | 3.3V |
| CE   | D9 |
| CSN  | D10 |
| SCK  | D13 |
| MOSI | D11 |
| MISO | D12 |
| IRQ  | — |

### Classic ESP32 (DevKit / WROOM)
| nRF24 pin | Connect to |
|-----------|-----------|
| GND  | GND |
| VCC  | 3V3 |
| CE   | GPIO4 |
| CSN  | GPIO5 |
| SCK  | GPIO18 |
| MOSI | GPIO23 |
| MISO | GPIO19 |
| IRQ  | — |

### ESP32-C3 Supermini
| nRF24 pin | Connect to |
|-----------|-----------|
| GND  | GND |
| VCC  | 3V3 |
| CE   | GPIO10 |
| CSN  | GPIO20 |
| SCK  | GPIO5 |
| MOSI | GPIO7 |
| MISO | GPIO6 |
| IRQ  | — |

> Using different pins? Edit the `#define PIN_...` block at the top of
> `nrf24_serial_chat/nrf24_serial_chat.ino`.

## "It isn't detected" — connection problem checklist

When the serial monitor prints `nRF24 detected: NO`, work through these in order:

1. **Add a capacitor.** Solder a **10 µF** (anything 1–100 µF) capacitor directly
   across the module's **VCC (+)** and **GND (−)** pins, long leg to VCC. This is
   the #1 fix — the radio draws sharp current spikes the breadboard rails can't
   supply, and the cap smooths them.
2. **3.3 V, not 5 V.** Confirm VCC is on a real 3.3 V rail.
3. **Swap MISO/MOSI?** These two are the most commonly reversed. Double-check.
4. **CE vs CSN.** Make sure they match the table for your board (easy to swap).
5. **Bad jumper / loose pin.** Reseat the module; use short, solid wires.
6. **PA+LNA (big antenna) version?** Its current draw is too high for most
   on-board 3.3 V regulators — power it from a **separate 3.3 V supply** (with the
   cap), grounds common.

See [TIPS.md](TIPS.md) for reliability tips once it *is* detected.
