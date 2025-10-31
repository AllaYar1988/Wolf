/**
 * @file energy_meter_hal.h
 * @brief STPM34 Energy Meter HAL layer interface
 *
 * This module provides hardware abstraction for the STPM34 energy metering IC.
 * Communication is via UART4 with DMA support.
 *
 * CURRENT DESIGN: Single energy meter instance
 * FUTURE EXTENSION: For multiple energy meters, use the following pattern:
 *
 * @code
 * // Define energy meter device structure
 * typedef struct {
 *     GPIO_TypeDef* cs_port;          // Chip select port
 *     uint16_t cs_pin;                 // Chip select pin
 *     UART_HandleTypeDef* uart;        // UART handle
 *     uint8_t device_id;               // Device identifier
 * } EnergyMeterDevice_t;
 *
 * // Create instances for each meter
 * EnergyMeterDevice_t meter1 = {GPIOB, GPIO_PIN_1, &huart4, 0};
 * EnergyMeterDevice_t meter2 = {GPIOB, GPIO_PIN_2, &huart5, 1};
 *
 * // Modify DLL functions to accept device pointer
 * void energy_meter_dll_transaction_send(EnergyMeterDevice_t* dev, u8* msg, u16 size);
 * void energy_meter_dll_transaction_end(EnergyMeterDevice_t* dev);
 * @endcode
 *
 * @date Created on: Oct 16, 2025
 * @author A. Moazami
 */

#ifndef BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_HAL_H_
#define BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_HAL_H_

#include "stm32f4xx_hal.h"
#include "platform.h"
#include "usart.h"

/* ========================================================================
 * GPIO Definitions - Energy Meter Chip Select
 * ======================================================================== */

/** @brief STPM34 Chip Select GPIO Pin */
#define ENERGY_METER_SCS_Pin        GPIO_PIN_1

/** @brief STPM34 Chip Select GPIO Port */
#define ENERGY_METER_SCS_GPIO_Port  GPIOB

/* Compatibility macros with existing code */
#define uGenerator_STPM_SCS_Pin       ENERGY_METER_SCS_Pin
#define uGenerator_STPM_SCS_GPIO_Port ENERGY_METER_SCS_GPIO_Port

/* ========================================================================
 * Chip Select Control Macros
 * ======================================================================== */

/** @brief Select energy meter (assert chip select LOW - active) */
#define ENERGY_METER_CS_SELECT()   \
    HAL_GPIO_WritePin(ENERGY_METER_SCS_GPIO_Port, ENERGY_METER_SCS_Pin, GPIO_PIN_RESET)

/** @brief Deselect energy meter (de-assert chip select HIGH - inactive) */
#define ENERGY_METER_CS_DESELECT() \
    HAL_GPIO_WritePin(ENERGY_METER_SCS_GPIO_Port, ENERGY_METER_SCS_Pin, GPIO_PIN_SET)

/* ========================================================================
 * UART Definitions - Energy Meter Communication
 * ======================================================================== */

/** @brief Energy meter UART interface (maps to huart4 from usart.h) */
#define ENERGY_METER_UART huart4

/* ========================================================================
 * STPM34 Register Addresses
 * ======================================================================== */

/** @defgroup STPM34_Registers STPM34 Register Map
 * @{
 */
/* Configuration Registers (Write during initialization) */
#define STPM34_REG_DSP_CR1          0x00    /**< DSP Control Register 1 (Channel 1 config) */
#define STPM34_REG_DSP_CR2          0x02    /**< DSP Control Register 2 (Channel 2 config) */
#define STPM34_REG_DSP_CR3          0x04    /**< DSP Control Register 3 (Reset, Latch, Frequency) */
#define STPM34_REG_DSP_CR4          0x06    /**< DSP Control Register 4 (Phase calibration) */
#define STPM34_REG_DSP_CR5          0x08    /**< DSP Control Register 5 (CH1 V calibration) */
#define STPM34_REG_DSP_CR6          0x0A    /**< DSP Control Register 6 (CH1 I calibration) */
#define STPM34_REG_DSP_CR7          0x0C    /**< DSP Control Register 7 (CH2 V calibration) */
#define STPM34_REG_DSP_CR8          0x0E    /**< DSP Control Register 8 (CH2 I calibration) */
#define STPM34_REG_DSP_CR9          0x10    /**< DSP Control Register 9 (CH1 power offset) */
#define STPM34_REG_DSP_CR10         0x12    /**< DSP Control Register 10 (CH1 energy offset) */
#define STPM34_REG_DSP_CR11         0x14    /**< DSP Control Register 11 (CH2 power offset) */
#define STPM34_REG_DSP_CR12         0x16    /**< DSP Control Register 12 (CH2 energy offset) */
#define STPM34_REG_DFE_CR1          0x18    /**< Digital Front End Control 1 (CH1 gain) */
#define STPM34_REG_DFE_CR2          0x1A    /**< Digital Front End Control 2 (CH2 gain) */
#define STPM34_REG_DSP_IRQ1         0x1C    /**< Interrupt config 1 */
#define STPM34_REG_DSP_IRQ2         0x1E    /**< Interrupt config 2 */
#define STPM34_REG_DSP_SR1          0x20    /**< Status register 1 */
#define STPM34_REG_DSP_SR2          0x22    /**< Status register 2 */
#define STPM34_REG_US_REG1          0x24    /**< UART/SPI config 1 */
#define STPM34_REG_US_REG2          0x26    /**< UART/SPI config 2 */
#define STPM34_REG_US_REG3          0x28    /**< UART/SPI config 3 */

#define STPM34_CONFIG_REGS_END      0x29    /**< Last configuration register address */

/* Data Registers (Read after latching) */
#define STPM34_DATA_REGS_START      0x20    /**< First data register */
#define STPM34_REG_DSP_REG1         0x24    /**< DSP Data Register 1 */
#define STPM34_REG_DSP_REG2         0x26    /**< DSP Data Register 2 */
#define STPM34_REG_DSP_REG3         0x28    /**< DSP Data Register 3 */
#define STPM34_REG_DSP_REG4         0x2A    /**< DSP Data Register 4 */
#define STPM34_REG_DSP_REG5         0x2C    /**< DSP Data Register 5 */
#define STPM34_REG_DSP_REG6         0x2E    /**< DSP Data Register 6 */
#define STPM34_REG_DSP_REG7         0x30    /**< DSP Data Register 7 */
#define STPM34_REG_DSP_REG8         0x32    /**< DSP Data Register 8 */
#define STPM34_REG_DSP_REG9         0x34    /**< DSP Data Register 9 (Line period) */
#define STPM34_REG_DSP_REG10        0x36    /**< DSP Data Register 10 (Momentary V/I) */
#define STPM34_REG_DSP_REG11        0x38    /**< DSP Data Register 11 */
#define STPM34_REG_DSP_REG12        0x3A    /**< DSP Data Register 12 */
#define STPM34_REG_DSP_REG13        0x3C    /**< DSP Data Register 13 */
#define STPM34_REG_DSP_REG14        0x3E    /**< DSP Data Register 14 */
#define STPM34_REG_DSP_REG15        0x40    /**< DSP Data Register 15 (RMS V/I CH1) */
#define STPM34_DATA_REGS_END        0x8B    /**< Last data register */

/* Energy Registers */
#define STPM34_REG_CH1_ACTIVE_ENERGY    0x04    /**< Channel 1 Active Energy */
#define STPM34_REG_CH1_REACTIVE_ENERGY  0x05    /**< Channel 1 Reactive Energy */
#define STPM34_REG_CH1_APPARENT_ENERGY  0x06    /**< Channel 1 Apparent Energy */
#define STPM34_REG_CH2_ACTIVE_ENERGY    0x07    /**< Channel 2 Active Energy */
#define STPM34_REG_CH2_REACTIVE_ENERGY  0x08    /**< Channel 2 Reactive Energy */
#define STPM34_REG_CH2_APPARENT_ENERGY  0x09    /**< Channel 2 Apparent Energy */

/* Instantaneous Power Registers */
#define STPM34_REG_CH1_ACTIVE_POWER     0x28    /**< Channel 1 Active Power */
#define STPM34_REG_CH1_REACTIVE_POWER   0x29    /**< Channel 1 Reactive Power */
#define STPM34_REG_CH1_APPARENT_RMS     0x2A    /**< Channel 1 Apparent RMS Power */
#define STPM34_REG_CH2_ACTIVE_POWER     0x2B    /**< Channel 2 Active Power */
#define STPM34_REG_CH2_REACTIVE_POWER   0x2C    /**< Channel 2 Reactive Power */
#define STPM34_REG_CH2_APPARENT_RMS     0x2D    /**< Channel 2 Apparent RMS Power */

/* RMS Voltage and Current Registers */
#define STPM34_REG_CH1_VOLTAGE_RMS      0x28    /**< Channel 1 RMS Voltage */
#define STPM34_REG_CH1_CURRENT_RMS      0x29    /**< Channel 1 RMS Current */
#define STPM34_REG_CH2_VOLTAGE_RMS      0x2A    /**< Channel 2 RMS Voltage */
#define STPM34_REG_CH2_CURRENT_RMS      0x2B    /**< Channel 2 RMS Current */

/** @} */

/* ========================================================================
 * STPM34 Command Bits
 * ======================================================================== */

/** @brief Read command bit (bit 7 = 1 for read) */
#define STPM34_READ_BIT             0x80

/** @brief Write command bit (bit 7 = 0 for write) */
#define STPM34_WRITE_BIT            0x00

/* ========================================================================
 * STPM34 DSP_CR3 Control Bits (for reset and latch operations)
 * ======================================================================== */

/** @brief Software reset bit in DSP_CR3 register (bit 14) */
#define STPM34_DSP_CR3_SW_RESET     (1 << 14)

/** @brief Software latch 1 bit in DSP_CR3 register (bit 15) */
#define STPM34_DSP_CR3_SW_LATCH1    (1 << 15)

/** @brief Software latch 2 bit in DSP_CR3 register (bit 16) */
#define STPM34_DSP_CR3_SW_LATCH2    (1 << 16)

/** @brief 60Hz frequency selection bit in DSP_CR3 register (bit 27) */
#define STPM34_DSP_CR3_REFFREQ_60HZ (1 << 27)

/* Default configuration values for STPM34 */
#define STPM34_DSP_CR3_DEFAULT      0x04E00000  /**< Default DSP_CR3: SAG_TIME_THR=0x4E0 */
#define STPM34_US_REG1_DEFAULT      0x00000700  /**< CRC enabled, polynomial 0x07 */
#define STPM34_US_REG2_DEFAULT      0x00000683  /**< Baud rate for 9600 */

/* ========================================================================
 * Data Structures
 * ======================================================================== */

/**
 * @struct StpmEnergyData
 * @brief Structure to hold STPM34 energy measurement data
 */
typedef struct {
	int32_t ch1_active_energy; /**< Channel 1 active energy (Wh) */
	int32_t ch1_reactive_energy; /**< Channel 1 reactive energy (VARh) */
	int32_t ch1_apparent_energy; /**< Channel 1 apparent energy (VAh) */
	int32_t ch2_active_energy; /**< Channel 2 active energy (Wh) */
	int32_t ch2_reactive_energy; /**< Channel 2 reactive energy (VARh) */
	int32_t ch2_apparent_energy; /**< Channel 2 apparent energy (VAh) */
} StpmEnergyData;

/**
 * @struct StpmPowerData
 * @brief Structure to hold STPM34 instantaneous power data
 */
typedef struct {
	int32_t ch1_active_power; /**< Channel 1 active power (W) */
	int32_t ch1_reactive_power; /**< Channel 1 reactive power (VAR) */
	int32_t ch1_apparent_power; /**< Channel 1 apparent power (VA) */
	int32_t ch2_active_power; /**< Channel 2 active power (W) */
	int32_t ch2_reactive_power; /**< Channel 2 reactive power (VAR) */
	int32_t ch2_apparent_power; /**< Channel 2 apparent power (VA) */
} StpmPowerData;

/**
 * @struct StpmRmsData
 * @brief Structure to hold STPM34 RMS voltage and current data
 */
typedef struct {
	uint32_t ch1_voltage_rms; /**< Channel 1 RMS voltage (mV) */
	uint32_t ch1_current_rms; /**< Channel 1 RMS current (mA) */
	uint32_t ch2_voltage_rms; /**< Channel 2 RMS voltage (mV) */
	uint32_t ch2_current_rms; /**< Channel 2 RMS current (mA) */
} StpmRmsData;

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief Initialize energy meter hardware (calls GPIO and UART init)
 *
 * This is a convenience function that calls the appropriate initialization
 * functions from gpio.c and usart.c
 */
void energy_meters_hal_init(void);

#endif /* BSW_HAL_ENERGY_METER_HAL_ENERGY_METER_HAL_H_ */
