# esp32-scope-bench

Dual‑core **ESP32 signal harness** for oscilloscope testing. UI runs on **core 1** (Serial), time‑critical waveforms on **core 0** for clean timing. Includes pin hints, DMA‑driven **10 MHz SPI** edge zoom, UART patterns, and LEDC PWM tests.

> Written for my review on the BSIDE O2, but works with any scope.

---

## Features

- **Dual‑core**: no prints on the signal core
- **SPI edge zoom** @ **10 MHz** via DMA bursts
- **LEDC PWM**: sweep, duty steps, static, 100 Hz
- **UART** 0x55 stream with live baud sweep
- **Single‑shot pulse** via BOOT button
- **Pin hints** printed on test select

---

## Get started (PlatformIO + VS Code)

1. **Install** the _PlatformIO IDE_ extension in VS Code.
2. **Clone** this repo and **Open Folder** in VS Code.
3. Click **Build** ▶️ **Upload** (board `esp32dev` by default).
4. Open the **Serial Monitor** at **115200**.

**Controls:**

- `m` → menu
- `1..7` → select test
- `0` → stop

---

## Tests (1–7)

| ID  | Test                   | Scope view                                     | Default pins                   |
| --- | ---------------------- | ---------------------------------------------- | ------------------------------ |
| 1   | LEDC Sweep             | Square wave cycling: 10 k, 100 k, 500 k, 1 MHz | `GPIO25` (`LEDC_TEST_PIN`)     |
| 2   | SPI Edge Zoom (10 MHz) | 10 MHz SCK; MOSI sends `0x55` (\~5 MHz toggle) | `SCK=18`, `MOSI=23`, `CS=5`    |
| 3   | Single‑shot Pulse      | One \~1 ms HIGH pulse when BOOT pressed        | Pulse `GPIO25`, Button `GPIO0` |
| 4   | UART Sweep (0x55)      | TX frames; baud steps every 5 s                | `GPIO17` (`UART_TX_PIN`)       |
| 5   | PWM Duty Steps         | 1 kHz PWM at 25 % → 50 % → 75 %                | `GPIO25` (`PWM_PIN`)           |
| 6   | Trigger Stability      | Static square @ `PWM_FREQ` (\~50 % duty)       | `GPIO25` (`PWM_PIN`)           |
| 7   | Zoom Depth (100 Hz)    | 100 Hz PWM (\~50 % duty, 10 ms period)         | `GPIO25` (`PWM_PIN`)           |

> Pin hints print automatically on selection. Always connect scope **GND → ESP32 GND**.

---

## Configure pins (optional)

Override via `platformio.ini → build_flags`:

```
-D SPI_SCK_PIN=18
-D SPI_MOSI_PIN=23
-D SPI_CS_PIN=5
-D LEDC_TEST_PIN=25
-D PIN_PULSE=25
-D PIN_BTN=0
-D UART_TX_PIN=17
-D PWM_PIN=25
-D PWM_FREQ=100000
-D PWM_BITS=6
-D KEEP_IDLE0_WDT          ; keep core‑0 idle watchdog enabled (default=disabled)
```

---

## Notes

- UI on core 1; waveform worker on core 0.
- SPI uses **DMA bursts**; tiny periodic `vTaskDelay(1)` keeps WDT happy.
- By default, the **core‑0 idle WDT is disabled** at startup for continuous output.

---

## License

MIT
