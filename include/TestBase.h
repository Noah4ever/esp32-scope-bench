#pragma once
#include <Arduino.h>

class Test
{
public:
    virtual ~Test() {}
    virtual const char *name() const = 0;       // for menu
    virtual void pinHints(Stream &s) const = 0; // print wiring hints
    virtual void begin() = 0;
    virtual void tick() = 0;
    virtual void end() = 0;
};

struct TestEntry
{
    uint8_t id;
    Test *test;
};

class TestRegistry
{
public:
    static constexpr int kMax = 16;
    int length = 0;
    bool add(uint8_t id, Test *t)
    {
        if (n_ >= kMax)
            return false;
        items_[n_++] = {id, t};
        length++;
        return true;
    }
    Test *byId(uint8_t id) const
    {
        for (int i = 0; i < n_; ++i)
            if (items_[i].id == id)
                return items_[i].test;
        return nullptr;
    }
    uint8_t minId() const
    {
        if (!n_)
            return 0;
        uint8_t m = items_[0].id;
        for (int i = 1; i < n_; ++i)
            m = items_[i].id < m ? items_[i].id : m;
        return m;
    }
    uint8_t maxId() const
    {
        if (!n_)
            return 0;
        uint8_t M = items_[0].id;
        for (int i = 1; i < n_; ++i)
            M = items_[i].id > M ? items_[i].id : M;
        return M;
    }
    void printMenu(Stream &s) const
    {
        s.println("\n=== Menu (core1) ===");
        for (int i = 0; i < n_; ++i)
            s.printf(" %u) %s\n", items_[i].id, items_[i].test->name());
        s.printf(" 0) Stop\nRange: %u..%u\n", minId(), maxId());
    }
    void printHints(Stream &s, uint8_t id) const
    {
        Test *t = byId(id);
        if (!t)
            return;
        s.println("\n-- Probe / Wiring --");
        t->pinHints(s);
        s.println("GND -> GND");
    }

private:
    TestEntry items_[kMax];
    int n_ = 0;
};

extern TestRegistry REG;