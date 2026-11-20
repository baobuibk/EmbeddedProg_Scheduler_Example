# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

This is a bare-metal STM32 firmware lab exercise (`LAB2209_PROG_TRAIN`): implementing a task scheduler
on an STM32F103C6T6 (Cortex-M3, 32KB Flash / 10KB RAM, LQFP48). The project is currently a freshly
generated STM32CubeIDE/CubeMX skeleton — `main.c` only initializes HAL and the system clock (HSI, no
PLL); no peripherals beyond RCC/NVIC are configured yet, and no scheduler logic has been written.

Top-level layout (numbered by workflow stage):
- `1.Document/` — lab docs/specs (currently empty).
- `2.Firmware/` — the STM32CubeIDE project (see below).
- `3.Tools/`, `4.Reference/`, `5.Release/` — currently empty placeholders.

## Firmware project structure (`2.Firmware/`)

- `template.ioc` — the CubeMX configuration file (MCU: `STM32F103C6Tx`). Project name in CubeMX is
  `template`, even though the repo is `STM32_Scheduler`. Regenerating code from this file (CubeMX
  "Generate Code") will overwrite `Core/` and `Drivers/`, preserving only `/* USER CODE BEGIN/END */`
  blocks — it does **not** touch anything under `Dev/`.
- `Core/` — CubeMX-generated boilerplate: `Inc/`+`Src/` (`main.c`, HAL config/IRQ handlers,
  `system_stm32f1xx.c`) and `Startup/` (reset handler + vector table, `.s` file). Treat as generated
  code; only edit inside `USER CODE` markers so regeneration doesn't clobber changes.
- `Drivers/` — CMSIS device headers and the STM32F1xx HAL driver sources, vendored by CubeMX. Not
  meant to be hand-edited.
- `Dev/` — scaffold intentionally separate from CubeMX-generated code, for the custom
  application/scheduler code this lab is about:
  - `1.Application/` — application-level tasks/logic.
  - `2.BSP/` — board support (peripheral wrappers above HAL).
  - `3.Drivers/` — custom low-level drivers not provided by STM32Cube HAL.
  - `4.MiddleWare/` — scheduler core / middleware (the heart of this lab; already has a
    `scheduler/` subfolder in progress).
  - `5.Utils/` — shared utilities.
  The top-level `Makefile` (see Build below) picks up any `.c`/`.h` added anywhere under `Dev/`
  automatically (recursive glob, any nesting). The STM32CubeIDE-managed build does **not**: new
  sources there still need matching entries added to `2.Firmware/.cproject` for the IDE/`Debug/`
  build to see them.
- `STM32F103C6TX_FLASH.ld` — linker script (32KB FLASH / 10KB RAM memory map).
- `Debug/` — the generated Eclipse CDT build directory (`makefile`, `sources.mk`, `objects.mk`,
  per-subdir `subdir.mk`, and build output `2.Firmware.elf`/`.map`/`.list`). Entirely auto-generated;
  never hand-edit — rebuilding or reopening the project in STM32CubeIDE regenerates these.

## Build

Both `arm-none-eabi-gcc` (GNU Arm Embedded toolchain) and GNU Make are on PATH. Two independent
build paths exist side by side:

- **`2.Firmware/Makefile`** (hand-written, preferred for CLI work) — builds `Core/`, `Drivers/` and
  everything under `Dev/` recursively, with no registration step needed for new `Dev/` files:
  ```
  cd 2.Firmware
  make          # -> build/2.Firmware.elf/.hex/.bin/.map
  make clean
  ```
  On Windows this pins `SHELL := cmd.exe` internally so it behaves identically from PowerShell, cmd,
  or Git Bash (Git Bash otherwise inherits an MSYS `sh.exe` via `$SHELL` that mangles `cmd /c`-style
  arguments — don't remove that pin).
- **`2.Firmware/Debug/`** — the STM32CubeIDE/Eclipse CDT-generated build (`makefile`, `sources.mk`,
  `objects.mk`, per-subdir `subdir.mk`); also runnable via `cd 2.Firmware/Debug && make -j`, or from
  STM32CubeIDE itself (Project > Build). This one only compiles sources registered in
  `2.Firmware/.cproject`, so new files under `Dev/` need a matching entry there too (see `Dev/` note
  above) — the top-level `Makefile` has no such requirement.

There is no separate lint or test target for the firmware.

Flashing/debugging is done through STM32CubeIDE's ST-Link debug/run configuration (no CLI flash
script is present in the repo).
