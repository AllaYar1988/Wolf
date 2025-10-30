/**
 * @file energy_meter_dll.c
 * @brief STPM34 Energy Meter Data Link Layer implementation
 *
 * Implements data link layer functions for STPM34 communication including
 * frame transmission, reception with DMA, and data buffering.
 *
 * @date Created on: Oct 16, 2025
 * @author A. Moazami
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

/** @brief Flag indicating data was read and buffer should be cleared */
static u8 gDataReadFlag = 0;

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Start transaction and send data to energy meter
 *
 * Asserts chip select LOW (active), then transmits data to the STPM34
 * energy meter using DMA. Chip select remains LOW until transaction_end()
 * is called.
 *
 * @param[in] msg Pointer to message buffer
 * @param[in] size Size of message in bytes
 *
 * @note Chip select timing is handled by DLL layer for proper protocol
 * @note For multiple meter support, add device instance parameter
 */
void energy_meter_dll_transaction_send(u8 *msg, u16 size)
{
    /* Assert chip select LOW (select device) */
    ENERGY_METER_CS_SELECT();

    /* Small delay to ensure chip select is stable before transmission */
    /* (typically 1-2 microseconds, but depends on hardware) */
    for (volatile int i = 0; i < 10; i++);

    /* Ensure UART is in ready state */
    ENERGY_METER_UART.gState = HAL_UART_STATE_READY;

    /* Transmit data via DMA */
    HAL_UART_Transmit_DMA(&ENERGY_METER_UART, msg, size);
}

/**
 * @brief End transaction and deselect energy meter
 *
 * De-asserts chip select HIGH (inactive) to end the communication transaction.
 * Should be called after receiving response or on timeout.
 *
 * @note For multiple meter support, add device instance parameter
 */
void energy_meter_dll_transaction_end(void)
{
    /* De-assert chip select HIGH (deselect device) */
    ENERGY_METER_CS_DESELECT();
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
 * @note After application reads data via get_rx_buffer(), next call will clear and restart DMA
 */
u16 energy_meter_dll_receive(void)
{
    u16 returnValue = 0;

    /* If previous data was read, clear buffer and restart DMA for next reception */
    if (gDataReadFlag) {
        /* Stop current DMA reception */
        HAL_UART_DMAStop(&ENERGY_METER_UART);

        /* Clear the buffer for next reception */
        memset(gEnergyRxData, 0, ENERGY_METER_BUFFER_SIZE);

        /* Restart DMA reception */
        HAL_UART_Receive_DMA(&ENERGY_METER_UART, gEnergyRxData, ENERGY_METER_BUFFER_SIZE);

        /* Clear the flag and byte count */
        gDataReadFlag = 0;
        gRxByteCount = 0;
    }

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

            /* Set flag to indicate data is ready to be read */
            if (gRxByteCount > 0) {
                gDataReadFlag = 1;
            }
        }
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
