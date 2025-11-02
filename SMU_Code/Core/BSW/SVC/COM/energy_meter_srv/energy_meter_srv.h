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


#endif /* BSW_SVC_COM_ENERGY_METER_SRV_ENERGY_METER_SRV_H_ */
