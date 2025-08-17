// ESP32 Dual-Core Test Harness - ALL tests on core 0 (fast), UI/menu on core 1
// Serial 115200: 'm' = menu, '3'..'9' = select, '0' = stop after test run.
// Override pins via platformio.ini build_flags, e.g.
//   -D SPI_SCK_PIN=18 -D SPI_MOSI_PIN=23 -D SPI_CS_PIN=5

#include <Arduino.h>
#include <SPI.h>
extern "C"
{
#include "esp_task_wdt.h"
}

// ---------- Pins & params (override via -D NAME=VALUE) ----------
#ifndef LEDC_TEST_PIN
#define LEDC_TEST_PIN 25
#endif
#ifndef SPI_SCK_PIN
#define SPI_SCK_PIN 18
#endif
#ifndef SPI_MOSI_PIN
#define SPI_MOSI_PIN 23
#endif
#ifndef SPI_CS_PIN
#define SPI_CS_PIN 5
#endif
#ifndef PIN_PULSE
#define PIN_PULSE 25
#endif
#ifndef PIN_BTN
#define PIN_BTN 0
#endif
#ifndef UART_TX_PIN
#define UART_TX_PIN 17
#endif
#ifndef PWM_PIN
#define PWM_PIN 25
#endif
#ifndef PWM_FREQ
#define PWM_FREQ 100000
#endif
#ifndef PWM_BITS
#define PWM_BITS 6
#endif

// ---------- Test selector ----------
enum TestId : uint8_t
{
  TEST_NONE = 0,
  TEST_3 = 3,
  TEST_4 = 4,
  TEST_5 = 5,
  TEST_6 = 6,
  TEST_7 = 7,
  TEST_8 = 8,
  TEST_9 = 9
};

static inline void ledcSafeDetach(int pin)
{
#if ARDUINO_ESP32_MAJOR >= 2
  ledcDetachPin(pin);
#endif
}

// ---------- Namespaced test APIs (forward decls) ----------
namespace Mod3
{
  void begin();
  void tick();
  void end();
}
namespace Mod4
{
  void begin();
  void tick();
  void end();
}
namespace Mod5
{
  void begin();
  void tick();
  void end();
}
namespace Mod6
{
  void begin();
  void tick();
  void end();
}
namespace Mod7
{
  void begin();
  void tick();
  void end();
}
namespace Mod8
{
  void begin();
  void tick();
  void end();
}
namespace Mod9
{
  void begin();
  void tick();
  void end();
}

// ---------------- Core 0 worker (runs ALL tests) ----------------
static TaskHandle_t g_worker = nullptr;
static QueueHandle_t g_cmdQ = nullptr;
static volatile TestId g_active = TEST_NONE;

static void core0Worker(void *)
{
  uint32_t wdt_spin = 0;
  for (;;)
  {
    // Handle new command (non-blocking)
    TestId cmd;
    if (xQueueReceive(g_cmdQ, &cmd, 0) == pdTRUE)
    {
      if (cmd != g_active)
      {
        // end old
        switch (g_active)
        {
        case TEST_3:
          Mod3::end();
          break;
        case TEST_4:
          Mod4::end();
          break;
        case TEST_5:
          Mod5::end();
          break;
        case TEST_6:
          Mod6::end();
          break;
        case TEST_7:
          Mod7::end();
          break;
        case TEST_8:
          Mod8::end();
          break;
        case TEST_9:
          Mod9::end();
          break;
        default:
          break;
        }
        g_active = cmd;
        // begin new
        switch (g_active)
        {
        case TEST_3:
          Mod3::begin();
          break;
        case TEST_4:
          Mod4::begin();
          break;
        case TEST_5:
          Mod5::begin();
          break;
        case TEST_6:
          Mod6::begin();
          break;
        case TEST_7:
          Mod7::begin();
          break;
        case TEST_8:
          Mod8::begin();
          break;
        case TEST_9:
          Mod9::begin();
          break;
        default:
          break;
        }
      }
    }

    // Run active test slice (no Serial, no delays)
    switch (g_active)
    {
    case TEST_3:
      Mod3::tick();
      break;
    case TEST_4:
      Mod4::tick();
      break;
    case TEST_5:
      Mod5::tick();
      break;
    case TEST_6:
      Mod6::tick();
      break;
    case TEST_7:
      Mod7::tick();
      break;
    case TEST_8:
      Mod8::tick();
      break;
    case TEST_9:
      Mod9::tick();
      break;
    default:
      vTaskDelay(1);
      break; // idle
    }

    // Let IDLE0 run periodically so the watchdog stays happy (if enabled)
    if ((++wdt_spin & 0x1FF) == 0)
    {                // every ~512 iterations
      vTaskDelay(1); // 1 tick ≈ 1 ms
    }
  }
}

// ---------------- UI on core 1 (Arduino task) ----------------
static void printMenu()
{
  Serial.println("\n=== Menu (core1) ===");
  Serial.println(" 3) LEDC sweep 10k→1M @ ~50% (GPIO LEDC_TEST_PIN)");
  Serial.println(" 4) SPI edge zoom: SCK 10 MHz (GPIO SPI_SCK_PIN), MOSI 0x55, CS LOW");
  Serial.println(" 5) Single-shot pulse: BOOT -> PIN_PULSE");
  Serial.println(" 6) UART TX 0x55 sweep (GPIO UART_TX_PIN)");
  Serial.println(" 7) PWM steps 25/50/75% @ 1 kHz (GPIO PWM_PIN)");
  Serial.println(" 8) Static square ~50% @ PWM_FREQ (GPIO PWM_PIN)");
  Serial.println(" 9) 100 Hz PWM ~50% (GPIO PWM_PIN)");
  Serial.println(" 0) Stop");
}
static void printPinHints(TestId id)
{
  Serial.println("\n-- Probe / Wiring --");
  switch (id)
  {
  case TEST_3:
    Serial.printf("CH1 -> GPIO%d (LEDC sweep 10k→1M @ ~50%%)\n", LEDC_TEST_PIN);
    break;
  case TEST_4:
    Serial.printf("SCK -> GPIO%d (10 MHz)\n", SPI_SCK_PIN);
    Serial.printf("MOSI -> GPIO%d (~5 MHz toggle from 0x55)\n", SPI_MOSI_PIN);
    Serial.printf("CS   -> GPIO%d (held LOW)\n", SPI_CS_PIN);
    break;
  case TEST_5:
    Serial.printf("Pulse -> GPIO%d ; press BOOT (GPIO%d)\n", PIN_PULSE, PIN_BTN);
    break;
  case TEST_6:
    Serial.printf("UART1 TX -> GPIO%d (8N1, 0x55 stream, sweeping baud)\n", UART_TX_PIN);
    break;
  case TEST_7:
    Serial.printf("PWM -> GPIO%d (LEDC 1 kHz, 25/50/75%%)\n", PWM_PIN);
    break;
  case TEST_8:
    Serial.printf("Square -> GPIO%d (~50%% @ %lu Hz)\n", PWM_PIN, (unsigned long)PWM_FREQ);
    break;
  case TEST_9:
    Serial.printf("PWM -> GPIO%d (100 Hz, ~50%%)\n", PWM_PIN);
    break;
  default:
    Serial.println("Select 3..9");
    break;
  }
  Serial.println("GND -> GND");
}
static inline void sendCmd(TestId id) { xQueueOverwrite(g_cmdQ, &id); }

void setup()
{
  Serial.begin(115200);
  delay(120);

#ifndef KEEP_IDLE0_WDT
  // For clean, continuous signals we disable the core-0 idle WDT (re-enable by defining KEEP_IDLE0_WDT)
  esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(0));
#endif

  g_cmdQ = xQueueCreate(1, sizeof(TestId));
  xTaskCreatePinnedToCore(core0Worker, "core0", 4096, nullptr, 2, &g_worker, 0);
  printMenu();
}
void loop()
{
  if (!Serial.available())
    return;
  int c = Serial.read();
  if (c == 'm' || c == 'M')
  {
    printMenu();
    return;
  }
  if (c == '0')
  {
    sendCmd(TEST_NONE);
    Serial.println("(stopped)");
    return;
  }
  if (c >= '3' && c <= '9')
  {
    TestId next = (TestId)(c - '0');
    sendCmd(next);
    Serial.printf("-> Test %c\n", c);
    printPinHints(next);
  }
}

// ===================== TESTS (run on core 0) =====================

// ---- Test 3: LEDC sweep 10k→1M @ ~50% ----
namespace Mod3
{
  static const int CH = 0;
  struct Step
  {
    uint32_t f;
    uint8_t bits;
    uint32_t ms;
  };
  static const Step STEPS[] = {{10000, 8, 10000}, {100000, 6, 10000}, {500000, 3, 10000}, {1000000, 2, 10000}};
  static size_t i = 0;
  static uint32_t next = 0;

  static inline void apply(const Step &s)
  {
    ledcSetup(CH, s.f, s.bits);
    ledcAttachPin(LEDC_TEST_PIN, CH);
    uint32_t m = (1u << s.bits) - 1u;
    ledcWrite(CH, (m * 50u) / 100u);
  }
  void begin()
  {
    i = 0;
    apply(STEPS[i]);
    next = millis() + STEPS[i].ms;
  }
  void tick()
  {
    uint32_t now = millis();
    if ((int32_t)(now - next) >= 0)
    {
      i = (i + 1) % 4;
      apply(STEPS[i]);
      next = now + STEPS[i].ms;
    }
  }
  void end() { ledcSafeDetach(LEDC_TEST_PIN); }
}

// ---- Test 4: SPI edge zoom 10 MHz (continuous DMA bursts w/ periodic yield) ----
namespace Mod4
{
  static SPIClass vspi(VSPI);
  DMA_ATTR static uint8_t BUF[2048];
  static bool on = false;

  void begin()
  {
    vspi.begin(SPI_SCK_PIN, 19 /*MISO*/, SPI_MOSI_PIN, SPI_CS_PIN);
    vspi.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    pinMode(SPI_CS_PIN, OUTPUT);
    digitalWrite(SPI_CS_PIN, LOW);
    for (size_t i = 0; i < sizeof(BUF); ++i)
      BUF[i] = 0x55;
    on = true;
  }
  void tick()
  {
    if (!on)
      return;
    static uint32_t bursts = 0;
    vspi.transferBytes(BUF, nullptr, sizeof(BUF)); // ~1.64 ms @ 10 MHz
    if ((++bursts % 400) == 0)
    {
      vTaskDelay(1);
    } // occasional yield for WDT
  }
  void end()
  {
    if (!on)
      return;
    on = false;
    digitalWrite(SPI_CS_PIN, HIGH);
    vspi.endTransaction();
    vspi.end();
  }
}

// ---- Test 5: Single-shot pulse (BOOT) ----
namespace Mod5
{
  static volatile bool fire = false;
  static volatile uint32_t last = 0;
  void IRAM_ATTR ISR()
  {
    uint32_t now = millis();
    if (now - last > 150)
    {
      fire = true;
      last = now;
    }
  }
  void begin()
  {
    pinMode(PIN_PULSE, OUTPUT);
    digitalWrite(PIN_PULSE, LOW);
    pinMode(PIN_BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), ISR, FALLING);
  }
  void tick()
  {
    if (!fire)
      return;
    noInterrupts();
    fire = false;
    interrupts();
    digitalWrite(PIN_PULSE, HIGH);
    delayMicroseconds(1000);
    digitalWrite(PIN_PULSE, LOW);
  }
  void end()
  {
    detachInterrupt(digitalPinToInterrupt(PIN_BTN));
    digitalWrite(PIN_PULSE, LOW);
  }
}

// ---- Test 6: UART TX 0x55 sweep (keeps FIFO fed) ----
namespace Mod6
{
  static HardwareSerial U1(1);
  static const uint32_t BAUDS[] = {9600, 38400, 115200, 230400, 460800, 921600};
  static size_t i = 0;
  static uint32_t next = 0;
  static bool on = false;
  DMA_ATTR static uint8_t BUF[512];

  void begin()
  {
    for (size_t j = 0; j < sizeof(BUF); ++j)
      BUF[j] = 0x55;
    i = 0;
    U1.begin(BAUDS[0], SERIAL_8N1, -1, UART_TX_PIN);
    next = millis() + 5000;
    on = true;
  }
  void tick()
  {
    if (!on)
      return;
    size_t space = U1.availableForWrite();
    if (space)
    {
      size_t chunk = (space < sizeof(BUF)) ? space : sizeof(BUF);
      U1.write(BUF, chunk);
    }
    uint32_t now = millis();
    if ((int32_t)(now - next) >= 0)
    {
      i = (i + 1) % (sizeof(BAUDS) / sizeof(BAUDS[0]));
      U1.updateBaudRate(BAUDS[i]);
      next = now + 5000;
    }
  }
  void end()
  {
    if (!on)
      return;
    U1.end();
    on = false;
  }
}

// ---- Test 7: PWM steps @ 1 kHz (hardware) ----
namespace Mod7
{
  static const int CH = 0;
  static uint8_t duties[] = {25, 50, 75};
  static uint8_t j = 0;
  static uint32_t next = 0;
  static inline void apply(uint8_t d)
  {
    ledcSetup(CH, 1000, 10);
    ledcAttachPin(PWM_PIN, CH);
    uint32_t m = (1u << 10) - 1u;
    ledcWrite(CH, (m * d) / 100u);
  }
  void begin()
  {
    j = 0;
    apply(duties[j]);
    next = millis() + 6000;
  }
  void tick()
  {
    uint32_t now = millis();
    if ((int32_t)(now - next) >= 0)
    {
      j = (j + 1) % 3;
      apply(duties[j]);
      next = now + 6000;
    }
  }
  void end() { ledcSafeDetach(PWM_PIN); }
}

// ---- Test 8: static square ~50% @ PWM_FREQ (hardware) ----
namespace Mod8
{
  static const int CH = 0;
  static bool on = false;
  void begin()
  {
    ledcAttachPin(PWM_PIN, CH);
    ledcSetup(CH, PWM_FREQ, PWM_BITS);
    uint32_t m = (1u << PWM_BITS) - 1u;
    ledcWrite(CH, m / 2);
    on = true;
  }
  void tick() { (void)on; }
  void end()
  {
    if (!on)
      return;
    ledcSafeDetach(PWM_PIN);
    on = false;
  }
}

// ---- Test 9: 100 Hz ~50% (hardware) ----
namespace Mod9
{
  static const int CH = 0;
  static bool on = false;
  void begin()
  {
    ledcAttachPin(PWM_PIN, CH);
    ledcSetup(CH, 100, 12);
    ledcWrite(CH, 2048);
    on = true;
  }
  void tick() { (void)on; }
  void end()
  {
    if (!on)
      return;
    ledcSafeDetach(PWM_PIN);
    on = false;
  }
}
