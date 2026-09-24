#include "stm32f1xx_hal.h"

SPI_HandleTypeDef hspi1;

#define MAX7219_CS_LOW() \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)

#define MAX7219_CS_HIGH() \
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

void MAX7219_Write(uint8_t address, uint8_t data)
{
    uint8_t tx[2];

    tx[0] = address;
    tx[1] = data;

    MAX7219_CS_LOW();

    HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);

    MAX7219_CS_HIGH();
}


void MAX7219_Init(void)
{

    MAX7219_Write(0x09, 0x0F);
    MAX7219_Write(0x0A, 0x08);
    MAX7219_Write(0x0B, 0x03);
    MAX7219_Write(0x0C, 0x01);
    MAX7219_Write(0x0F, 0x00);

    MAX7219_Write(1, 0);
    MAX7219_Write(2, 0);
    MAX7219_Write(3, 0);
    MAX7219_Write(4, 0);
}


void MAX7219_DisplayNumber(uint16_t number)
{
    uint8_t digit0;
    uint8_t digit1;
    uint8_t digit2;
    uint8_t digit3;

    digit0 = number % 10;
    digit1 = (number / 10) % 10;
    digit2 = (number / 100) % 10;
    digit3 = (number / 1000) % 10;

    MAX7219_Write(1, digit0);
    MAX7219_Write(2, digit1);
    MAX7219_Write(3, digit2);
    MAX7219_Write(4, digit3);
}


int main(void)
{
    uint16_t count = 0;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();
    MAX7219_Init();

    while (1)
    {
        MAX7219_DisplayNumber(count);

        HAL_Delay(1000);

        count++;

        if (count > 9999)
        {
            count = 0;
        }
    }
}


void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while (1);
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

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        while (1);
    }
}


static void MX_SPI1_Init(void)
{
    __HAL_RCC_SPI1_CLK_ENABLE();

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
        while (1);
    }
}


static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();



    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}