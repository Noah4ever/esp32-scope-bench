#pragma once
#include <Arduino.h>
#include "TestBase.h"

namespace UIMenu
{
    using SelectFn = void (*)(uint8_t id);

    // Initialize UI module
    void init(Stream *io, TestRegistry *reg, SelectFn onSelect, bool ansiColors = false);

    // Poll input, print logs from core0, and handle commands
    void tick();

    // Force redraw of the main menu
    void showMenu();

    // Set currently active id & name (call when selection changed)
    void setActive(uint8_t id, const char *name);
}