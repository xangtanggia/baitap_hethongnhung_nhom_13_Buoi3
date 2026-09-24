TARGET = bai5

CPU = cortex-m3
MCU = STM32F103xB

PREFIX = arm-none-eabi-

CC = $(PREFIX)gcc
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

CPUFLAGS = -mcpu=$(CPU) -mthumb

CFLAGS = $(CPUFLAGS)

CFLAGS += -Wall
CFLAGS += -Wextra

CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections

CFLAGS += -g3
CFLAGS += -O0

CFLAGS += -DSTM32F103xB
CFLAGS += -DUSE_HAL_DRIVER

CFLAGS += -ICore/Inc

CFLAGS += -IDrivers/CMSIS/Include
CFLAGS += -IDrivers/CMSIS/Device/ST/STM32F1xx/Include

CFLAGS += -IDrivers/STM32F1xx_HAL_Driver/Inc
CFLAGS += -IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy

LDSCRIPT = STM32F103C8Tx_FLASH.ld

LDFLAGS = $(CPUFLAGS)

LDFLAGS += -T$(LDSCRIPT)

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(TARGET).map

LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs

C_SOURCES = \
Core/Src/main.c \
Core/Src/system_stm32f1xx.c

C_SOURCES += \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c\
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c\
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c

ASM_SOURCES = \
Startup/startup_stm32f103xb.s

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

all: $(TARGET).elf $(TARGET).hex $(TARGET).bin

%.o: %.c
	@echo "CC $<"
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	@echo "AS $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	@echo "LINK $@"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(TARGET).hex: $(TARGET).elf
	@echo "HEX $@"
	$(CP) -O ihex $< $@

$(TARGET).bin: $(TARGET).elf
	@echo "BIN $@"
	$(CP) -O binary $< $@

clean:
	rm -f $(C_OBJECTS)
	rm -f $(ASM_OBJECTS)
	rm -f $(TARGET).elf
	rm -f $(TARGET).hex
	rm -f $(TARGET).bin
	rm -f $(TARGET).map
