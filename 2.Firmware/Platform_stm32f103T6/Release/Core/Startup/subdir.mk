################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f103c4tx.s 

OBJS += \
./Core/Startup/startup_stm32f103c4tx.o 

S_DEPS += \
./Core/Startup/startup_stm32f103c4tx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m3 -c -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/event" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds1307" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds3234" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/eeprom24cxx" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/keypad4x4" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_i2c" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/dio" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/I2C" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_eeprom" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_exit_button" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_i2c1" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_keypad" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lcd" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_led" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lock" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rtc" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_systick" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_exit_button" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_keypad" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_lcd" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_led" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_lock" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_log" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_persist" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_rtc" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_system" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_parallel" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lcd_parallel" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f103c4tx.d ./Core/Startup/startup_stm32f103c4tx.o

.PHONY: clean-Core-2f-Startup

