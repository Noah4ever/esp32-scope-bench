#include "TestGuide.h"

// Note: keep lines short for Serial readability.
static const GuideSection G1 = {
    /*what*/ "Checks how well the scope shows faster signals.",
    /*why*/ "Finds the real usable speed (when squares stop looking square).",
    /*how*/ "ESP32 CH1: squares at 10k, 100k, 500k, 1M (as board allows).",
    /*setup*/ "Time/div: 10k→50–100us, 100k→5–10us, 500k→200–500ns, 1M→100–200ns. Volt/div: 1–2V. Trigger: rising ~1.6V.",
    /*expect*/ "Corners round & height may drop as speed rises. Note fastest that still looks square enough.",
    /*data*/ "Freq (Hz), Square? (Y/N), Height (div or Vpp)."};

static const GuideSection G2 = {
    "Zooms into the fastest rising edge.",
    "Sharper edges = better for fast events.",
    "Use fastest ESP32 square; trigger rising; zoom until edge ~full screen; read Rise Time if available.",
    "Start 1us/div then zoom down; go as low as scope allows. Volt/div ~1V. Trigger rising.",
    "Edge not vertical; some slope is normal on small scopes.",
    "Timebase used, Edge width (boxes or ms), Rise time (if shown)."};

static const GuideSection G3 = {
    "One‑shot pulse on button press.",
    "Checks if scope can catch rare events.",
    "Set Single trigger (rising); press ESP32 BOOT; pulse captured.",
    "Time/div 0.5–1ms; Volt/div 1V; Trigger rising ~1.6V; Mode: Single.",
    "Scope should freeze with pulse; width near programmed.",
    "Pulse width expected (ms), seen (ms), Caught 1st try? (Y/N)."};

static const GuideSection G4 = {
    "UART text/bit timing visibility.",
    "Useful for debugging sensors/boards.",
    "ESP32 TX→CH1; send 'HELLO 1234' or 0x55; try 9600, 38400, 115200, 230400+, decode if supported.",
    "Time/div: 9600→1ms, 38400→200us, 115200→50–100us, 230400→20–50us. Volt/div 1–2V. Trigger on start bit (falling).",
    "Lower speeds decode fine; errors at higher speeds are normal on small scopes.",
    "Baud, Decoded OK (Yes/Partial/No), Wrong chars?, If no decode: timing OK?"};

static const GuideSection G5 = {
    "PWM at 1kHz with duty steps.",
    "Confirms scope duty% matches set values.",
    "Set 25%, 50%, 75%; measure Duty% and Frequency.",
    "Time/div 100–200us; Volt/div 1–2V; Trigger rising mid‑level.",
    "Measured duty should be near target; freq ~1kHz.",
    "Target duty (%), Measured duty (%), Measured freq (Hz)."};

static const GuideSection G6 = {
    "Stability of triggered display.",
    "Steady trace = easier to work with.",
    "Stable square (100k–1M) or PWM_FREQ; trigger rising mid‑level; watch for drift.",
    "Time/div 2–10us (for 100k–1M); Volt/div 1–2V; Trigger rising.",
    "Small drift OK; big wander means weak triggering.",
    "Frequency (Hz), Stability (steady/slight/wanders), Trigger level."};

static const GuideSection G7 = {
    "How much detail remains when zooming stored data.",
    "Shows usefulness of memory depth.",
    "Slow square (100Hz); capture full screen; zoom/scroll; repeat at 1ms/div and 100us/div.",
    "Start 10ms/div; also try 1ms/div and 100us/div. Volt/div 1V.",
    "Some detail is lost on deep zoom; note when it looks blocky.",
    "Start timebase, How far you could zoom, When it looked blocky."};

const GuideSection *getGuide(uint8_t id)
{
    switch (id)
    {
    case 1:
        return &G1;
    case 2:
        return &G2;
    case 3:
        return &G3;
    case 4:
        return &G4;
    case 5:
        return &G5;
    case 6:
        return &G6;
    case 7:
        return &G7;
    default:
        return nullptr;
    }
}

static inline void printKV(Stream &s, const char *k, const char *v)
{
    s.print("  - ");
    s.print(k);
    s.print(": ");
    s.println(v);
}

void printGuideHeader(Stream &s, uint8_t id, const char *name)
{
    s.printf("\n=== Test %u: %s ===\n", id, name);
}
void printGuideAll(Stream &s, uint8_t id, const char *name)
{
    const GuideSection *g = getGuide(id);
    if (!g)
        return;
    // printGuideHeader(s, id, name);
    printKV(s, "What", g->what);
    printKV(s, "Why", g->why);
    printKV(s, "How", g->how);
    printKV(s, "Scope", g->setup);
    printKV(s, "Expect", g->expect);
    printKV(s, "Record", g->data);
}
void printGuideWhatWhy(Stream &s, uint8_t id)
{
    const GuideSection *g = getGuide(id);
    if (!g)
        return;
    printKV(s, "What", g->what);
    printKV(s, "Why", g->why);
}
void printGuideSetup(Stream &s, uint8_t id)
{
    const GuideSection *g = getGuide(id);
    if (!g)
        return;
    printKV(s, "Scope", g->setup);
}
void printGuideExpect(Stream &s, uint8_t id)
{
    const GuideSection *g = getGuide(id);
    if (!g)
        return;
    printKV(s, "Expect", g->expect);
}
void printGuideData(Stream &s, uint8_t id)
{
    const GuideSection *g = getGuide(id);
    if (!g)
        return;
    printKV(s, "Record", g->data);
}