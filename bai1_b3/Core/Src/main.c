#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

UART_HandleTypeDef huart1;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;

#define MPU6050_ADDR          (0x68 << 1)
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

#define MAX7219_CS_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define MAX7219_CS_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

int16_t accel_x;
int16_t accel_y;
int16_t accel_z;

uint8_t accel_data[6];

char uart_buffer[100];

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);

void UART_SendString(char *str)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)str,
        strlen(str),
        100
    );
}


/* ================= MPU6050 ================= */

uint8_t MPU6050_Init(void)
{
    uint8_t data = 0x00;

    return HAL_I2C_Mem_Write(
        &hi2c1,
        MPU6050_ADDR,
        MPU6050_PWR_MGMT_1,
        I2C_MEMADD_SIZE_8BIT,
        &data,
        1,
        100
    ) == HAL_OK;
}

uint8_t MPU6050_Read_Accel(void)
{
    if (HAL_I2C_Mem_Read(
            &hi2c1,
            MPU6050_ADDR,
            MPU6050_ACCEL_XOUT_H,
            I2C_MEMADD_SIZE_8BIT,
            accel_data,
            6,
            100) != HAL_OK)
    {
        return 0;
    }

    accel_x = (int16_t)((accel_data[0] << 8) | accel_data[1]);
    accel_y = (int16_t)((accel_data[2] << 8) | accel_data[3]);
    accel_z = (int16_t)((accel_data[4] << 8) | accel_data[5]);

    return 1;
}


/* ================= MAX7219 ================= */

void MAX7219_Write(uint8_t address, uint8_t data)
{
    uint8_t tx[2];

    tx[0] = address;
    tx[1] = data;

    MAX7219_CS_LOW();

    HAL_SPI_Transmit(
        &hspi1,
        tx,
        2,
        100
    );

    MAX7219_CS_HIGH();
}

void MAX7219_Init(void)
{
    MAX7219_Write(0x0F, 0x00);
    MAX7219_Write(0x0C, 0x01);
    MAX7219_Write(0x0B, 0x07);
    MAX7219_Write(0x09, 0xFF);
    MAX7219_Write(0x0A, 0x08);

    MAX7219_Write(0x01, 0);
    MAX7219_Write(0x02, 0);
    MAX7219_Write(0x03, 0);
    MAX7219_Write(0x04, 0);
    MAX7219_Write(0x05, 0);
    MAX7219_Write(0x06, 0);
    MAX7219_Write(0x07, 0);
    MAX7219_Write(0x08, 0);
}


/* Hiển thị số nguyên trên MAX7219 */

void MAX7219_DisplayNumber(int32_t number)
{
    uint8_t digit;
    uint8_t i;

    if (number < 0)
    {
        number = -number;

        MAX7219_Write(0x01, 0x0A);

        for (i = 2; i <= 8; i++)
        {
            digit = number % 10;
            number /= 10;

            MAX7219_Write(i, digit);

            if (number == 0)
                break;
        }

        return;
    }

    for (i = 1; i <= 8; i++)
    {
        if (number > 0)
        {
            digit = number % 10;
            number /= 10;

            MAX7219_Write(i, digit);
        }
        else
        {
            MAX7219_Write(i, 0x0F);
        }
    }
}


/* ================= MAIN ================= */

int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_4,
        GPIO_PIN_SET
    );

    MAX7219_Init();

    UART_SendString("\r\n");
    UART_SendString("SYSTEM START\r\n");

    if (MPU6050_Init())
    {
        UART_SendString("MPU6050 INIT OK\r\n");
    }
    else
    {
        UART_SendString("MPU6050 INIT ERROR\r\n");
    }

    HAL_Delay(100);

    while (1)
    {
        if (MPU6050_Read_Accel())
        {
            sprintf(
                uart_buffer,
                "ACC X=%d Y=%d Z=%d\r\n",
                accel_x,
                accel_y,
                accel_z
            );

            UART_SendString(uart_buffer);

            MAX7219_DisplayNumber(accel_x);
        }
        else
        {
            UART_SendString("I2C READ ERROR\r\n");

            MAX7219_Write(0x01, 0x0E);
            MAX7219_Write(0x02, 0x0E);
            MAX7219_Write(0x03, 0x0E);
            MAX7219_Write(0x04, 0x0E);
            MAX7219_Write(0x05, 0x0E);
            MAX7219_Write(0x06, 0x0E);
            MAX7219_Write(0x07, 0x0E);
            MAX7219_Write(0x08, 0x0E);
        }

        HAL_Delay(200);
    }
}


/* ================= SPI1 ================= */

static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ================= UART1 ================= */

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;

    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ================= I2C1 ================= */

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;

    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ================= GPIO ================= */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_4,
        GPIO_PIN_SET
    );
}


/* ================= UART MSP ================= */

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;

        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}


/* ================= I2C MSP ================= */

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hi2c->Instance == I2C1)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}


/* ================= SPI MSP ================= */

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hspi->Instance == SPI1)
    {
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}


/* ================= CLOCK ================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ================= SYSTICK ================= */

void SysTick_Handler(void)
{
    HAL_IncTick();
}


/* ================= ERROR ================= */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}