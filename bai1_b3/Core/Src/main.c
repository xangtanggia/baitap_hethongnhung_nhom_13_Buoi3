#include "main.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart1;
I2C_HandleTypeDef hi2c1;

#define MPU6050_ADDR          (0x68 << 1)
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

int16_t accel_x;
int16_t accel_y;
int16_t accel_z;
int16_t gyr_x;
int16_t gyr_y;
int16_t gyr_z;
int16_t temperature;
uint8_t accel_data[14];

char uart_buffer[100];

void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);




void UART_SendString(char *str)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)str,
        strlen(str),
        100
    );
}




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
            14,
            100) != HAL_OK)
    {
        return 0;
    }

    accel_x = (int16_t)((accel_data[0] << 8) | accel_data[1]);
    accel_y = (int16_t)((accel_data[2] << 8) | accel_data[3]);
    accel_z = (int16_t)((accel_data[4] << 8) | accel_data[5]);
    gyr_x = (int16_t)((accel_data[8] << 8) | accel_data[9]);
    gyr_y = (int16_t)((accel_data[10] << 8) | accel_data[11]);
    gyr_z = (int16_t)((accel_data[12] << 8) | accel_data[13]);
    temperature = (int16_t)((accel_data[6] << 8) | accel_data[7]);

    return 1;
}



int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();

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
                "ACC: X=%d Y=%d Z=%d GYR:X=%d Y=%d Z=%d temp:%d\r\n",
                (int)((accel_x/16384.0f)*10),
                (int)((accel_y/16384.0f)*10),
                (int)((accel_z/16384.0f)*10),
                (int)((gyr_x/131.0f)*(3.14f/180.0f)),
                (int)((gyr_y/131.0f)*(3.14f/180.0f)),
                (int)((gyr_z/131.0f)*(3.14f/180.0f)),
                (int)((temperature/340.0f)+36.53f)
            );

            UART_SendString(uart_buffer);
        }
        else
        {
            UART_SendString("I2C READ ERROR\r\n");
        }

        HAL_Delay(200);
    }
}




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




static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}




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



void SysTick_Handler(void)
{
    HAL_IncTick();
}



void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}