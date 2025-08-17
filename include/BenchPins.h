#pragma once
#include <Arduino.h>

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

static inline void
ledcSafeDetach(int pin)
{
#if ARDUINO_ESP32_MAJOR >= 2
    ledcDetachPin(pin);
#endif
}