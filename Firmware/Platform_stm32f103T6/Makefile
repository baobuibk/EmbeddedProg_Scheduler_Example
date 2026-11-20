##############################################################################
# Makefile for the Scheduler firmware project (STM32F103C4Tx / low-density)
#
# Hand-written, standalone build (independent from the STM32CubeIDE-generated
# Debug/Release/ makefiles). Builds this project's own Core/, Drivers/ and
# everything under ../Dev/ (recursively, so new scheduler/application files
# just need to be dropped into Dev/<layer>/ - no manual registration needed
# here).
#
# Layout note: this project sits directly under Firmware/, as a sibling of
# Firmware/Dev/ and Firmware/Platform/RFID/ (see Firmware/Makefile, the
# equivalent build for the RFID project) - Dev/ is one level up from here and
# is linked into this Eclipse project as a linked resource (see .project),
# precisely so both build paths see the same files.
#
# Usage:
#   make            # -> build/Scheduler.elf/.hex/.bin/.map
#   make clean      # remove build output
#   make OPT=-O2    # override optimization level
##############################################################################

TARGET    = Scheduler
BUILD_DIR = build

######################################
# Toolchain
######################################
PREFIX = arm-none-eabi-
CC     = $(PREFIX)gcc
AS     = $(PREFIX)gcc -x assembler-with-cpp
CP     = $(PREFIX)objcopy
SZ     = $(PREFIX)size
HEX    = $(CP) -O ihex
BIN    = $(CP) -O binary -S

######################################
# MCU
######################################
MCU = -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

######################################
# Defines (match this project's .ioc / .cproject)
######################################
C_DEFS = \
-DSTM32F103x6 \
-DUSE_FULL_LL_DRIVER

######################################
# Project layout
######################################
PLATFORM_DIR = .
DEV_DIR      = ../Dev

######################################
# Recursive wildcard helper (pure GNU Make, no shell dependency)
######################################
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

######################################
# Sources
######################################
ASM_SOURCES = $(PLATFORM_DIR)/Core/Startup/startup_stm32f103c4tx.s

C_SOURCES_CORE = $(wildcard $(PLATFORM_DIR)/Core/Src/*.c)
C_SOURCES_HAL  = $(wildcard $(PLATFORM_DIR)/Drivers/STM32F1xx_HAL_Driver/Src/*.c)

# Reference/other-platform backends under Dev/3.Drivers/Components/ -
# excluded because each one defines the same symbols as the stm32/
# backend actually used by this project (not just tidy - duplicate
# definitions would fail to link if more than one were built):
#   - Components/SPI/samv71/      -> see Components/SPI/stm32/
#   - Components/dio/nxp/         -> see Components/dio/stm32/
#   - Components/dio/samv71/      -> see Components/dio/stm32/
#   - Components/dio/dummy/       -> weak stand-in, not needed once a
#     real backend (stm32/) is built
DEV_EXCLUDE_DIRS = \
$(DEV_DIR)/3.Drivers/Components/SPI/samv71/% \
$(DEV_DIR)/3.Drivers/Components/dio/nxp/% \
$(DEV_DIR)/3.Drivers/Components/dio/samv71/% \
$(DEV_DIR)/3.Drivers/Components/dio/dummy/%

C_SOURCES_DEV  = $(filter-out $(DEV_EXCLUDE_DIRS),$(call rwildcard,$(DEV_DIR)/,*.c))

C_SOURCES = $(C_SOURCES_CORE) $(C_SOURCES_HAL) $(C_SOURCES_DEV)

######################################
# Includes
######################################
C_INCLUDES = \
-I$(PLATFORM_DIR)/Core/Inc \
-I$(PLATFORM_DIR)/Drivers/STM32F1xx_HAL_Driver/Inc \
-I$(PLATFORM_DIR)/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-I$(PLATFORM_DIR)/Drivers/CMSIS/Device/ST/STM32F1xx/Include \
-I$(PLATFORM_DIR)/Drivers/CMSIS/Include

# Any directory under Dev/ that contains a .h is added as an include path,
# so headers in Dev/4.MiddleWare/, Dev/5.Utils/, etc. are found automatically.
DEV_INC_DIRS = $(sort $(dir $(call rwildcard,$(DEV_DIR)/,*.h)))
C_INCLUDES  += $(addprefix -I,$(DEV_INC_DIRS))

######################################
# Compile flags
######################################
OPT ?= -Og

CFLAGS  = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -g -gdwarf-2
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

ASFLAGS = $(MCU) -g -gdwarf-2

######################################
# Link flags
######################################
LDSCRIPT = $(PLATFORM_DIR)/STM32F103C4TX_FLASH.ld
LIBS     = -lc -lm -lnosys
LDFLAGS  = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS) \
           -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

######################################
# Portable mkdir/rm (no POSIX shell/coreutils assumed - works whether make
# is invoked from PowerShell, cmd.exe or Git Bash). On Windows, SHELL is
# pinned to cmd.exe so recipes always run through it, regardless of a
# SHELL env var inherited from Git Bash (whose MSYS sh.exe would otherwise
# mangle "/c"-style arguments into a drive path and break this).
######################################
ifeq ($(OS),Windows_NT)
SHELL     := cmd.exe
MKDIR_CMD = if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
RM_CMD    = if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
else
MKDIR_CMD = mkdir -p $(BUILD_DIR)
RM_CMD    = rm -rf $(BUILD_DIR)
endif

######################################
# Build rules
######################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

.PHONY: all clean size

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

$(BUILD_DIR):
	$(MKDIR_CMD)

clean:
	$(RM_CMD)

-include $(wildcard $(BUILD_DIR)/*.d)
