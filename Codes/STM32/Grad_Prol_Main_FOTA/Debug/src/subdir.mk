################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/HA_LedMatrix_program.c \
../src/HA_SSD_program.c \
../src/HA_STP_program.c \
../src/HA_TFT_program.c \
../src/MC_DMA_program.c \
../src/MC_EXTI_program.c \
../src/MC_GPIO_program.c \
../src/MC_NVIC_program.c \
../src/MC_RCC_program.c \
../src/MC_SPI_program.c \
../src/MC_STK_program.c \
../src/MC_USART_program.c \
../src/main.c 

OBJS += \
./src/HA_LedMatrix_program.o \
./src/HA_SSD_program.o \
./src/HA_STP_program.o \
./src/HA_TFT_program.o \
./src/MC_DMA_program.o \
./src/MC_EXTI_program.o \
./src/MC_GPIO_program.o \
./src/MC_NVIC_program.o \
./src/MC_RCC_program.o \
./src/MC_SPI_program.o \
./src/MC_STK_program.o \
./src/MC_USART_program.o \
./src/main.o 

C_DEPS += \
./src/HA_LedMatrix_program.d \
./src/HA_SSD_program.d \
./src/HA_STP_program.d \
./src/HA_TFT_program.d \
./src/MC_DMA_program.d \
./src/MC_EXTI_program.d \
./src/MC_GPIO_program.d \
./src/MC_NVIC_program.d \
./src/MC_RCC_program.d \
./src/MC_SPI_program.d \
./src/MC_STK_program.d \
./src/MC_USART_program.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F401xC -DUSE_HAL_DRIVER -DHSE_VALUE=16000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f4-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


