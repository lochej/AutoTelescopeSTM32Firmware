################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Arduino_HAL.c \
../Src/adc.c \
../Src/dma.c \
../Src/gpio.c \
../Src/main.c \
../Src/nmea_parser.c \
../Src/nmea_receiver.c \
../Src/printf_redirect.c \
../Src/rtc.c \
../Src/starmap.c \
../Src/stepper_adapt2.c \
../Src/stepper_it.c \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_it.c \
../Src/syscalls.c \
../Src/system_stm32l4xx.c \
../Src/telescope_app.c \
../Src/telescope_commands.c \
../Src/telescope_utils.c \
../Src/tim.c \
../Src/time_util.c \
../Src/timer.c \
../Src/uart_driver.c \
../Src/usart.c 

OBJS += \
./Src/Arduino_HAL.o \
./Src/adc.o \
./Src/dma.o \
./Src/gpio.o \
./Src/main.o \
./Src/nmea_parser.o \
./Src/nmea_receiver.o \
./Src/printf_redirect.o \
./Src/rtc.o \
./Src/starmap.o \
./Src/stepper_adapt2.o \
./Src/stepper_it.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_it.o \
./Src/syscalls.o \
./Src/system_stm32l4xx.o \
./Src/telescope_app.o \
./Src/telescope_commands.o \
./Src/telescope_utils.o \
./Src/tim.o \
./Src/time_util.o \
./Src/timer.o \
./Src/uart_driver.o \
./Src/usart.o 

C_DEPS += \
./Src/Arduino_HAL.d \
./Src/adc.d \
./Src/dma.d \
./Src/gpio.d \
./Src/main.d \
./Src/nmea_parser.d \
./Src/nmea_receiver.d \
./Src/printf_redirect.d \
./Src/rtc.d \
./Src/starmap.d \
./Src/stepper_adapt2.d \
./Src/stepper_it.d \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_it.d \
./Src/syscalls.d \
./Src/system_stm32l4xx.d \
./Src/telescope_app.d \
./Src/telescope_commands.d \
./Src/telescope_utils.d \
./Src/tim.d \
./Src/time_util.d \
./Src/timer.d \
./Src/uart_driver.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32L452xx -I"D:/DevRoot/eclipse-workspace-oxygen-C/STM32L452_Telescope_App/Inc" -I"D:/DevRoot/eclipse-workspace-oxygen-C/STM32L452_Telescope_App/Drivers/STM32L4xx_HAL_Driver/Inc" -I"D:/DevRoot/eclipse-workspace-oxygen-C/STM32L452_Telescope_App/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy" -I"D:/DevRoot/eclipse-workspace-oxygen-C/STM32L452_Telescope_App/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"D:/DevRoot/eclipse-workspace-oxygen-C/STM32L452_Telescope_App/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


