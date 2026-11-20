/*
 * app_exit_button.h
 *
 * Event handler for BSP_EXIT_BUTTON_EVENT_PRESSED
 * (Dev/2.BSP/bsp_exit_button, Dev/4.MiddleWare/event) - UC-03. Runs
 * from event_run()'s main-loop context (never from the EXTI ISR
 * itself) whenever the exit button was pressed, and issues app_lock's
 * open command - see app_lock.h for why that's the only thing this
 * module needs to know how to do.
 *
 * Debounces in software here (APP_EXIT_BUTTON_DEBOUNCE_MS in the .c
 * file) - contact bounce on PA11 can fire the EXTI several times per
 * physical press, and this is the layer that turns that into exactly
 * one accepted press.
 */

#pragma once

void app_exit_button_init(void);

/* Event handler - see app_main.c's g_pEventTable,
 * BSP_EXIT_BUTTON_EVENT_PRESSED (Dev/2.BSP/bsp_exit_button.h). */
void app_exit_button_on_press(void);
