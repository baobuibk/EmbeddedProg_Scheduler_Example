################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.c \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.c \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.c \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.c \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.c \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.c \
../Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.c 

OBJS += \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.o \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.o \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.o \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.o \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.o \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.o \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.o 

C_DEPS += \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.d \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.d \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.d \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.d \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.d \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.d \
./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32F1xx_HAL_Driver/Src/%.o Drivers/STM32F1xx_HAL_Driver/Src/%.su Drivers/STM32F1xx_HAL_Driver/Src/%.cyclo: ../Drivers/STM32F1xx_HAL_Driver/Src/%.c Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DSTM32F103x6 -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DHSI_VALUE=8000000 -DLSI_VALUE=40000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/scheduler" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/os" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/4.MiddleWare/event" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds1307" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/ds3234" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/eeprom24cxx" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/keypad4x4" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_i2c" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/dio" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/I2C" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Components/SPI" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_eeprom" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_exit_button" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_i2c1" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_keypad" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lcd" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_led" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lock" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_rtc" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_systick" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_exit_button" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_keypad" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_lcd" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_led" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_lock" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_log" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_persist" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_rc522" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_rtc" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application/app_system" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/5.Utils" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/1.Application" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/3.Drivers/Devices/lcd1602_parallel" -I"D:/PROJECT/LAB2209_PROG_TRAIN/STM32_Scheduler/Firmware/Dev/2.BSP/bsp_lcd_parallel" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-STM32F1xx_HAL_Driver-2f-Src

clean-Drivers-2f-STM32F1xx_HAL_Driver-2f-Src:
	-$(RM) ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.su ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.su ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.su ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.su ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.su ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.su ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.cyclo ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.d ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.o ./Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.su

.PHONY: clean-Drivers-2f-STM32F1xx_HAL_Driver-2f-Src

