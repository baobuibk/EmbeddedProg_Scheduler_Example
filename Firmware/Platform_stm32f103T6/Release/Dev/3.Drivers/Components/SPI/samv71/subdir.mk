################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.c 

OBJS += \
./Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.o 

C_DEPS += \
./Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.d 


# Each subdirectory must supply rules for building sources it contributes
Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.o: D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.c Dev/3.Drivers/Components/SPI/samv71/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DSTM32F103x6 -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DHSI_VALUE=8000000 -DLSI_VALUE=40000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Dev-2f-3-2e-Drivers-2f-Components-2f-SPI-2f-samv71

clean-Dev-2f-3-2e-Drivers-2f-Components-2f-SPI-2f-samv71:
	-$(RM) ./Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.cyclo ./Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.d ./Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.o ./Dev/3.Drivers/Components/SPI/samv71/samv71_spi_io.su

.PHONY: clean-Dev-2f-3-2e-Drivers-2f-Components-2f-SPI-2f-samv71

