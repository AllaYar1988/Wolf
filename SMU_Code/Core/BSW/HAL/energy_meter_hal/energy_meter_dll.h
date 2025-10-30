/**
 * @file energy_meter_dll.h
 * @brief STPM34 Energy Meter Data Link Layer interface
 *
 * Provides data link layer functions for STPM34 communication including
 * frame transmission, reception with DMA, CRC verification, and chip select
 * control.
 *
 * ARCHITECTURE DECISION: Chip Select Handling
 * ------------------------------------------
 * Chip select is managed at the DLL (Data Link Layer) because:
 * - DLL handles frame-level communication protocol
 * - Chip select timing is part of the physical layer protocol
 * - Application layer should not worry about low-level hardware signals
 * - Allows for proper transaction encapsulation
 *
 * CURRENT DESIGN: Single energy meter with hardcoded chip select macros
 * FUTURE EXTENSION: For multiple meters, modify functions to accept device instance
 *
 * @date Created on: Oct 16, 2025
 * @author A. Moazami
 */

#ifndef BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_DLL_H_
#define BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_DLL_H_

#include "platform.h"

/* ========================================================================
 * Constants
 * ======================================================================== */

#define ENERGY_METER_BUFFER_SIZE    100     /**< Size of energy meter RX buffer */
#define STPM34_FRAME_SIZE           5       /**< Size of STPM34 communication frame */

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief Initialize DMA reception for energy meter
 */
void energy_meter_dll_receive_init(void);

/**
 * @brief Start transaction and send data to energy meter
 *
 * Asserts chip select LOW, then transmits data via UART with DMA.
 * This function combines chip select control with data transmission.
 *
 * @param[in] msg Pointer to message buffer
 * @param[in] size Size of message in bytes
 *
 * @note Chip select remains LOW until energy_meter_dll_transaction_end() is called
 * @note For multiple meter support, this would take a device instance parameter
 */
void energy_meter_dll_transaction_send(u8 *msg, u16 size);

/**
 * @brief Check and process received energy meter data
 *
 * Checks if data has been received via DMA using UART IDLE detection.
 *
 * @return Number of bytes received, 0 if no data available
 */
u16 energy_meter_dll_receive(void);

/**
 * @brief End transaction and deselect energy meter
 *
 * De-asserts chip select HIGH to end the transaction.
 * Call this after receiving response or on timeout.
 *
 * @note For multiple meter support, this would take a device instance parameter
 */
void energy_meter_dll_transaction_end(void);

/**
 * @brief Get pointer to received data buffer
 * @return Pointer to internal RX buffer
 */
u8* energy_meter_dll_get_rx_buffer(void);

/**
 * @brief Get number of bytes received in last transaction
 * @return Number of bytes received
 */
u16 energy_meter_dll_get_rx_count(void);

/**
 * @brief Get DMA transmission statistics (for debugging)
 *
 * Returns counters that help debug DMA transmission issues.
 *
 * @param[out] tx_attempts Total number of transmission attempts (can be NULL)
 * @param[out] tx_success Number of successful HAL_UART_Transmit_DMA calls (can be NULL)
 */
void energy_meter_dll_get_tx_stats(u32 *tx_attempts, u32 *tx_success);

#endif /* BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_DLL_H_ */
