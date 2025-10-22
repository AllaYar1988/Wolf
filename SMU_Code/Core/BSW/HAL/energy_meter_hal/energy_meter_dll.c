/**
 * @file energy_meter_dll.c
 * @brief STPM34 Energy Meter Data Link Layer implementation
 *
 * Implements data link layer functions for STPM34 communication including
 * frame transmission, reception with DMA, and data buffering.
 *
 * @date Created on: Oct 16, 2025
 * @author Allahyar Moazami
 */

#include "energy_meter_dll.h"
#include "energy_meter_hal.h"
#include "platform.h"
#include "stm32f4xx_hal_uart.h"
#include <string.h>

/* ========================================================================
 * Private Variables
 * ======================================================================== */

/** @brief RX buffer for energy meter data */
static u8 gEnergyRxData[ENERGY_METER_BUFFER_SIZE];

/** @brief Number of bytes received in last transaction */
static u16 gRxByteCount = 0;

/** @brief DMA reception state flag */
static u8 gRxInitialized = 0;

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Send data to energy meter via UART with DMA
 *
 * Transmits data to the STPM34 energy meter using DMA.
 * Ensures UART is ready before starting transmission.
 *
 * @param[in] msg Pointer to message buffer
 * @param[in] size Size of message in bytes
 *
 * @note This function blocks briefly to ensure UART is ready
 */
void energy_meter_dll_send(u8 *msg, u16 size)
{
    /* Ensure UART is in ready state */
    ENERGY_METER_UART.gState = HAL_UART_STATE_READY;

    /* Transmit data via DMA */
    HAL_UART_Transmit_DMA(&ENERGY_METER_UART, msg, size);
}

/**
 * @brief Initialize DMA reception for energy meter
 *
 * Starts continuous DMA reception on UART4 for energy meter communication.
 * Should be called once during initialization.
 */
void energy_meter_dll_receive_init(void)
{
    if (!gRxInitialized) {
        /* Clear RX buffer */
        memset(gEnergyRxData, 0, ENERGY_METER_BUFFER_SIZE);

        /* Start DMA reception in circular mode */
        HAL_UART_Receive_DMA(&ENERGY_METER_UART, gEnergyRxData, ENERGY_METER_BUFFER_SIZE);

        gRxInitialized = 1;
    }
}

/**
 * @brief Check and process received energy meter data
 *
 * Checks if data has been received via DMA. Uses UART IDLE line detection
 * to determine when a frame has been completely received.
 *
 * @return Number of bytes received, 0 if no data available
 *
 * @note This function should be called periodically to poll for received data
 */
u16 energy_meter_dll_receive(void)
{
    u16 returnValue = 0;

    /* Check if UART IDLE flag is set (frame completed) */
    if (__HAL_UART_GET_FLAG(&ENERGY_METER_UART, UART_FLAG_IDLE))
    {
        uint32_t temp;

        /* Clear idle flag by reading SR and DR registers */
        temp = ENERGY_METER_UART.Instance->SR;
        temp = ENERGY_METER_UART.Instance->DR;
        (void)temp; /* Avoid compiler warning */

        /* Calculate number of bytes received */
        if (ENERGY_METER_UART.hdmarx != NULL) {
            gRxByteCount = ENERGY_METER_BUFFER_SIZE -
                          __HAL_DMA_GET_COUNTER(ENERGY_METER_UART.hdmarx);
            returnValue = gRxByteCount;
        }

        /* Note: DMA continues in circular mode, no need to restart */
    }

    return returnValue;
}

/**
 * @brief Get pointer to received data buffer
 *
 * Provides access to the internal RX buffer containing received data.
 *
 * @return Pointer to internal RX buffer
 *
 * @note Data in buffer is valid only after energy_meter_dll_receive()
 *       returns non-zero value
 */
u8* energy_meter_dll_get_rx_buffer(void)
{
    return gEnergyRxData;
}

/**
 * @brief Get number of bytes received in last transaction
 *
 * Returns the count of bytes received in the most recent reception event.
 *
 * @return Number of bytes received
 */
u16 energy_meter_dll_get_rx_count(void)
{
    return gRxByteCount;
}
