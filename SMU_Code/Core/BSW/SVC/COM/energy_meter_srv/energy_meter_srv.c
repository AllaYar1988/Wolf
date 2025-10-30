/**
 * @file energy_meter_srv.c
 * @brief Energy Meter Service Layer Implementation
 *
 * Implements service layer functions for STPM34 energy meter operations.
 * Provides blocking/non-blocking API with timeout handling and service availability.
 *
 * ARCHITECTURE:
 * -------------
 * Application → Service Layer (THIS) → Application/State Machine → DLL → HAL
 *
 * @date Created on: Oct 30, 2025
 * @author A. Moazami
 */

#include "energy_meter_srv.h"
#include "energy_meters.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/* ========================================================================
 * Private Variables
 * ======================================================================== */

/** @brief Current service state */
static EnuEnergyMeterSrvState gServiceState = ENU_EM_SRV_STATE_IDLE;

/** @brief Current service request */
static StEnergyMeterSrvRequest gCurrentRequest;

/** @brief Response value storage */
static u32 gResponseValue = 0;

/** @brief Service operation result */
static EnuEnergyMeterSrvStatus gLastResult = ENU_EM_SRV_OK;

/** @brief Timeout start timestamp */
static u32 gTimeoutStartTick = 0;

/** @brief Service statistics counters */
static u32 gTotalRequests = 0;
static u32 gSuccessfulRequests = 0;
static u32 gFailedRequests = 0;
static u32 gTimeoutRequests = 0;

/** @brief Service initialization flag */
static u8 gServiceInitialized = 0;

/** @brief Number of consecutive failures (for availability check) */
static u32 gConsecutiveFailures = 0;

#define MAX_CONSECUTIVE_FAILURES  5  /**< Service marked unavailable after this many failures */

/* ========================================================================
 * Private Function Prototypes
 * ======================================================================== */

static EnuEnergyMeterSrvStatus energy_meter_srv_wait_completion(u32 timeout_ms);
static u8 energy_meter_srv_check_timeout(u32 timeout_ms);

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Initialize energy meter service layer
 *
 * Must be called once during system initialization before using service.
 * Initializes internal state and starts background state machine.
 */
void energy_meter_srv_init(void)
{
    /* Reset service state */
    gServiceState = ENU_EM_SRV_STATE_IDLE;
    gServiceInitialized = 1;

    /* Clear statistics */
    gTotalRequests = 0;
    gSuccessfulRequests = 0;
    gFailedRequests = 0;
    gTimeoutRequests = 0;
    gConsecutiveFailures = 0;

    /* Clear request and response */
    memset(&gCurrentRequest, 0, sizeof(StEnergyMeterSrvRequest));
    gResponseValue = 0;
    gLastResult = ENU_EM_SRV_OK;
}

/**
 * @brief Process energy meter service (must be called periodically)
 *
 * This function drives the internal state machine and must be called
 * periodically (e.g., every 10-50ms) from main loop or RTOS task.
 */
void energy_meter_srv_process(void)
{
    u32 success_count, timeout_count, crc_error_count;

    /* Get statistics from lower layer to check communication health */
    energy_meters_get_statistics(&success_count, &timeout_count, &crc_error_count);

    /* Update consecutive failure counter based on communication statistics */
    /* If we have recent failures, track them for availability */
    if ((timeout_count + crc_error_count) > 0 && success_count == 0) {
        gConsecutiveFailures++;
    } else if (success_count > 0) {
        /* Reset on successful communication */
        gConsecutiveFailures = 0;
    }

    /* Service state machine is driven by blocking/async functions */
    /* This function mainly monitors communication health */
}

/**
 * @brief Check if energy meter service is available
 *
 * Returns service availability status. Application should check this
 * before making requests.
 *
 * @return 1 if service available, 0 if not available
 */
u8 energy_meter_srv_is_available(void)
{
    /* Service not available if not initialized */
    if (!gServiceInitialized) {
        return 0;
    }

    /* Service not available if too many consecutive failures */
    if (gConsecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
        return 0;
    }

    /* Service not available if currently in error state */
    if (gServiceState == ENU_EM_SRV_STATE_ERROR) {
        return 0;
    }

    return 1;
}

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
)
{
    EnuEnergyMeterSrvStatus status;

    /* Validate parameters */
    if (value == NULL) {
        return ENU_EM_SRV_INVALID_PARAM;
    }

    /* Check service availability */
    if (!energy_meter_srv_is_available()) {
        return ENU_EM_SRV_NOT_AVAILABLE;
    }

    /* Check if service is busy */
    if (gServiceState != ENU_EM_SRV_STATE_IDLE) {
        return ENU_EM_SRV_BUSY;
    }

    /* Use default timeout if not specified */
    if (timeout_ms == 0) {
        timeout_ms = ENERGY_METER_SRV_DEFAULT_TIMEOUT_MS;
    }

    /* Increment total requests */
    gTotalRequests++;

    /* Setup request */
    gCurrentRequest.register_addr = addr;
    gCurrentRequest.timeout_ms = timeout_ms;
    gCurrentRequest.is_write = 0;
    gCurrentRequest.retry_count = 0;

    /* Change state to busy */
    gServiceState = ENU_EM_SRV_STATE_BUSY;

    /* Initiate read request to lower layer */
    energy_meters_read_register(addr, NULL);

    /* Wait for completion with timeout */
    status = energy_meter_srv_wait_completion(timeout_ms);

    /* Store result value */
    if (status == ENU_EM_SRV_OK) {
        *value = gResponseValue;
        gSuccessfulRequests++;
        gConsecutiveFailures = 0;
    } else if (status == ENU_EM_SRV_TIMEOUT) {
        gTimeoutRequests++;
        gFailedRequests++;
        gConsecutiveFailures++;
    } else {
        gFailedRequests++;
        gConsecutiveFailures++;
    }

    /* Return to idle state */
    gServiceState = ENU_EM_SRV_STATE_IDLE;

    return status;
}

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
)
{
    EnuEnergyMeterSrvStatus status;

    /* Check service availability */
    if (!energy_meter_srv_is_available()) {
        return ENU_EM_SRV_NOT_AVAILABLE;
    }

    /* Check if service is busy */
    if (gServiceState != ENU_EM_SRV_STATE_IDLE) {
        return ENU_EM_SRV_BUSY;
    }

    /* Use default timeout if not specified */
    if (timeout_ms == 0) {
        timeout_ms = ENERGY_METER_SRV_DEFAULT_TIMEOUT_MS;
    }

    /* Increment total requests */
    gTotalRequests++;

    /* Setup request */
    gCurrentRequest.register_addr = addr;
    gCurrentRequest.value = value;
    gCurrentRequest.timeout_ms = timeout_ms;
    gCurrentRequest.is_write = 1;
    gCurrentRequest.retry_count = 0;

    /* Change state to busy */
    gServiceState = ENU_EM_SRV_STATE_BUSY;

    /* Initiate write request to lower layer */
    energy_meters_write_register(addr, value);

    /* Wait for completion with timeout */
    status = energy_meter_srv_wait_completion(timeout_ms);

    /* Update statistics */
    if (status == ENU_EM_SRV_OK) {
        gSuccessfulRequests++;
        gConsecutiveFailures = 0;
    } else if (status == ENU_EM_SRV_TIMEOUT) {
        gTimeoutRequests++;
        gFailedRequests++;
        gConsecutiveFailures++;
    } else {
        gFailedRequests++;
        gConsecutiveFailures++;
    }

    /* Return to idle state */
    gServiceState = ENU_EM_SRV_STATE_IDLE;

    return status;
}

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
)
{
    /* Check service availability */
    if (!energy_meter_srv_is_available()) {
        return ENU_EM_SRV_NOT_AVAILABLE;
    }

    /* Check if service is busy */
    if (gServiceState != ENU_EM_SRV_STATE_IDLE) {
        return ENU_EM_SRV_BUSY;
    }

    /* Use default timeout if not specified */
    if (timeout_ms == 0) {
        timeout_ms = ENERGY_METER_SRV_DEFAULT_TIMEOUT_MS;
    }

    /* Increment total requests */
    gTotalRequests++;

    /* Setup request */
    gCurrentRequest.register_addr = addr;
    gCurrentRequest.timeout_ms = timeout_ms;
    gCurrentRequest.is_write = 0;
    gCurrentRequest.retry_count = 0;

    /* Start timeout tracking */
    gTimeoutStartTick = HAL_GetTick();

    /* Change state to wait response */
    gServiceState = ENU_EM_SRV_STATE_WAIT_RESPONSE;

    /* Initiate read request to lower layer */
    energy_meters_read_register(addr, NULL);

    /* Set last result to busy */
    gLastResult = ENU_EM_SRV_BUSY;

    return ENU_EM_SRV_OK;
}

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
)
{
    /* Check service availability */
    if (!energy_meter_srv_is_available()) {
        return ENU_EM_SRV_NOT_AVAILABLE;
    }

    /* Check if service is busy */
    if (gServiceState != ENU_EM_SRV_STATE_IDLE) {
        return ENU_EM_SRV_BUSY;
    }

    /* Use default timeout if not specified */
    if (timeout_ms == 0) {
        timeout_ms = ENERGY_METER_SRV_DEFAULT_TIMEOUT_MS;
    }

    /* Increment total requests */
    gTotalRequests++;

    /* Setup request */
    gCurrentRequest.register_addr = addr;
    gCurrentRequest.value = value;
    gCurrentRequest.timeout_ms = timeout_ms;
    gCurrentRequest.is_write = 1;
    gCurrentRequest.retry_count = 0;

    /* Start timeout tracking */
    gTimeoutStartTick = HAL_GetTick();

    /* Change state to wait response */
    gServiceState = ENU_EM_SRV_STATE_WAIT_RESPONSE;

    /* Initiate write request to lower layer */
    energy_meters_write_register(addr, value);

    /* Set last result to busy */
    gLastResult = ENU_EM_SRV_BUSY;

    return ENU_EM_SRV_OK;
}

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
EnuEnergyMeterSrvStatus energy_meter_srv_get_result(u32 *value)
{
    u32 success_count, timeout_count, crc_error_count;
    u32 prev_success_count;
    static u32 last_success_count = 0;

    /* If service is idle, no async operation in progress */
    if (gServiceState == ENU_EM_SRV_STATE_IDLE) {
        return gLastResult;
    }

    /* Check for timeout */
    if (energy_meter_srv_check_timeout(gCurrentRequest.timeout_ms)) {
        /* Timeout occurred */
        gServiceState = ENU_EM_SRV_STATE_IDLE;
        gLastResult = ENU_EM_SRV_TIMEOUT;
        gTimeoutRequests++;
        gFailedRequests++;
        gConsecutiveFailures++;
        return ENU_EM_SRV_TIMEOUT;
    }

    /* Get statistics from lower layer */
    energy_meters_get_statistics(&success_count, &timeout_count, &crc_error_count);

    /* Check if we have new successful data */
    if (success_count > last_success_count) {
        /* New data available - read it */
        gResponseValue = energy_meters_get_last_value();

        /* Store result */
        if (value != NULL) {
            *value = gResponseValue;
        }

        /* Update statistics */
        gSuccessfulRequests++;
        gConsecutiveFailures = 0;
        last_success_count = success_count;

        /* Return to idle */
        gServiceState = ENU_EM_SRV_STATE_IDLE;
        gLastResult = ENU_EM_SRV_OK;

        return ENU_EM_SRV_OK;
    }

    /* Still waiting for response */
    return ENU_EM_SRV_BUSY;
}

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
)
{
    EnuEnergyMeterSrvStatus status;
    u32 value;
    u32 per_register_timeout;

    /* Validate parameters */
    if (energy_data == NULL) {
        return ENU_EM_SRV_INVALID_PARAM;
    }

    /* Check service availability */
    if (!energy_meter_srv_is_available()) {
        return ENU_EM_SRV_NOT_AVAILABLE;
    }

    /* Use default timeout if not specified */
    if (timeout_ms == 0) {
        timeout_ms = ENERGY_METER_SRV_DEFAULT_TIMEOUT_MS;
    }

    /* Divide timeout among 6 registers */
    per_register_timeout = timeout_ms / 6;
    if (per_register_timeout < 100) {
        per_register_timeout = 100;  /* Minimum 100ms per register */
    }

    /* Read Channel 1 Active Energy */
    status = energy_meter_srv_read_register_blocking(
        STPM34_REG_CH1_ACTIVE_ENERGY, &value, per_register_timeout);
    if (status != ENU_EM_SRV_OK) {
        return status;
    }
    energy_data->ch1_active_energy = (int32_t)value;

    /* Read Channel 1 Reactive Energy */
    status = energy_meter_srv_read_register_blocking(
        STPM34_REG_CH1_REACTIVE_ENERGY, &value, per_register_timeout);
    if (status != ENU_EM_SRV_OK) {
        return status;
    }
    energy_data->ch1_reactive_energy = (int32_t)value;

    /* Read Channel 1 Apparent Energy */
    status = energy_meter_srv_read_register_blocking(
        STPM34_REG_CH1_APPARENT_ENERGY, &value, per_register_timeout);
    if (status != ENU_EM_SRV_OK) {
        return status;
    }
    energy_data->ch1_apparent_energy = (int32_t)value;

    /* Read Channel 2 Active Energy */
    status = energy_meter_srv_read_register_blocking(
        STPM34_REG_CH2_ACTIVE_ENERGY, &value, per_register_timeout);
    if (status != ENU_EM_SRV_OK) {
        return status;
    }
    energy_data->ch2_active_energy = (int32_t)value;

    /* Read Channel 2 Reactive Energy */
    status = energy_meter_srv_read_register_blocking(
        STPM34_REG_CH2_REACTIVE_ENERGY, &value, per_register_timeout);
    if (status != ENU_EM_SRV_OK) {
        return status;
    }
    energy_data->ch2_reactive_energy = (int32_t)value;

    /* Read Channel 2 Apparent Energy */
    status = energy_meter_srv_read_register_blocking(
        STPM34_REG_CH2_APPARENT_ENERGY, &value, per_register_timeout);
    if (status != ENU_EM_SRV_OK) {
        return status;
    }
    energy_data->ch2_apparent_energy = (int32_t)value;

    return ENU_EM_SRV_OK;
}

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
)
{
    if (total_requests != NULL) {
        *total_requests = gTotalRequests;
    }

    if (successful_requests != NULL) {
        *successful_requests = gSuccessfulRequests;
    }

    if (failed_requests != NULL) {
        *failed_requests = gFailedRequests;
    }

    if (timeout_requests != NULL) {
        *timeout_requests = gTimeoutRequests;
    }
}

/**
 * @brief Reset service statistics
 *
 * Resets all service statistics counters to zero.
 */
void energy_meter_srv_reset_statistics(void)
{
    gTotalRequests = 0;
    gSuccessfulRequests = 0;
    gFailedRequests = 0;
    gTimeoutRequests = 0;
}

/**
 * @brief Cancel ongoing operation
 *
 * Cancels any ongoing async operation and returns service to idle state.
 */
void energy_meter_srv_cancel(void)
{
    /* Return to idle state */
    gServiceState = ENU_EM_SRV_STATE_IDLE;
    gLastResult = ENU_EM_SRV_ERROR;
}

/* ========================================================================
 * Private Function Implementations
 * ======================================================================== */

/**
 * @brief Wait for operation completion with timeout
 *
 * Polling loop that waits for lower layer to complete operation.
 * Uses application layer statistics to detect completion.
 *
 * @param[in] timeout_ms Timeout in milliseconds
 * @return EnuEnergyMeterSrvStatus result code
 */
static EnuEnergyMeterSrvStatus energy_meter_srv_wait_completion(u32 timeout_ms)
{
    u32 start_tick;
    u32 success_count, timeout_count, crc_error_count;
    u32 initial_success_count;

    /* Get initial statistics */
    energy_meters_get_statistics(&initial_success_count, &timeout_count, &crc_error_count);

    /* Start timeout tracking */
    start_tick = HAL_GetTick();

    /* Wait for completion or timeout */
    while (1)
    {
        /* Check for timeout */
        if ((HAL_GetTick() - start_tick) >= timeout_ms) {
            return ENU_EM_SRV_TIMEOUT;
        }

        /* Get current statistics */
        energy_meters_get_statistics(&success_count, &timeout_count, &crc_error_count);

        /* Check if we have new successful data */
        if (success_count > initial_success_count) {
            /* New data available - read it */
            gResponseValue = energy_meters_get_last_value();
            return ENU_EM_SRV_OK;
        }

        /* Small delay to prevent CPU hogging (adjust based on your RTOS or system) */
        /* In bare metal, this could be a simple delay */
        /* In RTOS, this could be osDelay() */
        HAL_Delay(1);  /* 1ms delay */
    }
}

/**
 * @brief Check if timeout has occurred for async operation
 *
 * @param[in] timeout_ms Timeout value in milliseconds
 * @return 1 if timeout occurred, 0 otherwise
 */
static u8 energy_meter_srv_check_timeout(u32 timeout_ms)
{
    u32 elapsed_ms;

    /* Calculate elapsed time */
    elapsed_ms = HAL_GetTick() - gTimeoutStartTick;

    /* Check if timeout reached */
    if (elapsed_ms >= timeout_ms) {
        return 1;
    }

    return 0;
}
