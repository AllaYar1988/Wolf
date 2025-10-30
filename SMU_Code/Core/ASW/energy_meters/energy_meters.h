/**
 * @file energy_meters.h
 * @brief STPM34 Energy Meter Application Layer Interface
 *
 * Provides high-level interface for STPM34 energy metering IC operations.
 *
 * @date Created on: Oct 16, 2025
 * @author KHAJEHHOD-GD09
 */

#ifndef ASW_ENERGY_METERS_ENERGY_METERS_H_
#define ASW_ENERGY_METERS_ENERGY_METERS_H_

#include "platform.h"

/* ========================================================================
 * Constants
 * ======================================================================== */

/** @brief Timeout for energy meter operations (in handler cycles) */
#define ENERGY_METER_TIMEOUT 100

/* ========================================================================
 * Type Definitions
 * ======================================================================== */

/**
 * @enum EnuEnrgyMeterState
 * @brief Energy meter state machine states
 */
typedef enum {
    ENU_EM_INIT = 0,            /**< Initialization state */
    ENU_EM_SEND_READ_REQ,       /**< Send read request state */
    ENU_EM_WAIT_FOR_RESPONSE,   /**< Wait for response state */
    ENU_EM_STOP                 /**< Stop/idle state */
} EnuEnrgyMeterState;

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief Main energy meter handler (state machine)
 *
 * Call this function periodically to manage STPM34 communication.
 * Handles initialization, register reading, and response processing.
 *
 * @note Should be called every 10-50ms depending on application needs
 */
void energy_meters_handler(void);

/**
 * @brief Read value from STPM34 register
 *
 * Sends a read command to the specified STPM34 register.
 * The value is returned via the value pointer from the previous read.
 *
 * @param[in] addr Register address to read (0x00 - 0x32)
 * @param[out] value Pointer to store the read value (can be NULL)
 * @return 1 if successful, 0 if failed
 *
 * @note Due to STPM34's protocol, the returned value is from the
 *       previous transaction, not the current one
 */
u8 energy_meters_read_register(u8 addr, u32 *value);

/**
 * @brief Write value to STPM34 register
 *
 * Sends a write command to the specified STPM34 register.
 *
 * @param[in] addr Register address to write (0x00 - 0x1F)
 * @param[in] value 24-bit value to write
 * @return 1 if successful, 0 if failed
 */
u8 energy_meters_write_register(u8 addr, u32 value);

/**
 * @brief Get the last value read from STPM34
 *
 * Returns the most recently read value from any STPM34 register.
 *
 * @return Last 24-bit value read from STPM34
 */
u32 energy_meters_get_last_value(void);

#endif /* ASW_ENERGY_METERS_ENERGY_METERS_H_ */
