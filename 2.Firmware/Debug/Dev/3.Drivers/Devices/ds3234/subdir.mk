################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Dev/3.Drivers/Devices/ds3234/ds3234.c 

OBJS += \
./Dev/3.Drivers/Devices/ds3234/ds3234.o 

C_DEPS += \
./Dev/3.Drivers/Devices/ds3234/ds3234.d 


# Each subdirectory must supply rules for building sources it contributes
Dev/3.Drivers/Devices/ds3234/%.o Dev/3.Drivers/Devices/ds3234/%.su Dev/3.Drivers/Devices/ds3234/%.cyclo: ../Dev/3.Drivers/Devices/ds3234/%.c Dev/3.Drivers/Devices/ds3234/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/3.Drivers/Components/dio" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/3.Drivers/Components/I2C" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/3.Drivers/Components/SPI" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/3.Drivers/Devices/ds3234" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Dev-2f-3-2e-Drivers-2f-Devices-2f-ds3234

clean-Dev-2f-3-2e-Drivers-2f-Devices-2f-ds3234:
	-$(RM) ./Dev/3.Drivers/Devices/ds3234/ds3234.cyclo ./Dev/3.Drivers/Devices/ds3234/ds3234.d ./Dev/3.Drivers/Devices/ds3234/ds3234.o ./Dev/3.Drivers/Devices/ds3234/ds3234.su

.PHONY: clean-Dev-2f-3-2e-Drivers-2f-Devices-2f-ds3234

