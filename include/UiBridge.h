#pragma once
#include <Arduino.h>

// Initialize the cross-core UI log bridge (create queue)
void UiBridge_init();

// Try to enqueue a formatted UI message (non-blocking). Returns true if queued.
bool UiBridge_logf(const char *fmt, ...);

// Drain pending messages to the provided stream (call from core 1 / UI)
void UiBridge_drain(Stream &out);