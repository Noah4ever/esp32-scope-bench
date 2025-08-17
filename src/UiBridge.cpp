#include "UiBridge.h"
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Tune these to taste
static constexpr size_t kMsgLen = 96;   // max bytes per message (incl. NUL)
static constexpr size_t kMsgCount = 16; // queue depth

struct UiMsg
{
    char text[kMsgLen];
};
static QueueHandle_t s_q = nullptr;

/**
 * Initialize the cross-core UI log bridge (create queue).
 * This function should be called once at the start of the program.
 */
void UiBridge_init()
{
    if (!s_q)
        s_q = xQueueCreate(kMsgCount, sizeof(UiMsg));
}

/**
 * Try to enqueue a formatted UI message (non-blocking).
 * Returns true if the message was successfully queued.
 * @param fmt The format string for the message.
 * @return true if the message was queued, false if the queue is full.
 */
bool UiBridge_logf(const char *fmt, ...)
{
    if (!s_q)
        return false; // not initialized yet
    UiMsg m;
    m.text[0] = '\0';
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(m.text, sizeof(m.text), fmt, ap);
    va_end(ap);
    // Non-blocking send; drop if full
    BaseType_t ok = xQueueSendToBack(s_q, &m, 0);
    return ok == pdTRUE;
}

/**
 * Drain pending messages to the provided stream (call from core 1 / UI).
 * This function will read all messages from the queue and print them to the stream.
 * It is non-blocking and will not wait for messages to arrive.
 * @param out The stream to which messages will be printed.
 */
void UiBridge_drain(Stream &out)
{
    if (!s_q)
        return;
    UiMsg m;
    while (xQueueReceive(s_q, &m, 0) == pdTRUE)
    {
        out.println(m.text);
    }
}