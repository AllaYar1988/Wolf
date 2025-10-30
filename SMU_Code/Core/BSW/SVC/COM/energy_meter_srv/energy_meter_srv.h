/**
 * @file energy_meter_srv.h
 * @brief Energy Meter Service Layer
 *
 * Provides high-level service API for STPM34 energy meter operations.
 * Service layer abstracts complexity of state machine, DLL, and HAL layers,
 * offering simple blocking/non-blocking functions with timeout handling.
 *
 * ARCHITECTURE:
 * -------------
 * Application → Service Layer (THIS) → Application/State Machine → DLL → HAL
 *
 * The service layer:
 * - Provides blocking read/write with timeout
 * - Manages request/response synchronization
 * - Checks service availability
 * - Handles errors and retries
 * - Simplifies application code
 *
 * @date Created on: Oct 30, 2025
 * @author A. Moazami
 */

#ifndef BSW_SVC_COM_ENERGY_METER_SRV_ENERGY_METER_SRV_H_
#define BSW_SVC_COM_ENERGY_METER_SRV_ENERGY_METER_SRV_H_

#include "platform.h"
#include "energy_meter_hal.h"

/* ========================================================================
 * Constants
 * ======================================================================== */

/** @brief Default timeout for service operations (in milliseconds) */
#define ENERGY_METER_SRV_DEFAULT_TIMEOUT_MS  1000

/** @brief Service retry count before giving up */
#define ENERGY_METER_SRV_MAX_RETRIES  3

/* ========================================================================
 * Type Definitions
 * ======================================================================== */

/**
 * @enum EnuEnergyMeterSrvStatus
 * @brief Service operation status codes
 */
typedef enum {
    ENU_EM_SRV_OK = 0,              /**< Operation successful */
    ENU_EM_SRV_TIMEOUT,             /**< Operation timed out */
    ENU_EM_SRV_ERROR,               /**< General error */
    ENU_EM_SRV_BUSY,                /**< Service busy, try again */
    ENU_EM_SRV_NOT_AVAILABLE,       /**< Service not available */
    ENU_EM_SRV_CRC_ERROR,           /**< CRC validation failed */
    ENU_EM_SRV_INVALID_PARAM        /**< Invalid parameter */
} EnuEnergyMeterSrvStatus;

/**
 * @enum EnuEnergyMeterSrvState
 * @brief Service layer internal states
 */
typedef enum {
    ENU_EM_SRV_STATE_IDLE = 0,      /**< Service idle, ready for request */
    ENU_EM_SRV_STATE_BUSY,          /**< Service processing request */
    ENU_EM_SRV_STATE_WAIT_RESPONSE, /**< Waiting for response */
    ENU_EM_SRV_STATE_ERROR          /**< Service in error state */
} EnuEnergyMeterSrvState;

/**
 * @struct StEnergyMeterSrvRequest
 * @brief Service request structure
 */
typedef struct {
    u8 register_addr;               /**< Register address to read/write */
    u32 value;                      /**< Value to write (for write operations) */
    u32 timeout_ms;                 /**< Timeout in milliseconds */
    u8 is_write;                    /**< 1 for write, 0 for read */
    u8 retry_count;                 /**< Number of retries attempted */
} StEnergyMeterSrvRequest;

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief Initialize energy meter service layer
 *
 * Must be called once during system initialization before using service.
 * Initializes internal state and starts background state machine.
 */
void energy_meter_srv_init(void);

/**
 * @brief Process energy meter service (must be called periodically)
 *
 * This function drives the internal state machine and must be called
 * periodically (e.g., every 10-50ms) from main loop or RTOS task.
 */
void energy_meter_srv_process(void);

/**
 * @brief Check if energy meter service is available
 *
 * Returns service availability status. Application should check this
 * before making requests.
 *
 * @return 1 if service available, 0 if not available
 */
u8 energy_meter_srv_is_available(void);

/**
 * @brief Read register from STPM34 (blocking with timeout)
 *
 * Performs blocking read operation with timeout. Function waits until
 * data is received or timeout occurs.
 *
 * @param[in] addr Register address (0x00 - 0x32)
 * @param[out] value Pointer to store read value
 * @param[in] timeout_ms Timeout in milliseconds (0 = use default)
 *
 * @return EnuEnergyMeterSrvStatus status code
 *
 * @note This function blocks! Use in non-RTOS or from low-priority task
 * @note Due to STPM34 protocol, first read returns previous value
 */
EnuEnergyMeterSrvStatus energy_meter_srv_read_register_blocking(
    u8 addr,
    u32 *value,
    u32 timeout_ms
);

/**
 * @brief Write register to STPM34 (blocking with timeout)
 *
 * Performs blocking write operation with timeout.
 *
 * @param[in] addr Register address (0x00 - 0x1F)
 * @param[in] value 24-bit value to write
 * @param[in] timeout_ms Timeout in milliseconds (0 = use default)
 *
 * @return EnuEnergyMeterSrvStatus status code
 *
 * @note This function blocks! Use in non-RTOS or from low-priority task
 */
EnuEnergyMeterSrvStatus energy_meter_srv_write_register_blocking(
    u8 addr,
    u32 value,
    u32 timeout_ms
);

/**
 * @brief Initiate non-blocking read request
 *
 * Starts read operation and returns immediately. Application must poll
 * energy_meter_srv_get_result() to get the result.
 *
 * @param[in] addr Register address (0x00 - 0x32)
 * @param[in] timeout_ms Timeout in milliseconds (0 = use default)
 *
 * @return ENU_EM_SRV_OK if request accepted, error code otherwise
 *
 * @note Non-blocking - returns immediately
 * @note Only one request can be active at a time
 */
EnuEnergyMeterSrvStatus energy_meter_srv_read_register_async(
    u8 addr,
    u32 timeout_ms
);

/**
 * @brief Initiate non-blocking write request
 *
 * Starts write operation and returns immediately.
 *
 * @param[in] addr Register address (0x00 - 0x1F)
 * @param[in] value 24-bit value to write
 * @param[in] timeout_ms Timeout in milliseconds (0 = use default)
 *
 * @return ENU_EM_SRV_OK if request accepted, error code otherwise
 *
 * @note Non-blocking - returns immediately
 * @note Only one request can be active at a time
 */
EnuEnergyMeterSrvStatus energy_meter_srv_write_register_async(
    u8 addr,
    u32 value,
    u32 timeout_ms
);

/**
 * @brief Get result of async operation
 *
 * Retrieves result of previously initiated async operation.
 *
 * @param[out] value Pointer to store result (for read operations, can be NULL for write)
 *
 * @return EnuEnergyMeterSrvStatus:
 *         - ENU_EM_SRV_OK: Operation completed successfully
 *         - ENU_EM_SRV_BUSY: Still in progress
 *         - ENU_EM_SRV_TIMEOUT: Operation timed out
 *         - Other: Error occurred
 */
EnuEnergyMeterSrvStatus energy_meter_srv_get_result(u32 *value);

/**
 * @brief Get energy data (blocking with timeout)
 *
 * Reads multiple energy registers and returns consolidated data.
 * Performs multiple read operations internally.
 *
 * @param[out] energy_data Pointer to energy data structure
 * @param[in] timeout_ms Total timeout in milliseconds (0 = use default)
 *
 * @return EnuEnergyMeterSrvStatus status code
 *
 * @note This function blocks for multiple read operations
 * @note Timeout applies to entire operation, not per register
 */
EnuEnergyMeterSrvStatus energy_meter_srv_get_energy_data(
    StpmEnergyData *energy_data,
    u32 timeout_ms
);

/**
 * @brief Get service statistics
 *
 * Returns service layer statistics for monitoring and debugging.
 *
 * @param[out] total_requests Total number of requests (can be NULL)
 * @param[out] successful_requests Number of successful requests (can be NULL)
 * @param[out] failed_requests Number of failed requests (can be NULL)
 * @param[out] timeout_requests Number of timeout requests (can be NULL)
 */
void energy_meter_srv_get_statistics(
    u32 *total_requests,
    u32 *successful_requests,
    u32 *failed_requests,
    u32 *timeout_requests
);

/**
 * @brief Reset service statistics
 *
 * Resets all service statistics counters to zero.
 */
void energy_meter_srv_reset_statistics(void);

/**
 * @brief Cancel ongoing operation
 *
 * Cancels any ongoing async operation and returns service to idle state.
 */
void energy_meter_srv_cancel(void);

#endif /* BSW_SVC_COM_ENERGY_METER_SRV_ENERGY_METER_SRV_H_ */
