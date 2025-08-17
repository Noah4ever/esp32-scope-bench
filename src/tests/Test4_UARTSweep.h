#pragma once
#include <Arduino.h>
#include "BenchPins.h"
#include "TestBase.h"
#include "UiBridge.h"

class Test4_UARTSweep : public Test
{
  HardwareSerial U1{1};
  static const uint32_t BAUDS_[6];
  size_t i_ = 0;
  uint32_t next_ = 0;
  bool on_ = false;
  uint8_t buf_[512];

public:
  const char *name() const override { return "UART1 TX 0x55 sweep (8N1)"; }
  void pinHints(Stream &s) const override { s.printf("UART1 TX -> GPIO%d\n", UART_TX_PIN); }
  void begin() override
  {
    memset(buf_, 0x55, sizeof(buf_));
    i_ = 0;
    U1.begin(BAUDS_[0], SERIAL_8N1, -1, UART_TX_PIN);
    next_ = millis() + 5000;
    on_ = true;
    UiBridge_logf("[Test4] UART start @ %lu baud", (unsigned long)BAUDS_[0]);
  }
  void tick() override
  {
    if (!on_)
      return;
    size_t space = U1.availableForWrite();
    if (space)
    {
      size_t n = (space < sizeof(buf_)) ? space : sizeof(buf_);
      U1.write(buf_, n);
    }
    uint32_t now = millis();
    if ((int32_t)(now - next_) >= 0)
    {
      i_ = (i_ + 1) % 6;
      U1.updateBaudRate(BAUDS_[i_]);
      UiBridge_logf("[Test4] UART start @ %lu baud", (unsigned long)BAUDS_[i_]);
      next_ = now + 5000;
    }
  }
  void end() override
  {
    if (!on_)
      return;
    U1.end();
    on_ = false;
  }
};
