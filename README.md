# esp32-scope-bench

A **dual-core ESP32 signal harness** for testing oscilloscopes.
Menu on **core 1** (Serial), time-critical waveforms on **core 0** (FreeRTOS worker) for clean timing.
Includes **pin hints**, **DMA-driven 10 MHz SPI “edge zoom”**, UART patterns, and LEDC PWM tests.

> Built to sanity-check compact scopes like the **BSIDE O2**, but works great for any scope.

---

## Features

- **Dual-core design:** UI on core 1; signals generated on core 0 for stability
- **SPI edge zoom @ 10 MHz** using `transferBytes()` + DMA bursts (near-continuous SCK)
- **LEDC PWM**: static square, duty steps, low-freq “zoom depth”, and frequency sweep
- **UART pattern**: steady `0x55` stream with live baud sweep
- **Single-shot pulse**: hardware-debounced pulse on BOOT button
- **Pin hints** printed whenever you switch tests
- **Watchdog-safe**: tiny cooperative yields; core-0 idle WDT disabled by default (optional)

---

## Quick start (PlatformIO)

Clone repo and upload to ESP32 **OR**:

1. Create a new PlatformIO project (`board = esp32dev`, framework Arduino).
2. Drop `src/main.cpp` from this repo into your project.
3. (Optional) Override pins with `build_flags` (see below).
4. Build & upload. Open Serial Monitor at **115200**.
5. Press:

   - `m` → menu
   - `3..9` → select a test
   - `0` → stop

### Example `platformio.ini`

```ini
[env:esp32-scope-bench]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

; Optional custom pins / options:
; build_flags =
;   -D SPI_SCK_PIN=18
;   -D SPI_MOSI_PIN=23
;   -D SPI_CS_PIN=5
;   -D LEDC_TEST_PIN=25
;   -D PIN_PULSE=25
;   -D PIN_BTN=0
;   -D UART_TX_PIN=17
;   -D PWM_PIN=25
;   -D PWM_FREQ=100000
;   -D PWM_BITS=6
;   -D KEEP_IDLE0_WDT        ; keep core-0 idle watchdog enabled (default is disabled)
```

---

## Tests (3–9)

| ID  | Name                   | What you see on the scope                                     | Probe pins (default)                           |
| --- | ---------------------- | ------------------------------------------------------------- | ---------------------------------------------- |
| 3   | LEDC Sweep             | Square wave cycles through 10 k, 100 k, 500 k, 1 MHz          | `GPIO25` (`LEDC_TEST_PIN`)                     |
| 4   | SPI Edge Zoom (10 MHz) | Continuous 10 MHz clock on SCK; MOSI toggles `0x55` (\~5 MHz) | `SCK=GPIO18`, `MOSI=GPIO23`, `CS=GPIO5` (LOW)  |
| 5   | Single-shot Pulse      | One \~1 ms HIGH pulse when BOOT pressed                       | Pulse: `GPIO25` (`PIN_PULSE`); Button: `GPIO0` |
| 6   | UART Sweep (0x55)      | UART TX frame; baud steps every 5 s                           | `GPIO17` (`UART_TX_PIN`)                       |
| 7   | PWM Duty Steps         | 1 kHz PWM at 25 % → 50 % → 75 %                               | `GPIO25` (`PWM_PIN`)                           |
| 8   | Trigger Stability      | Static square @ `PWM_FREQ` (\~50 % duty)                      | `GPIO25` (`PWM_PIN`)                           |
| 9   | Zoom Depth (100 Hz)    | 100 Hz PWM (\~50 % duty, 10 ms period)                        | `GPIO25` (`PWM_PIN`)                           |

> Every time you pick a test, **pin hints** are printed so you know exactly where to connect.

---

## Why it’s stable

- UI/Serial lives on **core 1**.
- The signal generator runs on **core 0** and **never prints**.
- **SPI** uses **DMA bursts** (`transferBytes`) from a pre-filled buffer so SCK stays busy while the CPU breathes.
- A tiny `vTaskDelay(1)` every few hundred bursts keeps the watchdog happy without visible gaps.
- By default, the **core-0 idle Task Watchdog is disabled** at startup for rock-solid continuous output. You can keep it enabled by defining `KEEP_IDLE0_WDT`.

---

## Pin overrides

All pins are configurable via `build_flags`:

- `LEDC_TEST_PIN` (default `25`)
- `SPI_SCK_PIN` (default `18`)
- `SPI_MOSI_PIN` (default `23`)
- `SPI_CS_PIN` (default `5`)
- `PIN_PULSE` (default `25`)
- `PIN_BTN` (default `0`, BOOT)
- `UART_TX_PIN` (default `17`)
- `PWM_PIN` (default `25`)
- `PWM_FREQ` (default `100000`)
- `PWM_BITS` (default `6`)

**Remember GND**: always clip the scope ground to ESP32 GND.

---

## Serial controls

- `m` → print menu
- `3..9` → run selected test (prints pin hints)
- `0` → stop output

Typical output:

```
=== Menu (core1) ===
 3) LEDC sweep 10k→1M...
 ...
-> Test 4

-- Probe / Wiring --
SCK -> GPIO18 (10 MHz)
MOSI -> GPIO23 (~5 MHz toggle from 0x55)
CS   -> GPIO5 (held LOW)
GND -> GND
```

---

## Troubleshooting

- **Task watchdog resets (IDLE0 on CPU0)**
  This code already yields periodically and disables the core-0 idle WDT by default. If you re-enable it (`-D KEEP_IDLE0_WDT`), keep the yields (`vTaskDelay(1)`) in place.

- **SPI clock has gaps**
  Ensure you’re on **Test 4** (DMA bursts) and that Wi-Fi/BLE are **off**. Keep CS **LOW** continuously (the code does). Avoid adding any `Serial.print` in the worker task.

- **No signal**
  Check your board’s pin mapping. Override pins in `platformio.ini` to match your dev kit’s labels. Confirm GND is connected.

- **Noise / ringing**
  Use a short ground spring on the probe. Keep leads short, especially for 10 MHz SCK.

---

## Project structure & style

- Clean separation via **namespaces** (`Mod3`..`Mod9`) for each test
- Core-0 worker dispatches `begin/tick/end` without allocations or prints
- Minimal dependencies (`Arduino.h`, `SPI.h`)

---

## Safety notes

- ESP32 is **3.3 V** logic. Don’t probe mains or high-energy circuits with this board.
- Keep scope ground isolated from anything other than the ESP32 ground.

---

## License

MIT — do whatever, just keep the notice.

---

## Credits

- ESP32 Arduino core & FreeRTOS
- Inspiration: need for quick scope validation on compact handhelds (e.g., BSIDE O2)

---

## Roadmap

- Optional compile-time “locked test” mode (no menu)
- Additional patterns (I²C, higher-frequency SPI if your board tolerates it, PRBS)
- Simple web UI (if Wi-Fi is enabled) running on core 1 only
