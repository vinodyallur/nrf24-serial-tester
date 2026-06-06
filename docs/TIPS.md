# Tips to make nRF24L01 actually work

These cheap modules are notorious for "sometimes works". 90% of problems are
**power** and **wiring**. Here is what reliably fixes them.

## 1. Add a capacitor across VCC and GND  ← do this first
Solder a **10 µF** capacitor (1–100 µF is fine; 4.7–47 µF typical) directly on
the module's **+ (VCC)** and **− (GND)** pins.
- Electrolytic cap: the **longer leg is +**, the side with the stripe is **−**.
- This absorbs the current spikes the radio makes when it transmits. Without it,
  the voltage dips, the chip browns out, and you get random failures or
  "not detected".
- A 100 nF (0.1 µF) ceramic in parallel is an optional bonus.

## 2. Power it from a clean 3.3 V
- **Never 5 V** on VCC — it can destroy the module. (Logic pins are 5 V tolerant,
  VCC is not.)
- The standard (small PCB antenna) module runs fine off most boards' 3.3 V pin
  **with the capacitor**.
- The **PA+LNA** version (the big one with a screw-on antenna) draws much more
  current — feed it from a **separate 3.3 V regulator/supply**, not the Arduino's
  3.3 V pin. Keep grounds connected together.

## 3. Use the most reliable radio settings (already set in the sketch)
- **Data rate 250 kbps** → best sensitivity / longest range. Both ends must match.
- **Channel 76** → away from most Wi-Fi. Both ends must match. Avoid busy
  channels; try `/chan 100` if your Wi-Fi is crowded.
- **PA level MIN** → on a flaky module, low power is actually *more* stable
  because it draws less current. Raise it later once the cap is in.
- **Auto-ACK + retries** → on by default, so `[TX ok]` means the other board
  truly received it.

## 4. Keep the SPI wiring short and solid
- Short jumper wires (≤ 20 cm). Long/loose wires corrupt SPI and the radio
  "disappears".
- Reseat the module; a single not-quite-connected pin causes "not detected".
- MISO and MOSI are the most commonly swapped pair — check them first.

## 5. Match EVERYTHING on both boards
Channel, data rate, address, and payload size must be identical. This sketch
hard-codes them, so just flash the same sketch to both and you're matched. If you
change `/chan` on one board, change it on the other too.

## 6. Two different modules, two roles
Flash the sketch to **both** boards. Make one **Node A** and the other **Node B**
(you're prompted at boot, or use `/role a` / `/role b`). They write to each
other's address — same role on both means they won't hear each other.

## 7. Quick health checks built into the sketch
- `nRF24 detected: YES/NO` at boot → proves the SPI wiring/power.
- `/test` → fires 20 packets and prints a success %. 100% = solid link,
  0% = other board off / out of range / wrong channel or role.
- `/ping` → streams a counter once a second; walk apart to test range.
- `/stats` → running ok / fail / received totals.

## 8. Still failing?
- Try a different physical module — a meaningful fraction of cheap ones are DOA.
- Try a different VCC source (a bench 3.3 V supply rules out power issues fast).
- Move away from Wi-Fi routers / USB 3 ports (both spew 2.4 GHz noise).
