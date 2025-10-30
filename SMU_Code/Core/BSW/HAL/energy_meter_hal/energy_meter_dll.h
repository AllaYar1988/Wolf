/**
 * @file energy_meter_dll.h
 * @brief STPM34 Energy Meter Data Link Layer interface
 *
 * Provides data link layer functions for STPM34 communication including
 * frame transmission, reception with DMA, and CRC verification.
 *
 * @date Created on: Oct 16, 2025
 * @author Allahyar Moazami
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
 * @brief Send data to energy meter via UART with DMA
 * @param[in] msg Pointer to message buffer
 * @param[in] size Size of message in bytes
 */
void energy_meter_dll_send(u8 *msg, u16 size);

/**
 * @brief Initialize DMA reception for energy meter
 */
void energy_meter_dll_receive_init(void);

/**
 * @brief Check and process received energy meter data
 * @return Number of bytes received, 0 if no data available
 */
u16 energy_meter_dll_receive(void);

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

#endif /* BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_DLL_H_ */
