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
