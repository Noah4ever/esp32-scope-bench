#include "UiMenu.h"
#include "UiBridge.h"
#include "TestGuide.h"

namespace
{
    Stream *IO = nullptr;
    TestRegistry *REGp = nullptr;
    UIMenu::SelectFn cb = nullptr;
    bool useAnsi = false;
    uint8_t activeId = 0;
    const char *activeName = "";
    char line[96];
    uint8_t linelen = 0;

    void hr() { IO->println("----------------------------------------"); }

    void help()
    {
        IO->println("\nCommands:");
        IO->println("  m         : menu");
        IO->println("  1..7      : select test");
        IO->println("  log       : flush logs");
        IO->println("  0         : stop");
        IO->println("  h, help   : this help");
    }

    void header()
    {
        IO->println("            ESP32 SCOPE BENCH");
        hr();
    }

    void handleSelect(uint8_t id)
    {
        if (!REGp)
            return;
        Test *t = REGp->byId(id);
        if (!t)
            return;
        activeId = id;
        activeName = t->name();
        if (cb)
            cb(id); // tell core0 to start test
        IO->printf("-> T%u: %s\n", id, activeName);
        REGp->printHints(*IO, id);
        IO->println();
    }

    void processCommand(const char *cmd)
    {
        if (!cmd || !*cmd)
            return;
        if (!strcmp(cmd, "m"))
        {
            UIMenu::showMenu();
            return;
        }
        if (!strcmp(cmd, "h") || !strcmp(cmd, "help"))
        {
            help();
            return;
        }
        if (!strcmp(cmd, "log"))
        {
            UiBridge_drain(*IO);
            return;
        }
        if (!strcmp(cmd, "0"))
        {
            if (cb)
                cb(0);
            IO->println("(stopped)");
            return;
        }

        if (strlen(cmd) == 1 && cmd[0] >= '1' && cmd[0] <= '9')
        {
            uint8_t id = (uint8_t)(cmd[0] - '0');
            if (REGp->byId(id))
            {
                handleSelect(id);
                return;
            }
        }
        IO->printf("Unknown cmd: %s (h for help)\n", cmd);
    }
}

void UIMenu::init(Stream *io, TestRegistry *reg, SelectFn onSelect, bool ansiColors)
{
    IO = io;
    REGp = reg;
    cb = onSelect;
    useAnsi = ansiColors;
    linelen = 0;
    line[0] = '\0';
    header();
    if (REGp)
        REGp->printMenu(*IO);
    help();
}

void UIMenu::tick()
{
    UiBridge_drain(*IO); // 1) drain logs first

    while (IO->available())
    {
        char c = (char)IO->read();
        if (c == '\r')
            continue;

        // Instant single‑key commands (no Enter needed)
        if (c == 'm')
        {
            UIMenu::showMenu();
            continue;
        }
        if (c == 'h')
        {
            help();
            continue;
        }
        if (c == '0')
        {
            if (cb)
                cb(0);
            IO->println("(stopped)");
            continue;
        }
        if (c >= '1' && c <= '9')
        {
            uint8_t id = (uint8_t)(c - '0');
            if (REGp->byId(id))
            {
                IO->println("\n");
                printGuideAll(*IO, id, activeName);
                delay(1);
                handleSelect(id);
            }
            continue;
        }

        if (c == '\n' || c == ' ')
        {
            line[linelen] = '\0';
            processCommand(line);
            linelen = 0;
            line[0] = '\0';
            continue;
        }
        if (linelen + 1 < sizeof(line))
            line[linelen++] = c;
    }
}

void UIMenu::showMenu()
{
    header();
    if (REGp)
        REGp->printMenu(*IO);
}
void UIMenu::setActive(uint8_t id, const char *name)
{
    activeId = id;
    activeName = name ? name : "";
}