################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Dev/4.MiddleWare/scheduler/subdir.mk
-include Dev/4.MiddleWare/os/baremetal/subdir.mk
-include Dev/4.MiddleWare/event/subdir.mk
-include Dev/3.Drivers/Devices/rc522/subdir.mk
-include Dev/3.Drivers/Devices/lcd1602_i2c/subdir.mk
-include Dev/3.Drivers/Devices/keypad4x4/subdir.mk
-include Dev/3.Drivers/Devices/eeprom24cxx/subdir.mk
-include Dev/3.Drivers/Devices/ds3234/subdir.mk
-include Dev/3.Drivers/Devices/ds1307/subdir.mk
-include Dev/3.Drivers/Components/dio/stm32/subdir.mk
-include Dev/3.Drivers/Components/SPI/stm32/subdir.mk
-include Dev/3.Drivers/Components/SPI/subdir.mk
-include Dev/3.Drivers/Components/I2C/stm32/subdir.mk
-include Dev/3.Drivers/Components/I2C/subdir.mk
-include Dev/2.BSP/bsp_systick/subdir.mk
-include Dev/2.BSP/bsp_rtc/subdir.mk
-include Dev/2.BSP/bsp_rc522/subdir.mk
-include Dev/2.BSP/bsp_lock/subdir.mk
-include Dev/2.BSP/bsp_lcd/subdir.mk
-include Dev/2.BSP/bsp_keypad/subdir.mk
-include Dev/2.BSP/bsp_i2c1/subdir.mk
-include Dev/2.BSP/bsp_exit_button/subdir.mk
-include Dev/2.BSP/bsp_eeprom/subdir.mk
-include Dev/1.Application/app_system/subdir.mk
-include Dev/1.Application/app_rtc/subdir.mk
-include Dev/1.Application/app_rc522/subdir.mk
-include Dev/1.Application/app_persist/subdir.mk
-include Dev/1.Application/app_log/subdir.mk
-include Dev/1.Application/app_lock/subdir.mk
-include Dev/1.Application/app_lcd/subdir.mk
-include Dev/1.Application/app_keypad/subdir.mk
-include Dev/1.Application/app_exit_button/subdir.mk
-include Dev/1.Application/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := RFID
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
RFID.elf \

MAP_FILES += \
RFID.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
RFID.list \

OBJCOPY_HEX += \
RFID.hex \


# All Target
all: main-build

# Main-build Target
main-build: RFID.elf secondary-outputs

# Tool invocations
RFID.elf RFID.map: $(OBJS) $(USER_OBJS) D:\PROJECT\LAB2209_PROG_TRAIN\STM32_Scheduler\Firmware\Platform\RFID\STM32F103C8TX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "RFID.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"D:\PROJECT\LAB2209_PROG_TRAIN\STM32_Scheduler\Firmware\Platform\RFID\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="RFID.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

RFID.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "RFID.list"
	@echo 'Finished building: $@'
	@echo ' '

RFID.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "RFID.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) RFID.elf RFID.hex RFID.list RFID.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
