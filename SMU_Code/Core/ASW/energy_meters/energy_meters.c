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

/** @brief Current register address being read/written */
static u8 gCurrentRegister = 0;

/** @brief Previous register address (for delayed response protocol) */
static u8 gPreviousRegister = 0xFF;

/** @brief Flag indicating if we should store response data (delayed response handling) */
static u8 gStoreResponseData = 0;

/** @brief Timeout occurrence counter (for diagnostics) */
static u32 gTimeoutCount = 0;

/** @brief Configuration complete flag */
static u8 gConfigComplete = 0;

/** @brief Chip initialized flag */
static u8 gChipInitialized = 0;

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
 * Implements complete STPM34 initialization and communication state machine.
 * Must be called periodically (e.g., every 10-50ms).
 *
 * Initialization sequence:
 * 1. Hardware init (UART, GPIO)
 * 2. Software reset chip
 * 3. Write all configuration registers
 * 4. Latch data
 * 5. Read data registers continuously
 */
void energy_meters_handler(void) {
	static EnuEnrgyMeterState state = ENU_EM_INIT;
	EnuEnergyMeterStatus status;

	switch (state) {
	case ENU_EM_INIT:
		/* Initialize hardware (UART, GPIO, DMA) */
		energy_meters_hal_init();
		energy_meter_dll_receive_init();

		/* Start initialization sequence with chip reset */
		state = ENU_EM_RESET_CHIP_TX;
		break;

	case ENU_EM_RESET_CHIP_TX:
		/* Send software reset command via DSP_CR3 register */
	{
		u32 dsp_cr3_value = STPM34_DSP_CR3_DEFAULT |
		STPM34_DSP_CR3_SW_RESET |
		STPM34_DSP_CR3_SW_LATCH1 |
		STPM34_DSP_CR3_SW_LATCH2;

		/* Note: sampleCode reads 0x05 while writing DSP_CR3 during reset */
		u8 txBuf[STPM34_FRAME_SIZE];
		txBuf[0] = 0x05; /* Read address 0x05 */
		txBuf[1] = STPM34_REG_DSP_CR3; /* Write DSP_CR3 */
		txBuf[2] = dsp_cr3_value & 0xFF; /* Data LOW */
		txBuf[3] = (dsp_cr3_value >> 8) & 0xFF; /* Data HIGH */
		txBuf[4] = crc_stpm3x(txBuf, 4); /* CRC */
		energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

		gEnergyMeterTimeout = 0;
		state = ENU_EM_RESET_CHIP_RX;
	}
		break;

	case ENU_EM_RESET_CHIP_RX:
		/* Wait for reset confirmation */
		status = energy_meters_process_response();

		if (status == ENU_EM_STATUS_SUCCESS
				|| status == ENU_EM_STATUS_TIMEOUT) {
			/* End transaction - deselect chip (CS goes HIGH) */
			energy_meter_dll_transaction_end();

			/* Reset complete - start writing configuration registers */
			gCurrentRegister = 0x00; /* Start from DSP_CR1 */
			state = ENU_EM_WRITE_CONFIG_TX;
		}
		break;

	case ENU_EM_WRITE_CONFIG_TX:
		/* Write configuration registers sequentially */
	{
		u32 reg_value = 0; /* Default value */

		/* Set specific register values based on address */
		switch (gCurrentRegister) {
		case STPM34_REG_DSP_CR3:
			reg_value = STPM34_DSP_CR3_DEFAULT; /* No reset/latch bits */
			break;
		case STPM34_REG_DFE_CR1:
			reg_value = 0x00000300; /* Enable CH1, Gain=2 */
			break;
		case STPM34_REG_DFE_CR2:
			reg_value = 0x00000300; /* Enable CH2, Gain=2 */
			break;
		case STPM34_REG_US_REG1:
			reg_value = STPM34_US_REG1_DEFAULT; /* CRC enabled */
			break;
		case STPM34_REG_US_REG2:
			reg_value = STPM34_US_REG2_DEFAULT; /* Baud rate */
			break;
		default:
			reg_value = 0; /* Other registers use default 0 */
			break;
		}

		/* sampleCode reads 0x01 while writing config registers */
		u8 txBuf[STPM34_FRAME_SIZE];
		txBuf[0] = 0x01; /* Read address 0x01 */
		txBuf[1] = gCurrentRegister; /* Write target register */
		txBuf[2] = reg_value & 0xFF; /* Data LOW */
		txBuf[3] = (reg_value >> 8) & 0xFF; /* Data HIGH */
		txBuf[4] = crc_stpm3x(txBuf, 4); /* CRC */
		energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

		gEnergyMeterTimeout = 0;
		state = ENU_EM_WRITE_CONFIG_RX;
	}
		break;

	case ENU_EM_WRITE_CONFIG_RX:
		/* Wait for write confirmation */
		status = energy_meters_process_response();

		if (status == ENU_EM_STATUS_SUCCESS
				|| status == ENU_EM_STATUS_TIMEOUT) {
			/* End transaction - deselect chip (CS goes HIGH) */
			energy_meter_dll_transaction_end();

			/* Move to next configuration register */
			gCurrentRegister += 2; /* STPM34 uses 16-bit addressing */

			if (gCurrentRegister > STPM34_CONFIG_REGS_END) {
				/* All config registers written - proceed to data latching */
				gConfigComplete = 1;
				state = ENU_EM_LATCH_DATA_TX;
			} else {
				/* Write next register */
				state = ENU_EM_WRITE_CONFIG_TX;
			}
		}
		break;

	case ENU_EM_LATCH_DATA_TX:
		/* Send latch command to freeze data registers for reading */
	{
		u32 dsp_cr3_value = STPM34_DSP_CR3_DEFAULT |
		STPM34_DSP_CR3_SW_LATCH1 |
		STPM34_DSP_CR3_SW_LATCH2;

		/* sampleCode uses 0xFF (no read) for latch operations */
		u8 txBuf[STPM34_FRAME_SIZE];
		txBuf[0] = 0xFF; /* No read */
		txBuf[1] = STPM34_REG_DSP_CR3; /* Write DSP_CR3 */
		txBuf[2] = dsp_cr3_value & 0xFF; /* Data LOW */
		txBuf[3] = (dsp_cr3_value >> 8) & 0xFF; /* Data HIGH */
		txBuf[4] = crc_stpm3x(txBuf, 4); /* CRC */
		energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

		gEnergyMeterTimeout = 0;
		state = ENU_EM_LATCH_DATA_RX;
	}
		break;

	case ENU_EM_LATCH_DATA_RX:
		/* Wait for latch confirmation */
		status = energy_meters_process_response();

		if (status == ENU_EM_STATUS_SUCCESS
				|| status == ENU_EM_STATUS_TIMEOUT) {
			/* End transaction - deselect chip (CS goes HIGH) */
			energy_meter_dll_transaction_end();

			/* Data latched - now we can start reading */
			gCurrentRegister = STPM34_DATA_REGS_START;
			gPreviousRegister = 0xFF; /* Reset previous register tracking */
			gStoreResponseData = 0; /* First read is dummy (delayed response) */
			gChipInitialized = 1;
			state = ENU_EM_SEND_READ_REQ;
		}
		break;

	case ENU_EM_SEND_READ_REQ:
		/* Send read request for data register */
		energy_meters_send_read_req(gCurrentRegister);
		gEnergyMeterTimeout = 0;
		state = ENU_EM_WAIT_FOR_RESPONSE;
		break;

	case ENU_EM_WAIT_FOR_RESPONSE:
		/* Process response from STPM34 */
		status = energy_meters_process_response();

		if (status == ENU_EM_STATUS_SUCCESS) {
			/* End transaction - deselect chip (CS goes HIGH) */
			energy_meter_dll_transaction_end();

            /* NOTE: Due to STPM34 delayed response protocol, the received data
             * is from the PREVIOUS read command, not the current one.
             * First read after latch returns dummy/garbage data. */

			if (gStoreResponseData
					&& gPreviousRegister >= STPM34_DATA_REGS_START) {
				/* Store response to PREVIOUS register address */
				/* TODO: Add data storage array when needed (currently only tracking last value) */
				/* regData[gPreviousRegister] = gLastReadValue; */
			}

			/* Update tracking for next iteration */
			gPreviousRegister = gCurrentRegister;
			gStoreResponseData = 1; /* After first read, start storing data */

			/* Move to next register */
			gCurrentRegister += 2; /* Increment by 2 (16-bit addressing) */

			if (gCurrentRegister > STPM34_DATA_REGS_END) {
				/* All registers read - latch new data and restart */
				state = ENU_EM_LATCH_DATA_TX;
			} else {
				/* Read next register */
				state = ENU_EM_SEND_READ_REQ;
			}
		} else if (status == ENU_EM_STATUS_TIMEOUT) {
			/* End transaction on timeout - deselect chip (CS goes HIGH) */
			energy_meter_dll_transaction_end();

			/* Update tracking even on timeout */
			gPreviousRegister = gCurrentRegister;
			gStoreResponseData = 1;

            /* Timeout - skip this register and continue */
            gCurrentRegister += 2;

            if (gCurrentRegister > STPM34_DATA_REGS_END)
            {
                state = ENU_EM_LATCH_DATA_TX;
            }
            else
            {



                state = ENU_EM_SEND_READ_REQ;

            }
        }
        /* else ENU_EM_STATUS_IDLE or CRC_ERROR - keep waiting */
        break;

	case ENU_EM_IDLE:
		/* Idle state - wait for next cycle */
		break;

	case ENU_EM_STOP:
		/* Error/stop state - do nothing */
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
u8 energy_meters_read_register(u8 addr, u32 *value) {
	u8 txBuf[STPM34_FRAME_SIZE];
	u8 returnValue = 0;

	/* Build read command frame */
	txBuf[0] = addr | STPM34_READ_BIT; /* Address with read bit set */
	txBuf[1] = 0xFF; /* Dummy bytes for response */
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
u8 energy_meters_write_register(u8 addr, u32 value) {
	return energy_meters_send_write_req(addr, value);
}

/**
 * @brief Get last read value from STPM34
 *
 * @return Last value read from STPM34
 */
u32 energy_meters_get_last_value(void) {
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
void energy_meters_get_statistics(u32 *success_count, u32 *timeout_count,
		u32 *crc_error_count) {
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
void energy_meters_reset_statistics(void) {
	gSuccessCount = 0;
	gTimeoutCount = 0;
	gCrcErrorCount = 0;
}

/**
 * @brief Check if STPM34 initialization is complete
 *
 * Returns the initialization status. The chip must complete initialization
 * (reset, configuration, and first data latch) before valid data can be read.
 *
 * @return 1 if initialization complete and chip is ready, 0 otherwise
 */
u8 energy_meters_is_initialized(void) {
	return gChipInitialized;
}

/* ========================================================================
 * Static Function Implementations
 * ======================================================================== */

/**
 * @brief Send read request to STPM34
 *
 * Builds read command frame and sends via DLL layer.
 * STPM34 protocol uses TWO addresses per frame: read addr and write addr.
 *
 * @param[in] addr Register address to read
 * @return Always returns 1
 *
 * @note Frame format: [ReadAddr][WriteAddr=0xFF][DataLow=0xFF][DataHigh=0xFF][CRC]
 */
static u8 energy_meters_send_read_req(u8 addr) {
	u8 txBuf[STPM34_FRAME_SIZE];

	/* Build read command frame - STPM34 two-address format */
	txBuf[0] = addr; /* Read address */
	txBuf[1] = 0xFF; /* Write address (0xFF = no write) */
	txBuf[2] = 0xFF; /* Write data LOW byte (dummy) */
	txBuf[3] = 0xFF; /* Write data HIGH byte (dummy) */

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
 * STPM34 protocol uses TWO addresses per frame: read addr and write addr.
 *
 * @param[in] addr Register address to write
 * @param[in] value 16-bit value to write
 * @return Always returns 1
 *
 * @note Frame format: [ReadAddr=0xFF][WriteAddr][DataLow][DataHigh][CRC]
 * @note Caller should call energy_meter_dll_transaction_end() after write completes
 */
static u8 energy_meters_send_write_req(u8 addr, u32 value) {
	u8 txBuf[STPM34_FRAME_SIZE];

	/* Build write command frame - STPM34 two-address format */
	txBuf[0] = 0xFF; /* Read address (0xFF = no read) */
	txBuf[1] = addr; /* Write address */
	txBuf[2] = value & 0xFF; /* Write data LOW byte */
	txBuf[3] = (value >> 8) & 0xFF; /* Write data HIGH byte */

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
static EnuEnergyMeterStatus energy_meters_process_response(void) {
	EnuEnergyMeterStatus status = ENU_EM_STATUS_IDLE;
	u16 bytesReceived;

	/* Check for received data */
	bytesReceived = energy_meter_dll_receive();

	if (bytesReceived >= STPM34_FRAME_SIZE) {
		u8 *rxBuf = energy_meter_dll_get_rx_buffer();

		/* Verify CRC */
		u8 receivedCrc = rxBuf[4];
		u8 calculatedCrc = crc_stpm3x(rxBuf, 4);

		if (receivedCrc == calculatedCrc) {
			/* Valid response received - parse and store the 24-bit value */
			gLastReadValue = energy_meters_parse_response(rxBuf);

			/* Reset timeout counter */
			gEnergyMeterTimeout = 0;

			/* Increment success counter */
			gSuccessCount++;

			status = ENU_EM_STATUS_SUCCESS;
		} else {
			/* CRC validation failed */
			gCrcErrorCount++;
			status = ENU_EM_STATUS_CRC_ERROR;

			/* Continue incrementing timeout for CRC errors */
			gEnergyMeterTimeout++;
		}
	} else {
		/* No data received yet - increment timeout counter */
		gEnergyMeterTimeout++;
	}

	/* Check if timeout reached */
	if (gEnergyMeterTimeout > ENERGY_METER_TIMEOUT) {
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
 * Extracts 16-bit data value from response frame.
 * Response format: [Word1_Low][Word1_High][Word2_Low][Word2_High][CRC]
 *
 * @param[in] rxBuf Pointer to received buffer (must be at least 5 bytes)
 * @return Extracted 16-bit value from first word (little endian)
 */
static u32 energy_meters_parse_response(u8 *rxBuf) {
	u32 value;

	/* Extract first 16-bit word (little endian: LOW byte, HIGH byte) */
	value = ((u32) rxBuf[0]) | ((u32) rxBuf[1] << 8);

	return value;
}
