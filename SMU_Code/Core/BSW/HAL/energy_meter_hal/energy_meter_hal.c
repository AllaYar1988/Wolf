/**
 * @file energy_meter_hal.c
 * @brief STPM34 Energy Meter HAL layer implementation
 *
 * This module provides hardware abstraction for the STPM34 energy metering IC.
 * It serves as a convenience wrapper around the GPIO and UART initialization
 * functions provided by gpio.c and usart.c.
 *
 * @date Created on: Oct 16, 2025
 * @author A. Moazami
 */

#include "energy_meter_hal.h"
#include "gpio.h"
#include "usart.h"

/* ========================================================================
 * Public Function Implementations
 * ======================================================================== */

/**
 * @brief Initialize energy meter hardware
 *
 * This function initializes the GPIO pin for chip select control and
 * the UART4 peripheral for communication with the STPM34 energy meter.
 * It calls the public initialization functions from gpio.c and usart.c.
 *
 * @note This function must be called before any energy meter operations.
 */
void energy_meters_hal_init(void)
{
    /* Initialize GPIO for chip select (PB1) */
    MX_EnergyMeter_GPIO_Init();

    /* Initialize UART4 for STPM34 communication (9600 baud, 8N1) */
    MX_UART4_UART_Init();
}
