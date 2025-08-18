#include <Arduino.h>
extern "C"
{
#include "esp_task_wdt.h"
}
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "BenchPins.h"
#include "TestBase.h"
#include "UiMenu.h"
#include "UiBridge.h"

#include "tests/Test1_LEDCSweep.h"
#include "tests/Test2_SPIZoom.h"
#include "tests/Test3_SingleShot.h"
#include "tests/Test4_UARTSweep.h"
#include "tests/Test5_PWMSteps.h"
#include "tests/Test6_StaticSquare.h"
#include "tests/Test7_100Hz.h"

extern TestRegistry REG;

static TaskHandle_t g_worker = nullptr;
static QueueHandle_t g_cmdQ = nullptr;
static volatile uint8_t g_activeId = 0;
static Test *g_active = nullptr;

static void core0Worker(void *)
{
  uint32_t wdt_spin = 0;
  for (;;)
  {
    uint8_t cmd;
    if (xQueueReceive(g_cmdQ, &cmd, 0) == pdTRUE)
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
      vTaskDelay(1);
  }
}

static inline void sendCmd(uint8_t id)
{
  if (g_cmdQ)
    xQueueOverwrite(g_cmdQ, &id);
}

void setup()
{
  Serial.begin(115200);
  delay(120);
  UiBridge_init();
#ifndef KEEP_IDLE0_WDT
  esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(0));
#endif

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

  UIMenu::init(&Serial, &REG, sendCmd, /*ansiColors*/ true);
}

void loop()
{
  UIMenu::tick();
}