/**
 * @file energy_meters.h
 * @brief STPM34 Energy Meter Application Layer Interface
 *
 * Provides high-level interface for STPM34 energy metering IC operations.
 *
 * @date Created on: Oct 16, 2025
 * @author A. Moazami
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

/**
 * @enum EnuEnergyMeterStatus
 * @brief Energy meter operation status codes
 */
typedef enum {
    ENU_EM_STATUS_IDLE = 0,         /**< No response yet, waiting */
    ENU_EM_STATUS_SUCCESS,          /**< Valid response received */
    ENU_EM_STATUS_TIMEOUT,          /**< Response timeout occurred */
    ENU_EM_STATUS_CRC_ERROR         /**< CRC validation failed */
} EnuEnergyMeterStatus;

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

/**
 * @brief Read active power from specified channel
 *
 * Reads and converts the active power measurement to Watts.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return Active power in Watts (W), or 0.0 if channel invalid
 */
float energy_meters_read_active_power(u8 channel);

/**
 * @brief Read reactive power from specified channel
 *
 * Reads and converts the reactive power measurement to VAR.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return Reactive power in VAR, or 0.0 if channel invalid
 */
float energy_meters_read_reactive_power(u8 channel);

/**
 * @brief Read RMS current from specified channel
 *
 * Reads and converts the RMS current measurement to milliamps.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return RMS current in milliamps (mA), or 0.0 if channel invalid
 */
float energy_meters_read_rms_current(u8 channel);

/**
 * @brief Read RMS voltage from specified channel
 *
 * Reads and converts the RMS voltage measurement to Volts.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return RMS voltage in Volts (V), or 0.0 if channel invalid
 */
float energy_meters_read_rms_voltage(u8 channel);

/**
 * @brief Set calibration factors for voltage and current
 *
 * Sets calibration multipliers to adjust for measurement errors.
 * Default values are 1.0 (no calibration).
 *
 * @param[in] voltage_cal Voltage calibration factor (typically 0.9 - 1.1)
 * @param[in] current_cal Current calibration factor (typically 0.9 - 1.1)
 */
void energy_meters_set_calibration(float voltage_cal, float current_cal);

/**
 * @brief Get diagnostic statistics for energy meter communication
 *
 * Retrieves counters for successful transactions, timeouts, and CRC errors.
 * Useful for monitoring communication health and debugging.
 *
 * @param[out] success_count Pointer to store successful transaction count (can be NULL)
 * @param[out] timeout_count Pointer to store timeout occurrence count (can be NULL)
 * @param[out] crc_error_count Pointer to store CRC error count (can be NULL)
 */
void energy_meters_get_statistics(u32 *success_count, u32 *timeout_count, u32 *crc_error_count);

/**
 * @brief Reset diagnostic statistics counters
 *
 * Resets all diagnostic counters (success, timeout, CRC errors) to zero.
 * Useful for periodic statistics collection or after maintenance.
 */
void energy_meters_reset_statistics(void);

#endif /* ASW_ENERGY_METERS_ENERGY_METERS_H_ */
