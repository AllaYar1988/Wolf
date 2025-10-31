/**
 * @file energy_meters.c
 * @brief STPM34 Energy Meter Application Layer
 *
 * Implements high-level energy meter functions including reading registers,
 * processing energy data, and managing communication state machine.
 *
 * @date Created on: Oct 16, 2025
 * @author A. Moazami
 */

#include "energy_meters.h"
#include "crc.h"
#include "energy_meter_hal.h"
#include "energy_meter_dll.h"
#include <string.h>

/* ========================================================================
 * Private Variables
 * ======================================================================== */

/** @brief Timeout counter for energy meter operations */
static u32 gEnergyMeterTimeout = 0;

/** @brief Last received data from STPM34 */
static u32 gLastReadValue = 0;

/** @brief Current register address being read */
static u8 gCurrentRegister = 0;

/** @brief Timeout occurrence counter (for diagnostics) */
static u32 gTimeoutCount = 0;

/** @brief CRC error counter (for diagnostics) */
static u32 gCrcErrorCount = 0;

/** @brief Successful transaction counter (for diagnostics) */
static u32 gSuccessCount = 0;

/* ========================================================================
 * Static Function Prototypes
 * ======================================================================== */

static u8 energy_meters_send_read_req(u8 addr);
static u8 energy_meters_send_write_req(u8 addr, u32 value);
static EnuEnergyMeterStatus energy_meters_process_response(void);
static u32 energy_meters_parse_response(u8 *rxBuf);

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Main energy meter handler state machine
 *
 * Implements state machine for periodic energy meter communication.
 * Call this function periodically (e.g., every 10ms).
 */
void energy_meters_handler(void)
{
    static EnuEnrgyMeterState state = ENU_EM_INIT;

    switch(state)
    {
    case ENU_EM_INIT:
        /* Initialize hardware */
        energy_meters_hal_init();

        /* Initialize DMA reception */
        energy_meter_dll_receive_init();

        /* Start with register 0x05 (DSP_CR5) */
        gCurrentRegister = STPM34_REG_DSP_CR5;

        state = ENU_EM_SEND_READ_REQ;
        break;

    case ENU_EM_SEND_READ_REQ:
        /* Send read request (chip select handled by DLL layer) */
    	gCurrentRegister = 0x05;
        energy_meters_send_read_req(gCurrentRegister);

        /* Reset timeout counter */
        gEnergyMeterTimeout = 0;

        state = ENU_EM_WAIT_FOR_RESPONSE;
        break;

    case ENU_EM_WAIT_FOR_RESPONSE:
        /* Process response from STPM34 */
        {
            EnuEnergyMeterStatus status = energy_meters_process_response();

            if (status == ENU_EM_STATUS_SUCCESS)
            {
                /* Valid response received */
               // energy_meter_dll_transaction_end();

                /* Cycle through different registers */
                gCurrentRegister++;
                if (gCurrentRegister > STPM34_REG_DSP_CR11) {
                    gCurrentRegister = STPM34_REG_DSP_CR5;
                }

                state = ENU_EM_SEND_READ_REQ;
            }
            else if (status == ENU_EM_STATUS_TIMEOUT)
            {
                /* Timeout occurred - end transaction and move to next register */
               // energy_meter_dll_transaction_end();

                /* Cycle through different registers */
                gCurrentRegister++;
                if (gCurrentRegister > STPM34_REG_DSP_CR11) {
                    gCurrentRegister = STPM34_REG_DSP_CR5;
                }

                state = ENU_EM_SEND_READ_REQ;
            }
            else if (status == ENU_EM_STATUS_CRC_ERROR)
            {
                /* CRC error - increment timeout and keep waiting */
                /* Transaction continues, might get valid data next time */
            }
            /* else ENU_EM_STATUS_IDLE - continue waiting */
        }
        break;

    case ENU_EM_STOP:
        /* Idle state - do nothing */
        break;
    }
}

/**
 * @brief Read STPM34 register value
 *
 * Sends a read command to the specified STPM34 register. Due to STPM34's
 * protocol, the value returned is from the PREVIOUS transaction.
 *
 * @param[in] addr Register address to read
 * @param[out] value Pointer to store read value from previous transaction (can be NULL)
 * @return 1 if successful, 0 if failed
 *
 * @note This is a blocking function that manages chip select automatically
 * @note For periodic register reading, use energy_meters_handler() state machine instead
 */
u8 energy_meters_read_register(u8 addr, u32 *value)
{
    u8 txBuf[STPM34_FRAME_SIZE];
    u8 returnValue = 0;

    /* Build read command frame */
    txBuf[0] = addr | STPM34_READ_BIT;  /* Address with read bit set */
    txBuf[1] = 0xFF;  /* Dummy bytes for response */
    txBuf[2] = 0xFF;
    txBuf[3] = 0xFF;

    /* Calculate CRC for the frame */
    txBuf[4] = crc_stpm3x(txBuf, 4);

    /* Send frame via DLL (chip select handled by DLL) */
    energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

    /* Note: Should wait for response and call energy_meter_dll_transaction_end() */
    /* For now, transaction end should be called by the caller after waiting */
    /* Typically this function is not used - the state machine handles communication */

    /* Value will be available in next transaction */
    if (value != NULL) {
        *value = gLastReadValue;
        returnValue = 1;
    }

    return returnValue;
}

/**
 * @brief Write value to STPM34 register
 *
 * @param[in] addr Register address to write
 * @param[in] value Value to write (24-bit)
 * @return 1 if successful, 0 if failed
 */
u8 energy_meters_write_register(u8 addr, u32 value)
{
    return energy_meters_send_write_req(addr, value);
}

/**
 * @brief Get last read value from STPM34
 *
 * @return Last value read from STPM34
 */
u32 energy_meters_get_last_value(void)
{
    return gLastReadValue;
}

/**
 * @brief Get diagnostic statistics for energy meter communication
 *
 * Retrieves counters for successful transactions, timeouts, and CRC errors.
 * These statistics help monitor communication health and identify issues.
 *
 * @param[out] success_count Pointer to store successful transaction count (can be NULL)
 * @param[out] timeout_count Pointer to store timeout occurrence count (can be NULL)
 * @param[out] crc_error_count Pointer to store CRC error count (can be NULL)
 */
void energy_meters_get_statistics(u32 *success_count, u32 *timeout_count, u32 *crc_error_count)
{
    if (success_count != NULL) {
        *success_count = gSuccessCount;
    }

    if (timeout_count != NULL) {
        *timeout_count = gTimeoutCount;
    }

    if (crc_error_count != NULL) {
        *crc_error_count = gCrcErrorCount;
    }
}

/**
 * @brief Reset diagnostic statistics counters
 *
 * Resets all diagnostic counters (success, timeout, CRC errors) to zero.
 * Useful for periodic statistics collection or after maintenance operations.
 */
void energy_meters_reset_statistics(void)
{
    gSuccessCount = 0;
    gTimeoutCount = 0;
    gCrcErrorCount = 0;
}

/* ========================================================================
 * Static Function Implementations
 * ======================================================================== */

/**
 * @brief Send read request to STPM34
 *
 * Builds read command frame and sends via DLL layer.
 * Chip select is handled by DLL transaction functions.
 *
 * @param[in] addr Register address to read
 * @return Always returns 1
 */
static u8 energy_meters_send_read_req(u8 addr)
{
    u8 txBuf[STPM34_FRAME_SIZE];

    /* Build read command frame */
    txBuf[0] = addr | STPM34_READ_BIT;  /* Address with read bit set */
    txBuf[1] = 0xFF;  /* Dummy bytes */
    txBuf[2] = 0xFF;
    txBuf[3] = 0xFF;

    /* Calculate and add CRC */
    txBuf[4] = crc_stpm3x(txBuf, 4);

    /* Send via DLL (chip select handled by DLL) */
    energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

    return 1;
}

/**
 * @brief Send write request to STPM34
 *
 * Builds write command frame and sends via DLL layer.
 * Chip select is handled by DLL transaction functions.
 *
 * @param[in] addr Register address to write
 * @param[in] value 24-bit value to write
 * @return Always returns 1
 *
 * @note Caller should call energy_meter_dll_transaction_end() after write completes
 */
static u8 energy_meters_send_write_req(u8 addr, u32 value)
{
    u8 txBuf[STPM34_FRAME_SIZE];

    /* Build write command frame */
    txBuf[0] = addr & (~STPM34_READ_BIT);  /* Address with read bit clear */
    txBuf[1] = (value >> 16) & 0xFF;       /* MSB */
    txBuf[2] = (value >> 8) & 0xFF;        /* Mid byte */
    txBuf[3] = value & 0xFF;                /* LSB */

    /* Calculate and add CRC */
    txBuf[4] = crc_stpm3x(txBuf, 4);

    /* Send via DLL (chip select handled by DLL) */
    energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

    return 1;
}

/**
 * @brief Process response from STPM34
 *
 * Checks for received data via DLL, validates CRC, and extracts value.
 * Implements proper timeout mechanism with diagnostic counters.
 *
 * @return EnuEnergyMeterStatus indicating the result:
 *         - ENU_EM_STATUS_SUCCESS: Valid response received
 *         - ENU_EM_STATUS_TIMEOUT: Timeout occurred
 *         - ENU_EM_STATUS_CRC_ERROR: Data received but CRC invalid
 *         - ENU_EM_STATUS_IDLE: Still waiting for response
 */
static EnuEnergyMeterStatus energy_meters_process_response(void)
{
    EnuEnergyMeterStatus status = ENU_EM_STATUS_IDLE;
    u16 bytesReceived;

    /* Check for received data */
    bytesReceived = energy_meter_dll_receive();

    if (bytesReceived >= STPM34_FRAME_SIZE)
    {
        u8 *rxBuf = energy_meter_dll_get_rx_buffer();

        /* Verify CRC */
        u8 receivedCrc = rxBuf[4];
        u8 calculatedCrc = crc_stpm3x(rxBuf, 4);

        if (receivedCrc == calculatedCrc)
        {
            /* Valid response received - parse and store the 24-bit value */
            gLastReadValue = energy_meters_parse_response(rxBuf);

            /* Reset timeout counter */
            gEnergyMeterTimeout = 0;

            /* Increment success counter */
            gSuccessCount++;

            status = ENU_EM_STATUS_SUCCESS;
        }
        else
        {
            /* CRC validation failed */
            gCrcErrorCount++;
            status = ENU_EM_STATUS_CRC_ERROR;

            /* Continue incrementing timeout for CRC errors */
            gEnergyMeterTimeout++;
        }
    }
    else
    {
        /* No data received yet - increment timeout counter */
        gEnergyMeterTimeout++;
    }

    /* Check if timeout reached */
    if (gEnergyMeterTimeout > ENERGY_METER_TIMEOUT)
    {
        /* Timeout occurred */
        gEnergyMeterTimeout = 0;
        gTimeoutCount++;
        status = ENU_EM_STATUS_TIMEOUT;
    }

    return status;
}

/**
 * @brief Parse STPM34 response frame
 *
 * Extracts 24-bit data value from response frame.
 *
 * @param[in] rxBuf Pointer to received buffer (must be at least 5 bytes)
 * @return Extracted 24-bit value as 32-bit integer
 */
static u32 energy_meters_parse_response(u8 *rxBuf)
{
    u32 value;

    /* Extract 24-bit value from bytes 1-3 */
    value = ((u32)rxBuf[1] << 16) | ((u32)rxBuf[2] << 8) | rxBuf[3];

    return value;
}
