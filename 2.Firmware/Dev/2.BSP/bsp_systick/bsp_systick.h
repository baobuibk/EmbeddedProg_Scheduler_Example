/*
 * bsp_systick.h
 *
 * Strong override of the scheduler's weak platform_systick_init() (see
 * Dev/4.MiddleWare/scheduler/scheduler.h/.c) - configures the Cortex-M3
 * SysTick timer to actually fire at the rate scheduler_init() asks for.
 *
 * Nothing to call directly: scheduler_init() calls platform_systick_init()
 * for you. The other half of the wiring is in
 * Platform/RFID/Core/Src/stm32f1xx_it.c's SysTick_Handler, which must call
 * scheduler_systick_int_handler() on every tick (see the comment there).
 */

#pragma once
