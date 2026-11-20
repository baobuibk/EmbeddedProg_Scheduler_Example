################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_keypad.c \
D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_systick.c 

OBJS += \
./Dev/2.BSP/bsp_keypad.o \
./Dev/2.BSP/bsp_systick.o 

C_DEPS += \
./Dev/2.BSP/bsp_keypad.d \
./Dev/2.BSP/bsp_systick.d 


# Each subdirectory must supply rules for building sources it contributes
Dev/2.BSP/bsp_keypad.o: D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_keypad.c Dev/2.BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds1307" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds3234" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/eeprom24cxx" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/keypad4x4" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_i2c" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/dio" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/I2C" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Dev/2.BSP/bsp_systick.o: D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_systick.c Dev/2.BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds1307" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds3234" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/eeprom24cxx" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/keypad4x4" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_i2c" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/dio" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/I2C" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Dev-2f-2-2e-BSP

clean-Dev-2f-2-2e-BSP:
	-$(RM) ./Dev/2.BSP/bsp_keypad.cyclo ./Dev/2.BSP/bsp_keypad.d ./Dev/2.BSP/bsp_keypad.o ./Dev/2.BSP/bsp_keypad.su ./Dev/2.BSP/bsp_systick.cyclo ./Dev/2.BSP/bsp_systick.d ./Dev/2.BSP/bsp_systick.o ./Dev/2.BSP/bsp_systick.su

.PHONY: clean-Dev-2f-2-2e-BSP

