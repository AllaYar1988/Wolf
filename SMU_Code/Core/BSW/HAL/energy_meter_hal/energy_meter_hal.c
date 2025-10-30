/**
 * @file energy_meter_hal.c
 * @brief STPM34 Energy Meter HAL layer implementation
 *
 * This module provides hardware abstraction for the STPM34 energy metering IC.
 * Initializes UART4 with DMA for communication and GPIO for chip select control.
 *
 * @date Created on: Oct 16, 2025
 * @author Allahyar Moazami
 */

#include "energy_meter_hal.h"
#include "stm32f4xx_hal.h"

/* ========================================================================
 * Global Variables
 * ======================================================================== */

/** @brief UART4 handle for energy meter communication */
UART_HandleTypeDef huart4;

/** @brief DMA handle for UART4 TX */
DMA_HandleTypeDef hdma_usart4_tx;

/** @brief DMA handle for UART4 RX */
DMA_HandleTypeDef hdma_usart4_rx;

/* ========================================================================
 * Static Function Prototypes
 * ======================================================================== */

static void energy_meters_hal_serial_Init(void);
static void energy_meters_hal_gpio_Init(void);

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Initialize energy meter hardware
 *
 * Initializes GPIO for chip select and UART4 for communication.
 * This function must be called before any energy meter operations.
 */
void energy_meters_hal_init(void)
{
    /* Initialize GPIO for chip select */
    energy_meters_hal_gpio_Init();

    /* Initialize UART4 for communication */
    energy_meters_hal_serial_Init();

    /* Set chip select high (inactive) initially */
    HAL_GPIO_WritePin(uGenerator_STPM_SCS_GPIO_Port, uGenerator_STPM_SCS_Pin, GPIO_PIN_SET);
}

/**
 * @brief Enable UART4 DMA for energy meter
 *
 * Enables the DMA channels for UART4. Must be called before DMA operations.
 * Note: DMA initialization is done in HAL_UART_MspInit() in usart.c
 */
void energy_meters_hal_enable_dma(void)
{
    /* Enable DMA clock if not already enabled */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Link DMA handles to UART (if not done during init) */
    if (ENERGY_METER_UART.hdmatx == NULL) {
        __HAL_LINKDMA(&ENERGY_METER_UART, hdmatx, hdma_usart4_tx);
    }

    if (ENERGY_METER_UART.hdmarx == NULL) {
        __HAL_LINKDMA(&ENERGY_METER_UART, hdmarx, hdma_usart4_rx);
    }
}

/* ========================================================================
 * Static Function Implementations
 * ======================================================================== */

/**
 * @brief Initialize GPIO for chip select
 *
 * Configures the GPIO pin used for STPM34 chip select as output.
 */
static void energy_meters_hal_gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOB clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin : uGenerator_STPM_SCS */
    GPIO_InitStruct.Pin = uGenerator_STPM_SCS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(uGenerator_STPM_SCS_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief Initialize UART4 for energy meter communication
 *
 * Configures UART4 with 9600 baud rate, 8 data bits, no parity, 1 stop bit.
 * DMA is initialized separately in HAL_UART_MspInit() callback.
 */
static void energy_meters_hal_serial_Init(void)
{
    ENERGY_METER_UART.Instance = UART4;
    ENERGY_METER_UART.Init.BaudRate = 9600;
    ENERGY_METER_UART.Init.WordLength = UART_WORDLENGTH_8B;
    ENERGY_METER_UART.Init.StopBits = UART_STOPBITS_1;
    ENERGY_METER_UART.Init.Parity = UART_PARITY_NONE;
    ENERGY_METER_UART.Init.Mode = UART_MODE_TX_RX;
    ENERGY_METER_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    ENERGY_METER_UART.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&ENERGY_METER_UART) != HAL_OK)
    {
        /* Initialization Error - Could call Error_Handler() if available */
    }
}
