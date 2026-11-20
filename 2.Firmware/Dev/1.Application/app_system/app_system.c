/*
 * app_system.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app_system.h"
#include "app_rc522.h"
#include "app_persist.h"
#include "app_lock.h"
#include "app_keypad.h"
#include "app_rtc.h"
#include "app_log.h"
#include "rc522.h"
#include "os.h"
#include "define.h"

/* All durations in ms (os_tick_get()'s unit). ADMIN_AUTH_TIMEOUT_MS is
 * reused for every admin sub-state's inactivity timeout, not just the
 * password-entry step - one named constant for "how long without a key
 * before giving up on an admin interaction", not a separate one per
 * sub-state. */
#define APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS   (10000u)
#define APP_SYSTEM_MESSAGE_DURATION_MS     (2000u)
#define APP_SYSTEM_MAX_FAILED_ATTEMPTS     (5u)
#define APP_SYSTEM_LOCKOUT_DURATION_MS     (30000u)

typedef enum
{
    APP_SYSTEM_STATE_WELCOME,   /* transient splash shown once at boot, see app_system_init() */
    APP_SYSTEM_STATE_IDLE,
    APP_SYSTEM_STATE_ADMIN_AUTH,
    APP_SYSTEM_STATE_ADMIN_MENU,        /* the menu root itself - no separate sub-state for it */
    APP_SYSTEM_STATE_ADMIN_ADD_USER,       /* waiting for the new card to be scanned */
    APP_SYSTEM_STATE_ADMIN_ADD_USER_NAME,  /* card scanned - now typing its display name */
    APP_SYSTEM_STATE_ADMIN_DEL_USER,
    APP_SYSTEM_STATE_ADMIN_CHANGE_PW1,  /* enter new password */
    APP_SYSTEM_STATE_ADMIN_CHANGE_PW2,  /* confirm new password */
    APP_SYSTEM_STATE_ADMIN_FACTORY_RESET_CONFIRM,  /* re-enter admin password to confirm - UC-10 */
    APP_SYSTEM_STATE_ADMIN_MESSAGE,     /* transient result message, returns to ADMIN_MENU or IDLE */
    APP_SYSTEM_STATE_ACCESS_GRANTED,
    APP_SYSTEM_STATE_ACCESS_DENIED,
    APP_SYSTEM_STATE_LOCKED_OUT,
    APP_SYSTEM_STATE_EXITING
}
app_system_state_t;

static app_system_state_t s_state = APP_SYSTEM_STATE_IDLE;
static uint32_t s_ui32StateEnterTick = 0U;

static app_system_display_t s_display;
static uint32_t s_ui32DisplaySeq = 0U;

static uint32_t s_ui32RcUidSeq = 0U;
static uint32_t s_ui32RtcSeq = 0U;

/* Shared text-entry buffer for ADMIN_AUTH and ADMIN_CHANGE_PW1/PW2 -
 * only one of those is ever active at a time, so one buffer is enough. */
static char s_szEntryBuf[APP_PERSIST_ADMIN_PASSWORD_LEN];
static uint32_t s_ui32EntryLen = 0U;
static char s_szPendingNewPassword[APP_PERSIST_ADMIN_PASSWORD_LEN];

/* Name-entry buffer for ADMIN_ADD_USER_NAME - separate from s_szEntryBuf
 * (letters, not digits, and a different max length: APP_PERSIST_NAME_LEN
 * vs APP_PERSIST_ADMIN_PASSWORD_LEN) - and the UID already scanned in
 * ADMIN_ADD_USER, held here until the name is confirmed and both get
 * passed to app_persist_user_add() together. */
static char s_szNameEntryBuf[APP_PERSIST_NAME_LEN];
static uint32_t s_ui32NameEntryLen = 0U;
static uint8_t s_pui8PendingAddUid[RC522_UID_LEN];

static uint32_t s_ui32FailedAttempts = 0U;        /* RFID mismatches - UC-05 */
static uint32_t s_ui32AdminFailedAttempts = 0U;    /* wrong admin password - UC-05, separate counter */
static uint32_t s_ui32DelUserIndex = 0U;

/*==============================================================================
 * Small formatting helpers (no snprintf in this codebase - see define.h's
 * own ERROR_* convention of plain C, nothing from <stdio.h> pulled in)
 *============================================================================*/

static uint32_t app_system_utoa(uint32_t ui32Value, char *pOut)
{
    char pTmp[10];
    uint32_t ui32Count = 0U;

    if (ui32Value == 0U) {
        pTmp[ui32Count++] = '0';
    }

    while (ui32Value > 0U) {
        pTmp[ui32Count++] = (char)('0' + (ui32Value % 10U));
        ui32Value /= 10U;
    }

    for (uint32_t i = 0U; i < ui32Count; i++) {
        pOut[i] = pTmp[ui32Count - 1U - i];
    }

    return ui32Count;
}

static void app_system_format_clock(const ds1307_time_t *pTime, char pOut[9])
{
    pOut[0] = (char)('0' + (pTime->ui8Hours / 10U));
    pOut[1] = (char)('0' + (pTime->ui8Hours % 10U));
    pOut[2] = ':';
    pOut[3] = (char)('0' + (pTime->ui8Minutes / 10U));
    pOut[4] = (char)('0' + (pTime->ui8Minutes % 10U));
    pOut[5] = ':';
    pOut[6] = (char)('0' + (pTime->ui8Seconds / 10U));
    pOut[7] = (char)('0' + (pTime->ui8Seconds % 10U));
    pOut[8] = '\0';
}

/*==============================================================================
 * Display cache
 *============================================================================*/

static void app_system_set_display(const char *pszLine1, const char *pszLine2)
{
    char pBuf1[APP_SYSTEM_DISPLAY_LINE_LEN];
    char pBuf2[APP_SYSTEM_DISPLAY_LINE_LEN];

    strncpy(pBuf1, pszLine1, APP_SYSTEM_DISPLAY_LINE_LEN - 1U);
    pBuf1[APP_SYSTEM_DISPLAY_LINE_LEN - 1U] = '\0';
    strncpy(pBuf2, pszLine2, APP_SYSTEM_DISPLAY_LINE_LEN - 1U);
    pBuf2[APP_SYSTEM_DISPLAY_LINE_LEN - 1U] = '\0';

    if ((strcmp(s_display.szLine1, pBuf1) == 0) && (strcmp(s_display.szLine2, pBuf2) == 0)) {
        return; /* no real change - don't bump the sequence for nothing */
    }

    memcpy(s_display.szLine1, pBuf1, sizeof(pBuf1));
    memcpy(s_display.szLine2, pBuf2, sizeof(pBuf2));
    s_ui32DisplaySeq++;
}

/* Masked entry display ("****") for ADMIN_AUTH/CHANGE_PW1/PW2 - takes
 * the line-1 label since it differs per state, line-2 is always the
 * mask. */
static void app_system_show_entry(const char *pszLabel)
{
    char pStars[APP_SYSTEM_DISPLAY_LINE_LEN];
    uint32_t ui32N = (s_ui32EntryLen < (APP_SYSTEM_DISPLAY_LINE_LEN - 1U))
                          ? s_ui32EntryLen : (APP_SYSTEM_DISPLAY_LINE_LEN - 1U);

    for (uint32_t i = 0U; i < ui32N; i++) {
        pStars[i] = '*';
    }

    pStars[ui32N] = '\0';

    app_system_set_display(pszLabel, pStars);
}

/* Unmasked, unlike app_system_show_entry() - a display name isn't a
 * secret, so showing it as it's typed is more useful than stars. */
static void app_system_show_name_entry(void)
{
    app_system_set_display("Name(A-D),# ok", s_szNameEntryBuf);
}

static void app_system_show_idle_now(void)
{
    ds1307_time_t time;
    char pClock[9] = "--:--:--";

    if (app_rtc_get_time(&time)) {
        app_system_format_clock(&time, pClock);
    }

    app_system_set_display(pClock, "Quet the de mo");
}

/*==============================================================================
 * State transition helper
 *============================================================================*/

static void app_system_enter_state(app_system_state_t state)
{
    s_state = state;
    s_ui32StateEnterTick = os_tick_get();
}

static bool app_system_timed_out(uint32_t ui32TimeoutMs)
{
    return ((os_tick_get() - s_ui32StateEnterTick) >= ui32TimeoutMs);
}

static void app_system_show_admin_menu_root(void)
{
    app_system_set_display("1Add 2Del 3Pw", "4Exit 5Reset");
}

static void app_system_enter_admin_menu(void)
{
    app_system_enter_state(APP_SYSTEM_STATE_ADMIN_MENU);
    app_system_show_admin_menu_root();
}

/* Where ADMIN_MESSAGE goes once its 2s is up - ADMIN_MENU for every
 * ordinary admin action, IDLE only for a completed factory reset (the
 * admin password that got them into the menu no longer exists once
 * reset runs, so staying "inside" the menu doesn't make sense). */
static app_system_state_t s_eMessageReturnState = APP_SYSTEM_STATE_ADMIN_MENU;

static void app_system_show_message_then(const char *pszLine1, const char *pszLine2,
                                           app_system_state_t eReturnState)
{
    app_system_set_display(pszLine1, pszLine2);
    s_eMessageReturnState = eReturnState;
    app_system_enter_state(APP_SYSTEM_STATE_ADMIN_MESSAGE);
}

static void app_system_show_admin_message(const char *pszLine1, const char *pszLine2)
{
    app_system_show_message_then(pszLine1, pszLine2, APP_SYSTEM_STATE_ADMIN_MENU);
}

/*==============================================================================
 * Per-state tick handlers
 *============================================================================*/

static void app_system_tick_idle(char cKey, bool bUidReady, const uint8_t *pui8Uid)
{
    ds1307_time_t time;

    if (app_rtc_consume_if_updated(&time, &s_ui32RtcSeq)) {
        char pClock[9];
        app_system_format_clock(&time, pClock);
        app_system_set_display(pClock, "Quet the de mo");
    }

    if (bUidReady) {
        char szName[APP_PERSIST_NAME_LEN];

        if (app_persist_user_find_by_uid(pui8Uid, szName)) {
            app_lock_request_open();
            app_log_record(APP_LOG_SOURCE_RFID_GRANTED, pui8Uid);
            s_ui32FailedAttempts = 0U;
            app_system_enter_state(APP_SYSTEM_STATE_ACCESS_GRANTED);
            app_system_set_display("Access Granted", szName);
        } else {
            app_log_record(APP_LOG_SOURCE_RFID_DENIED, pui8Uid);
            s_ui32FailedAttempts++;

            if (s_ui32FailedAttempts >= APP_SYSTEM_MAX_FAILED_ATTEMPTS) {
                app_system_enter_state(APP_SYSTEM_STATE_LOCKED_OUT);
            } else {
                app_system_enter_state(APP_SYSTEM_STATE_ACCESS_DENIED);
                app_system_set_display("Access Denied", "Unknown Card");
            }
        }

        return; /* a card scan takes priority over any key this tick */
    }

    if ((cKey >= '0') && (cKey <= '9')) {
        s_szEntryBuf[0] = cKey;
        s_szEntryBuf[1] = '\0';
        s_ui32EntryLen = 1U;
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_AUTH);
        app_system_show_entry("Admin password:");
    }
}

static void app_system_tick_admin_auth(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_state(APP_SYSTEM_STATE_IDLE);
        app_system_show_idle_now();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    if (cKey == '*') {
        app_system_enter_state(APP_SYSTEM_STATE_IDLE);
        app_system_show_idle_now();
        return;
    }

    if (cKey == '#') {
        if (app_persist_admin_password_check(s_szEntryBuf)) {
            s_ui32AdminFailedAttempts = 0U;
            app_system_enter_admin_menu();
        } else {
            s_ui32AdminFailedAttempts++;

            if (s_ui32AdminFailedAttempts >= APP_SYSTEM_MAX_FAILED_ATTEMPTS) {
                app_system_enter_state(APP_SYSTEM_STATE_LOCKED_OUT);
            } else {
                app_system_enter_state(APP_SYSTEM_STATE_IDLE);
                app_system_show_idle_now();
            }
        }
        return;
    }

    if ((cKey >= '0') && (cKey <= '9') && (s_ui32EntryLen < (APP_PERSIST_ADMIN_PASSWORD_LEN - 1U))) {
        s_szEntryBuf[s_ui32EntryLen++] = cKey;
        s_szEntryBuf[s_ui32EntryLen] = '\0';
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_AUTH); /* resets the inactivity timer */
        app_system_show_entry("Admin password:");
    }
}

static void app_system_tick_admin_menu(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_state(APP_SYSTEM_STATE_IDLE);
        app_system_show_idle_now();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    app_system_enter_state(APP_SYSTEM_STATE_ADMIN_MENU); /* resets the inactivity timer */

    switch (cKey) {
        case '1':
            app_system_enter_state(APP_SYSTEM_STATE_ADMIN_ADD_USER);
            app_system_set_display("Scan new card", "then type name");
            break;

        case '2':
            s_ui32DelUserIndex = 0U;
            app_system_enter_state(APP_SYSTEM_STATE_ADMIN_DEL_USER);

            if (app_persist_user_count() == 0U) {
                app_system_show_admin_message("No users", "to delete");
            } else {
                uint8_t pui8Uid[RC522_UID_LEN];
                char szName[APP_PERSIST_NAME_LEN];
                (void)app_persist_user_get_by_index(0U, pui8Uid, szName);
                app_system_set_display("Del:#yes *next", szName);
            }
            break;

        case '3':
            s_ui32EntryLen = 0U;
            s_szEntryBuf[0] = '\0';
            app_system_enter_state(APP_SYSTEM_STATE_ADMIN_CHANGE_PW1);
            app_system_show_entry("New password:");
            break;

        case '5':
            s_ui32EntryLen = 0U;
            s_szEntryBuf[0] = '\0';
            app_system_enter_state(APP_SYSTEM_STATE_ADMIN_FACTORY_RESET_CONFIRM);
            app_system_show_entry("Confirm w/pass:");
            break;

        case '4':
        case '*':
            app_system_enter_state(APP_SYSTEM_STATE_IDLE);
            app_system_show_idle_now();
            break;

        default:
            app_system_show_admin_menu_root();
            break;
    }
}

static void app_system_tick_admin_add_user(char cKey, bool bUidReady, const uint8_t *pui8Uid)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS) || (cKey == '*')) {
        app_system_enter_admin_menu();
        return;
    }

    if (!bUidReady) {
        return;
    }

    memcpy(s_pui8PendingAddUid, pui8Uid, RC522_UID_LEN);
    s_ui32NameEntryLen = 0U;
    s_szNameEntryBuf[0] = '\0';
    app_system_enter_state(APP_SYSTEM_STATE_ADMIN_ADD_USER_NAME);
    app_system_show_name_entry();
}

/* Keypad only has A/B/C/D as letter keys (keypad4x4's keymap - no E/F
 * key exists on a standard 4x4 matrix), so the name alphabet is A-D,
 * not the A-F originally asked for - noted here since it's a hardware
 * constraint, not a design choice. */
static void app_system_tick_admin_add_user_name(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    if (cKey == '*') {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '#') {
        if (s_ui32NameEntryLen == 0U) {
            return; /* ignore empty confirm - need at least 1 letter */
        }

        uint32_t ui32Err = app_persist_user_add(s_pui8PendingAddUid, s_szNameEntryBuf);

        if (ui32Err == ERROR_OK) {
            app_system_show_admin_message("User added:", s_szNameEntryBuf);
        } else if (ui32Err == ERROR_FAIL) {
            app_system_show_admin_message("Card already", "registered");
        } else {
            app_system_show_admin_message("Add failed:", "table full");
        }

        return;
    }

    if ((cKey >= 'A') && (cKey <= 'D') && (s_ui32NameEntryLen < (APP_PERSIST_NAME_LEN - 1U))) {
        s_szNameEntryBuf[s_ui32NameEntryLen++] = cKey;
        s_szNameEntryBuf[s_ui32NameEntryLen] = '\0';
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_ADD_USER_NAME); /* resets the inactivity timer */
        app_system_show_name_entry();
    }
}

static void app_system_tick_admin_del_user(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    uint32_t ui32Count = app_persist_user_count();

    if (cKey == '*') {
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_DEL_USER); /* resets the inactivity timer */

        if (ui32Count > 0U) {
            s_ui32DelUserIndex = (s_ui32DelUserIndex + 1U) % ui32Count;
            uint8_t pui8Uid[RC522_UID_LEN];
            char szName[APP_PERSIST_NAME_LEN];
            (void)app_persist_user_get_by_index(s_ui32DelUserIndex, pui8Uid, szName);
            app_system_set_display("Del:#yes *next", szName);
        }

        return;
    }

    if (cKey == '#') {
        if (ui32Count == 0U) {
            app_system_enter_admin_menu();
            return;
        }

        uint8_t pui8Uid[RC522_UID_LEN];
        char szName[APP_PERSIST_NAME_LEN];
        (void)app_persist_user_get_by_index(s_ui32DelUserIndex, pui8Uid, szName);
        (void)app_persist_user_remove(pui8Uid);
        app_system_show_admin_message("Deleted:", szName);
        return;
    }

    /* any other key cancels - simplest rule, matches app-system.md */
    app_system_enter_admin_menu();
}

static void app_system_tick_admin_change_pw1(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    if (cKey == '*') {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '#') {
        if (s_ui32EntryLen == 0U) {
            return; /* ignore empty confirm */
        }

        strncpy(s_szPendingNewPassword, s_szEntryBuf, sizeof(s_szPendingNewPassword));
        s_ui32EntryLen = 0U;
        s_szEntryBuf[0] = '\0';
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_CHANGE_PW2);
        app_system_show_entry("Confirm again:");
        return;
    }

    if ((cKey >= '0') && (cKey <= '9') && (s_ui32EntryLen < (APP_PERSIST_ADMIN_PASSWORD_LEN - 1U))) {
        s_szEntryBuf[s_ui32EntryLen++] = cKey;
        s_szEntryBuf[s_ui32EntryLen] = '\0';
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_CHANGE_PW1);
        app_system_show_entry("New password:");
    }
}

static void app_system_tick_admin_change_pw2(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    if (cKey == '*') {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '#') {
        if (strcmp(s_szEntryBuf, s_szPendingNewPassword) == 0) {
            (void)app_persist_admin_password_set(s_szEntryBuf);
            app_system_show_admin_message("Password", "changed");
        } else {
            app_system_show_admin_message("Mismatch,", "try again");
        }

        s_ui32EntryLen = 0U;
        s_szEntryBuf[0] = '\0';
        memset(s_szPendingNewPassword, 0, sizeof(s_szPendingNewPassword));
        return;
    }

    if ((cKey >= '0') && (cKey <= '9') && (s_ui32EntryLen < (APP_PERSIST_ADMIN_PASSWORD_LEN - 1U))) {
        s_szEntryBuf[s_ui32EntryLen++] = cKey;
        s_szEntryBuf[s_ui32EntryLen] = '\0';
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_CHANGE_PW2);
        app_system_show_entry("Confirm again:");
    }
}

/* UC-10: re-enter the admin password (even though they already proved
 * it once to reach ADMIN_MENU) as a deliberate extra confirmation step
 * before a destructive, irreversible action. Wrong-password attempts
 * here count against the SAME s_ui32AdminFailedAttempts counter as
 * ADMIN_AUTH - this is another place a wrong admin password can be
 * tried, so it gets the same brute-force protection, not a separate
 * unprotected counter. */
static void app_system_tick_admin_factory_reset_confirm(char cKey)
{
    if (app_system_timed_out(APP_SYSTEM_ADMIN_AUTH_TIMEOUT_MS)) {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '\0') {
        return;
    }

    if (cKey == '*') {
        app_system_enter_admin_menu();
        return;
    }

    if (cKey == '#') {
        if (app_persist_admin_password_check(s_szEntryBuf)) {
            s_ui32AdminFailedAttempts = 0U;
            app_persist_factory_reset();
            app_system_show_message_then("Factory reset", "done", APP_SYSTEM_STATE_IDLE);
        } else {
            s_ui32AdminFailedAttempts++;

            if (s_ui32AdminFailedAttempts >= APP_SYSTEM_MAX_FAILED_ATTEMPTS) {
                app_system_enter_state(APP_SYSTEM_STATE_LOCKED_OUT);
            } else {
                app_system_show_message_then("Wrong password", "cancelled", APP_SYSTEM_STATE_ADMIN_MENU);
            }
        }
        return;
    }

    if ((cKey >= '0') && (cKey <= '9') && (s_ui32EntryLen < (APP_PERSIST_ADMIN_PASSWORD_LEN - 1U))) {
        s_szEntryBuf[s_ui32EntryLen++] = cKey;
        s_szEntryBuf[s_ui32EntryLen] = '\0';
        app_system_enter_state(APP_SYSTEM_STATE_ADMIN_FACTORY_RESET_CONFIRM); /* resets the inactivity timer */
        app_system_show_entry("Confirm w/pass:");
    }
}

static void app_system_tick_admin_message(void)
{
    if (app_system_timed_out(APP_SYSTEM_MESSAGE_DURATION_MS)) {
        if (s_eMessageReturnState == APP_SYSTEM_STATE_IDLE) {
            app_system_enter_state(APP_SYSTEM_STATE_IDLE);
            app_system_show_idle_now();
        } else {
            app_system_enter_admin_menu();
        }
    }
}

static void app_system_tick_transient_to_idle(void)
{
    if (app_system_timed_out(APP_SYSTEM_MESSAGE_DURATION_MS)) {
        app_system_enter_state(APP_SYSTEM_STATE_IDLE);
        app_system_show_idle_now();
    }
}

static void app_system_tick_locked_out(void)
{
    uint32_t ui32Elapsed = os_tick_get() - s_ui32StateEnterTick;

    if (ui32Elapsed >= APP_SYSTEM_LOCKOUT_DURATION_MS) {
        s_ui32FailedAttempts = 0U;
        s_ui32AdminFailedAttempts = 0U; /* reset both regardless of which one triggered this */
        app_system_enter_state(APP_SYSTEM_STATE_IDLE);
        app_system_show_idle_now();
        return;
    }

    uint32_t ui32RemainingSec = (APP_SYSTEM_LOCKOUT_DURATION_MS - ui32Elapsed) / 1000U;
    char pLine2[APP_SYSTEM_DISPLAY_LINE_LEN];
    uint32_t ui32Len = app_system_utoa(ui32RemainingSec, pLine2);
    pLine2[ui32Len++] = 's';
    pLine2[ui32Len] = '\0';

    app_system_set_display("Locked, wait:", pLine2);
}

/*==============================================================================
 * Public API
 *============================================================================*/

void app_system_init(void)
{
    memset(&s_display, 0, sizeof(s_display));
    app_system_enter_state(APP_SYSTEM_STATE_WELCOME);
    app_system_set_display("Welcome", "");
}

void app_system_task(void)
{
    char cKey = '\0';
    (void)app_keypad_get_key(&cKey);

    uint8_t pui8Uid[RC522_UID_LEN];
    bool bUidReady = app_rc522_consume_if_updated(pui8Uid, &s_ui32RcUidSeq);

    switch (s_state) {
        case APP_SYSTEM_STATE_WELCOME:
            app_system_tick_transient_to_idle(); /* 2s splash, then into IDLE's own clock display */
            break;

        case APP_SYSTEM_STATE_IDLE:
            app_system_tick_idle(cKey, bUidReady, pui8Uid);
            break;

        case APP_SYSTEM_STATE_ADMIN_AUTH:
            app_system_tick_admin_auth(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_MENU:
            app_system_tick_admin_menu(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_ADD_USER:
            app_system_tick_admin_add_user(cKey, bUidReady, pui8Uid);
            break;

        case APP_SYSTEM_STATE_ADMIN_ADD_USER_NAME:
            app_system_tick_admin_add_user_name(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_DEL_USER:
            app_system_tick_admin_del_user(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_CHANGE_PW1:
            app_system_tick_admin_change_pw1(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_CHANGE_PW2:
            app_system_tick_admin_change_pw2(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_FACTORY_RESET_CONFIRM:
            app_system_tick_admin_factory_reset_confirm(cKey);
            break;

        case APP_SYSTEM_STATE_ADMIN_MESSAGE:
            app_system_tick_admin_message();
            break;

        case APP_SYSTEM_STATE_ACCESS_GRANTED:
        case APP_SYSTEM_STATE_ACCESS_DENIED:
        case APP_SYSTEM_STATE_EXITING:
            app_system_tick_transient_to_idle();
            break;

        case APP_SYSTEM_STATE_LOCKED_OUT:
            app_system_tick_locked_out();
            break;

        default:
            app_system_enter_state(APP_SYSTEM_STATE_IDLE);
            break;
    }
}

void app_system_notify_exit_pressed(void)
{
    if (s_state == APP_SYSTEM_STATE_IDLE) {
        app_system_enter_state(APP_SYSTEM_STATE_EXITING);
        app_system_set_display("Exiting...", "");
    }
}

bool app_system_consume_display_if_updated(app_system_display_t *pOut, uint32_t *pui32LastSeenSeq)
{
    if ((pOut == NULL) || (pui32LastSeenSeq == NULL)) {
        return false;
    }

    if (*pui32LastSeenSeq == s_ui32DisplaySeq) {
        return false;
    }

    *pOut = s_display;
    *pui32LastSeenSeq = s_ui32DisplaySeq;

    return true;
}
