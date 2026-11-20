################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rtc/bsp_rtc.c 

OBJS += \
./Dev/2.BSP/bsp_rtc/bsp_rtc.o 

C_DEPS += \
./Dev/2.BSP/bsp_rtc/bsp_rtc.d 


# Each subdirectory must supply rules for building sources it contributes
Dev/2.BSP/bsp_rtc/bsp_rtc.o: D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rtc/bsp_rtc.c Dev/2.BSP/bsp_rtc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds1307" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds3234" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/eeprom24cxx" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/keypad4x4" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_i2c" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/dio" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/I2C" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_keypad" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_keypad" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_systick" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rtc" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_rtc" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_i2c1" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_eeprom" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/event" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_persist" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_lock" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_exit_button" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_exit_button" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lock" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_system" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_lcd" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lcd" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_log" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Dev-2f-2-2e-BSP-2f-bsp_rtc

clean-Dev-2f-2-2e-BSP-2f-bsp_rtc:
	-$(RM) ./Dev/2.BSP/bsp_rtc/bsp_rtc.cyclo ./Dev/2.BSP/bsp_rtc/bsp_rtc.d ./Dev/2.BSP/bsp_rtc/bsp_rtc.o ./Dev/2.BSP/bsp_rtc/bsp_rtc.su

.PHONY: clean-Dev-2f-2-2e-BSP-2f-bsp_rtc

