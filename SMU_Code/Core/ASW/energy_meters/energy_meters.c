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

/** @brief Voltage calibration factor (multiplier for voltage readings) */
static float gVoltageCal = 1.0f;

/** @brief Current calibration factor (multiplier for current readings) */
static float gCurrentCal = 1.0f;

/** @brief Cached channel 1 active power (in Watts) */
static float gCh1ActivePower = 0.0f;

/** @brief Cached channel 1 reactive power (in VAR) */
static float gCh1ReactivePower = 0.0f;

/** @brief Cached channel 1 RMS voltage (in Volts) */
static float gCh1RmsVoltage = 0.0f;

/** @brief Cached channel 1 RMS current (in mA) */
static float gCh1RmsCurrent = 0.0f;

/** @brief Cached channel 2 active power (in Watts) */
static float gCh2ActivePower = 0.0f;

/** @brief Cached channel 2 reactive power (in VAR) */
static float gCh2ReactivePower = 0.0f;

/** @brief Cached channel 2 RMS voltage (in Volts) */
static float gCh2RmsVoltage = 0.0f;

/** @brief Cached channel 2 RMS current (in mA) */
static float gCh2RmsCurrent = 0.0f;

/* ========================================================================
 * Static Function Prototypes
 * ======================================================================== */

static u8 energy_meters_send_read_req(u8 addr);
static u8 energy_meters_send_write_req(u8 addr, u32 value);
static EnuEnergyMeterStatus energy_meters_process_response(void);
static u32 energy_meters_parse_response(u8 *rxBuf);
static int32_t energy_meters_convert_to_signed(u32 rawValue);
static float energy_meters_convert_voltage(u32 rawValue);
static float energy_meters_convert_current(u32 rawValue);
static float energy_meters_convert_power(u32 rawValue);
static void energy_meters_update_cached_values(u8 regAddr, u32 rawValue);

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

        /* Start with Channel 1 Active Power register */
        gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;

        state = ENU_EM_SEND_READ_REQ;
        break;

    case ENU_EM_SEND_READ_REQ:
        /* Send read request (chip select handled by DLL layer) */
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
                /* Valid response received - update cached values */
                energy_meters_update_cached_values(gCurrentRegister, gLastReadValue);

                /* Cycle through measurement registers:
                 * CH1: Active Power -> Reactive Power -> Voltage -> Current
                 * CH2: Active Power -> Reactive Power -> Voltage -> Current
                 */
                switch (gCurrentRegister)
                {
                    case STPM34_REG_CH1_ACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH1_REACTIVE_POWER;
                        break;
                    case STPM34_REG_CH1_REACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH1_VOLTAGE_RMS;
                        break;
                    case STPM34_REG_CH1_VOLTAGE_RMS:
                        gCurrentRegister = STPM34_REG_CH1_CURRENT_RMS;
                        break;
                    case STPM34_REG_CH1_CURRENT_RMS:
                        gCurrentRegister = STPM34_REG_CH2_ACTIVE_POWER;
                        break;
                    case STPM34_REG_CH2_ACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH2_REACTIVE_POWER;
                        break;
                    case STPM34_REG_CH2_REACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH2_VOLTAGE_RMS;
                        break;
                    case STPM34_REG_CH2_VOLTAGE_RMS:
                        gCurrentRegister = STPM34_REG_CH2_CURRENT_RMS;
                        break;
                    case STPM34_REG_CH2_CURRENT_RMS:
                        /* Wrap back to start */
                        gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;
                        break;
                    default:
                        /* Unknown register - reset to start */
                        gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;
                        break;
                }

                state = ENU_EM_SEND_READ_REQ;
            }
            else if (status == ENU_EM_STATUS_TIMEOUT)
            {
                /* Timeout occurred - move to next register */
                /* Use same cycling logic as success case */
                switch (gCurrentRegister)
                {
                    case STPM34_REG_CH1_ACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH1_REACTIVE_POWER;
                        break;
                    case STPM34_REG_CH1_REACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH1_VOLTAGE_RMS;
                        break;
                    case STPM34_REG_CH1_VOLTAGE_RMS:
                        gCurrentRegister = STPM34_REG_CH1_CURRENT_RMS;
                        break;
                    case STPM34_REG_CH1_CURRENT_RMS:
                        gCurrentRegister = STPM34_REG_CH2_ACTIVE_POWER;
                        break;
                    case STPM34_REG_CH2_ACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH2_REACTIVE_POWER;
                        break;
                    case STPM34_REG_CH2_REACTIVE_POWER:
                        gCurrentRegister = STPM34_REG_CH2_VOLTAGE_RMS;
                        break;
                    case STPM34_REG_CH2_VOLTAGE_RMS:
                        gCurrentRegister = STPM34_REG_CH2_CURRENT_RMS;
                        break;
                    case STPM34_REG_CH2_CURRENT_RMS:
                        /* Wrap back to start */
                        gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;
                        break;
                    default:
                        /* Unknown register - reset to start */
                        gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;
                        break;
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

/**
 * @brief Convert 24-bit unsigned value to signed integer
 *
 * STPM34 uses 24-bit two's complement representation for signed values.
 * This function converts from unsigned 24-bit to signed 32-bit.
 *
 * @param[in] rawValue Raw 24-bit value from STPM34 register
 * @return Signed 32-bit integer value
 */
static int32_t energy_meters_convert_to_signed(u32 rawValue)
{
    int32_t signedValue;

    /* Check if sign bit (bit 23) is set */
    if (rawValue & 0x800000)
    {
        /* Negative value - extend sign to 32 bits */
        signedValue = (int32_t)(rawValue | 0xFF000000);
    }
    else
    {
        /* Positive value */
        signedValue = (int32_t)rawValue;
    }

    return signedValue;
}

/**
 * @brief Convert raw voltage value to Volts
 *
 * Converts STPM34 raw voltage reading to calibrated voltage in Volts.
 *
 * @param[in] rawValue Raw 24-bit voltage value from STPM34
 * @return Voltage in Volts (V)
 */
static float energy_meters_convert_voltage(u32 rawValue)
{
    int32_t signedValue = energy_meters_convert_to_signed(rawValue);
    float voltage_mV = (float)signedValue * STPM34_VOLTAGE_LSB_MV;
    float voltage_V = (voltage_mV / 1000.0f) * gVoltageCal;

    return voltage_V;
}

/**
 * @brief Convert raw current value to milliamps
 *
 * Converts STPM34 raw current reading to calibrated current in mA.
 *
 * @param[in] rawValue Raw 24-bit current value from STPM34
 * @return Current in milliamps (mA)
 */
static float energy_meters_convert_current(u32 rawValue)
{
    int32_t signedValue = energy_meters_convert_to_signed(rawValue);
    float current_mA = (float)signedValue * STPM34_CURRENT_LSB_MA * gCurrentCal;

    return current_mA;
}

/**
 * @brief Convert raw power value to Watts
 *
 * Converts STPM34 raw power reading to power in Watts or VAR.
 * Works for both active and reactive power.
 *
 * @param[in] rawValue Raw 24-bit power value from STPM34
 * @return Power in Watts (W) or VAR
 */
static float energy_meters_convert_power(u32 rawValue)
{
    int32_t signedValue = energy_meters_convert_to_signed(rawValue);
    float power_mW = (float)signedValue * STPM34_POWER_LSB_MW;
    float power_W = (power_mW / 1000.0f) * gVoltageCal * gCurrentCal;

    return power_W;
}

/**
 * @brief Update cached measurement values based on register address
 *
 * This function is called when a successful register read occurs.
 * It converts the raw value to engineering units and stores it in
 * the appropriate cached variable for later retrieval.
 *
 * @param[in] regAddr Register address that was read
 * @param[in] rawValue Raw 24-bit value from the register
 */
static void energy_meters_update_cached_values(u8 regAddr, u32 rawValue)
{
    switch (regAddr)
    {
        case STPM34_REG_CH1_ACTIVE_POWER:
            gCh1ActivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_CH1_REACTIVE_POWER:
            gCh1ReactivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_CH1_VOLTAGE_RMS:
            gCh1RmsVoltage = energy_meters_convert_voltage(rawValue);
            break;

        case STPM34_REG_CH1_CURRENT_RMS:
            gCh1RmsCurrent = energy_meters_convert_current(rawValue);
            break;

        case STPM34_REG_CH2_ACTIVE_POWER:
            gCh2ActivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_CH2_REACTIVE_POWER:
            gCh2ReactivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_CH2_VOLTAGE_RMS:
            gCh2RmsVoltage = energy_meters_convert_voltage(rawValue);
            break;

        case STPM34_REG_CH2_CURRENT_RMS:
            gCh2RmsCurrent = energy_meters_convert_current(rawValue);
            break;

        default:
            /* Unknown register - do nothing */
            break;
    }
}

/**
 * @brief Set calibration factors for voltage and current measurements
 *
 * Calibration factors are multipliers applied to all measurements to
 * compensate for sensor inaccuracies, resistor tolerances, etc.
 *
 * @param[in] voltage_cal Voltage calibration factor (typically 0.9 - 1.1)
 * @param[in] current_cal Current calibration factor (typically 0.9 - 1.1)
 *
 * @note Default values are 1.0 (no calibration)
 * @note Power calibration is automatically voltage_cal * current_cal
 */
void energy_meters_set_calibration(float voltage_cal, float current_cal)
{
    gVoltageCal = voltage_cal;
    gCurrentCal = current_cal;
}

/**
 * @brief Read active power from specified channel
 *
 * Returns the most recently measured active power value for the
 * specified channel. Values are automatically updated by the
 * energy_meters_handler() state machine.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return Active power in Watts (W), or 0.0 if channel invalid
 *
 * @note Ensure energy_meters_handler() is called periodically to update values
 */
float energy_meters_read_active_power(u8 channel)
{
    if (channel == 1) {
        return gCh1ActivePower;
    } else if (channel == 2) {
        return gCh2ActivePower;
    }
    return 0.0f;
}

/**
 * @brief Read reactive power from specified channel
 *
 * Returns the most recently measured reactive power value for the
 * specified channel. Values are automatically updated by the
 * energy_meters_handler() state machine.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return Reactive power in VAR, or 0.0 if channel invalid
 *
 * @note Ensure energy_meters_handler() is called periodically to update values
 */
float energy_meters_read_reactive_power(u8 channel)
{
    if (channel == 1) {
        return gCh1ReactivePower;
    } else if (channel == 2) {
        return gCh2ReactivePower;
    }
    return 0.0f;
}

/**
 * @brief Read RMS current from specified channel
 *
 * Returns the most recently measured RMS current value for the
 * specified channel. Values are automatically updated by the
 * energy_meters_handler() state machine.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return RMS current in milliamps (mA), or 0.0 if channel invalid
 *
 * @note Ensure energy_meters_handler() is called periodically to update values
 */
float energy_meters_read_rms_current(u8 channel)
{
    if (channel == 1) {
        return gCh1RmsCurrent;
    } else if (channel == 2) {
        return gCh2RmsCurrent;
    }
    return 0.0f;
}

/**
 * @brief Read RMS voltage from specified channel
 *
 * Returns the most recently measured RMS voltage value for the
 * specified channel. Values are automatically updated by the
 * energy_meters_handler() state machine.
 *
 * @param[in] channel Channel number (1 or 2)
 * @return RMS voltage in Volts (V), or 0.0 if channel invalid
 *
 * @note Ensure energy_meters_handler() is called periodically to update values
 */
float energy_meters_read_rms_voltage(u8 channel)
{
    if (channel == 1) {
        return gCh1RmsVoltage;
    } else if (channel == 2) {
        return gCh2RmsVoltage;
    }
    return 0.0f;
}
