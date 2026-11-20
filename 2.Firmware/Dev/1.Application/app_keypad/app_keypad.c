/*
 * app_keypad.c
 */

#include <stddef.h>
#include <stdbool.h>

#include "app_keypad.h"
#include "keypad4x4.h"
#include "bsp_keypad.h"

static keypad4x4_t s_keypad;
static char s_cPendingKey = KEYPAD4X4_NO_KEY;

void app_keypad_init(void)
{
    bsp_keypad_init();
    keypad4x4_init(&s_keypad);
}

void app_keypad_task(void)
{
    char key = keypad4x4_scan(&s_keypad);

    if (key != KEYPAD4X4_NO_KEY) {
        s_cPendingKey = key;
    }
}

bool app_keypad_get_key(char *pcKey)
{
    if ((pcKey == NULL) || (s_cPendingKey == KEYPAD4X4_NO_KEY)) {
        return false;
    }

    *pcKey = s_cPendingKey;
    s_cPendingKey = KEYPAD4X4_NO_KEY;

    return true;
}
