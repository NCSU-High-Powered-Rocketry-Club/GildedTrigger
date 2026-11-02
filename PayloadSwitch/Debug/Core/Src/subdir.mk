################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/bma580.c \
../Core/Src/flash_memory_driver.c \
../Core/Src/flight_log.c \
../Core/Src/led.c \
../Core/Src/main.c \
../Core/Src/stm32u0xx_hal_msp.c \
../Core/Src/stm32u0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32u0xx.c \
../Core/Src/trigger.c 

OBJS += \
./Core/Src/bma580.o \
./Core/Src/flash_memory_driver.o \
./Core/Src/flight_log.o \
./Core/Src/led.o \
./Core/Src/main.o \
./Core/Src/stm32u0xx_hal_msp.o \
./Core/Src/stm32u0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32u0xx.o \
./Core/Src/trigger.o 

C_DEPS += \
./Core/Src/bma580.d \
./Core/Src/flash_memory_driver.d \
./Core/Src/flight_log.d \
./Core/Src/led.d \
./Core/Src/main.d \
./Core/Src/stm32u0xx_hal_msp.d \
./Core/Src/stm32u0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32u0xx.d \
./Core/Src/trigger.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U083xx -c -I../Core/Inc -I../Drivers/STM32U0xx_HAL_Driver/Inc -I../Drivers/STM32U0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/bma580.cyclo ./Core/Src/bma580.d ./Core/Src/bma580.o ./Core/Src/bma580.su ./Core/Src/flash_memory_driver.cyclo ./Core/Src/flash_memory_driver.d ./Core/Src/flash_memory_driver.o ./Core/Src/flash_memory_driver.su ./Core/Src/flight_log.cyclo ./Core/Src/flight_log.d ./Core/Src/flight_log.o ./Core/Src/flight_log.su ./Core/Src/led.cyclo ./Core/Src/led.d ./Core/Src/led.o ./Core/Src/led.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32u0xx_hal_msp.cyclo ./Core/Src/stm32u0xx_hal_msp.d ./Core/Src/stm32u0xx_hal_msp.o ./Core/Src/stm32u0xx_hal_msp.su ./Core/Src/stm32u0xx_it.cyclo ./Core/Src/stm32u0xx_it.d ./Core/Src/stm32u0xx_it.o ./Core/Src/stm32u0xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32u0xx.cyclo ./Core/Src/system_stm32u0xx.d ./Core/Src/system_stm32u0xx.o ./Core/Src/system_stm32u0xx.su ./Core/Src/trigger.cyclo ./Core/Src/trigger.d ./Core/Src/trigger.o ./Core/Src/trigger.su

.PHONY: clean-Core-2f-Src

