#pragma once
#include <Arduino.h>

struct GuideSection
{
    const char *what;   // what it does
    const char *why;    // why it matters
    const char *how;    // how to reproduce (steps)
    const char *setup;  // scope setup: time/div, volt/div, trigger
    const char *expect; // what to expect
    const char *data;   // what to write down
};

// Access guide by test id (1..7). Returns nullptr if unknown.
const GuideSection *getGuide(uint8_t id);

// Pretty printers for one guide section (compact, monitor‑friendly)
void printGuideHeader(Stream &s, uint8_t id, const char *name);
void printGuideAll(Stream &s, uint8_t id, const char *name);
void printGuideWhatWhy(Stream &s, uint8_t id);
void printGuideSetup(Stream &s, uint8_t id);
void printGuideExpect(Stream &s, uint8_t id);
void printGuideData(Stream &s, uint8_t id);