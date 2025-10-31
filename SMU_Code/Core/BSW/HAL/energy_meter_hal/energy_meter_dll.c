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

/** @brief TX buffer to ensure data persists during DMA transmission */
static u8 gEnergyTxData[STPM34_FRAME_SIZE];

/** @brief Flag indicating TX is in progress */
static volatile u8 gTxBusy = 0;

/** @brief TX complete flag (set by HAL_UART_TxCpltCallback) */
static volatile u8 gTxComplete = 0;

/** @brief RX complete flag (set by HAL_UART_RxCpltCallback) */
static volatile u8 gRxComplete = 0;

/** @brief Counter for transmission attempts (for debugging) */
static volatile u32 gTxAttemptCount = 0;

/** @brief Counter for successful transmissions (for debugging) */
static volatile u32 gTxSuccessCount = 0;

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
 * @param[in] size Size of message in bytes (max STPM34_FRAME_SIZE)
 *
 * @note Chip select timing is handled by DLL layer for proper protocol
 * @note Data is copied to internal buffer to ensure it persists during DMA
 * @note DMA completion triggers HAL_UART_TxCpltCallback / HAL_UART_RxCpltCallback
 * @note For multiple meter support, add device instance parameter
 */
void energy_meter_dll_transaction_send(u8 *msg, u16 size)
{
    HAL_StatusTypeDef status;

    /* Increment attempt counter for debugging */
    gTxAttemptCount++;

    /* Validate size */
    if (size > STPM34_FRAME_SIZE) {
        size = STPM34_FRAME_SIZE;
    }

    /* Copy data to internal TX buffer (DMA needs persistent buffer) */
    memcpy(gEnergyTxData, msg, size);

    /* Assert chip select LOW (select device) */
    ENERGY_METER_CS_SELECT();

    /* Small delay to ensure chip select is stable before transmission */
    for (volatile int i = 0; i < 10; i++);

    /* Clear completion flags */
    gTxComplete = 0;
    gRxComplete = 0;

    /* Abort any ongoing DMA transfers to ensure clean state */
    HAL_UART_AbortTransmit(&ENERGY_METER_UART);
    HAL_UART_AbortReceive(&ENERGY_METER_UART);

    /* Wait a bit for abort to complete */
    for (volatile int i = 0; i < 100; i++);

    /* Start DMA reception first (must be ready before TX) */
    HAL_UART_Receive_DMA(&ENERGY_METER_UART, gEnergyRxData, STPM34_FRAME_SIZE);

    /* Then transmit data via DMA */
    status = HAL_UART_Transmit_DMA(&ENERGY_METER_UART, gEnergyTxData, size);

    /* Track status */
    if (status == HAL_OK) {
        gTxSuccessCount++;
        gTxBusy = 1;
    } else {
        /* Transmission failed - still mark as busy but it will timeout */
        gTxBusy = 1;
    }
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

    /* Clear TX busy flag (transaction is complete) */
    gTxBusy = 0;
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
 * Checks if data has been received via interrupt mode (IT).
 * Uses gRxComplete flag set by HAL_UART_RxCpltCallback.
 *
 * @return Number of bytes received, 0 if no data available
 *
 * @note This function should be called periodically to poll for received data
 * @note After application reads data via get_rx_buffer(), reception is complete
 */
u16 energy_meter_dll_receive(void)
{
    u16 returnValue = 0;

    /* Check if RX complete flag is set by interrupt callback */
    if (gRxComplete)
    {
        /* Data received successfully */
        gRxByteCount = STPM34_FRAME_SIZE;
        returnValue = gRxByteCount;

        /* Clear the flag (one-time read) */
        gRxComplete = 0;
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

/**
 * @brief Get DMA transmission statistics (for debugging)
 *
 * Returns counters that help debug DMA transmission issues.
 * Useful to check if HAL_UART_Transmit_DMA is being called and succeeding.
 *
 * @param[out] tx_attempts Total number of transmission attempts (can be NULL)
 * @param[out] tx_success Number of successful HAL_UART_Transmit_DMA calls (can be NULL)
 */
void energy_meter_dll_get_tx_stats(u32 *tx_attempts, u32 *tx_success)
{
    if (tx_attempts != NULL) {
        *tx_attempts = gTxAttemptCount;
    }

    if (tx_success != NULL) {
        *tx_success = gTxSuccessCount;
    }
}
