// ESP32 Dual-Core Test Harness — ALL tests on core 0 (fast), UI/menu on core 1
// Serial 115200: 'm' = menu, '1'..'7' = select, '0' = stop.
// Override pins via platformio.ini build_flags.

#include <Arduino.h>
#include <SPI.h>
extern "C"
{
#include "esp_task_wdt.h"
}

#include "BenchPins.h"
#include "UiBridge.h"
#include "TestBase.h"
// tests
#include "tests/Test1_LEDCSweep.h"
#include "tests/Test2_SPIZoom.h"
#include "tests/Test3_SingleShot.h"
#include "tests/Test4_UARTSweep.h"
#include "tests/Test5_PWMSteps.h"
#include "tests/Test6_StaticSquare.h"
#include "tests/Test7_100Hz.h"

// ---------- Core 0 worker ----------
static TaskHandle_t g_worker = nullptr; // worker task on core 0
static QueueHandle_t g_cmdQ = nullptr;  // command queue for core 0
static volatile uint8_t g_activeId = 0; // currently active test ID
static Test *g_active = nullptr;        // currently active test instance

static void core0Worker(void *)
{
  uint32_t wdt_spin = 0;
  for (;;)
  {
    uint8_t cmd;
    if (xQueueReceive(g_cmdQ, &cmd, 0) == pdTRUE) // receive command from queue
    {
      if (cmd != g_activeId)
      {
        if (g_active)
          g_active->end();
        g_activeId = cmd;
        g_active = REG.byId(g_activeId);
        if (g_active)
          g_active->begin();
      }
    }

    if (g_active)
      g_active->tick();
    else
      vTaskDelay(1);

    if ((++wdt_spin & 0x1FF) == 0)
      vTaskDelay(1); // let IDLE0 run
  }
}

// ---------- UI on core 1 ----------
static void printMenu() { REG.printMenu(Serial); }
static void printHints(uint8_t id) { REG.printHints(Serial, id); }
static inline void sendCmd(uint8_t id) { xQueueOverwrite(g_cmdQ, &id); }

void setup()
{
  Serial.begin(115200);
  delay(120);
  UiBridge_init();
#ifndef KEEP_IDLE0_WDT
  esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(0)); // disable core0 idle WDT for continuous output
#endif

  // register tests
  static Test1_LEDCSweep t1;
  REG.add(1, &t1);
  static Test2_SPIZoom t2;
  REG.add(2, &t2);
  static Test3_SingleShot t3;
  REG.add(3, &t3);
  static Test4_UARTSweep t4;
  REG.add(4, &t4);
  static Test5_PWMSteps t5;
  REG.add(5, &t5);
  static Test6_StaticSquare t6;
  REG.add(6, &t6);
  static Test7_100Hz t7;
  REG.add(7, &t7);
  // new tests here:

  g_cmdQ = xQueueCreate(1, sizeof(uint8_t));
  xTaskCreatePinnedToCore(core0Worker, "core0", 4096, nullptr, 2, &g_worker, 0);
  printMenu();
}

void loop()
{
  UiBridge_drain(Serial);

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
    sendCmd(0);
    Serial.println("(stopped)");
    return;
  }
  if (c >= '1' && c <= '9')
  {
    uint8_t id = (uint8_t)(c - '0');
    if (REG.byId(id))
    {
      uint8_t id = static_cast<uint8_t>(c - '0');
      if (id >= 1 && id <= REG.length && REG.byId(id))
      {
        sendCmd(id);
        Serial.printf("-> Test %u\n", id);
        REG.printHints(Serial, id);
      }
    }
  }
}