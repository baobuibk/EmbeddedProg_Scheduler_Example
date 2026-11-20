################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Dev/3.Drivers/SPI/stm32/stm32_spi_io.c 

OBJS += \
./Dev/3.Drivers/SPI/stm32/stm32_spi_io.o 

C_DEPS += \
./Dev/3.Drivers/SPI/stm32/stm32_spi_io.d 


# Each subdirectory must supply rules for building sources it contributes
Dev/3.Drivers/SPI/stm32/%.o Dev/3.Drivers/SPI/stm32/%.su Dev/3.Drivers/SPI/stm32/%.cyclo: ../Dev/3.Drivers/SPI/stm32/%.c Dev/3.Drivers/SPI/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103x6 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/2.Firmware/Dev/3.Drivers/SPI" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Dev-2f-3-2e-Drivers-2f-SPI-2f-stm32

clean-Dev-2f-3-2e-Drivers-2f-SPI-2f-stm32:
	-$(RM) ./Dev/3.Drivers/SPI/stm32/stm32_spi_io.cyclo ./Dev/3.Drivers/SPI/stm32/stm32_spi_io.d ./Dev/3.Drivers/SPI/stm32/stm32_spi_io.o ./Dev/3.Drivers/SPI/stm32/stm32_spi_io.su

.PHONY: clean-Dev-2f-3-2e-Drivers-2f-SPI-2f-stm32

