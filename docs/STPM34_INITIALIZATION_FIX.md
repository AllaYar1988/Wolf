# STPM34 Energy Meter Initialization Fix

## Problem Identified

The original implementation had a **critical flaw**: it attempted to read STPM34 data registers immediately without proper initialization.

### Issues Found:
1. ❌ **No chip reset** - STPM34 requires software reset via DSP_CR3 register
2. ❌ **No configuration** - Control registers must be written before reading data
3. ❌ **No data latching** - STPM34 requires explicit data latch command before reads
4. ❌ **Missing initialization states** - State machine jumped directly from INIT to READ

## Solution Implemented

Based on the sampleCode branch, a complete initialization sequence was added:

### Initialization Sequence:
```
1. ENU_EM_INIT
   └─> Initialize UART4, GPIO, DMA

2. ENU_EM_RESET_CHIP_TX / RX
   └─> Software reset via DSP_CR3 (SW_Reset bit)

3. ENU_EM_WRITE_CONFIG_TX / RX (Loop)
   └─> Write all configuration registers (0x00 to 0x29):
       - DSP_CR1-12: DSP control
       - DFE_CR1-2: Digital front end (gain settings)
       - DSP_IRQ1-2: Interrupt configuration
       - DSP_SR1-2: Status registers
       - US_REG1-3: UART/SPI settings (CRC, baud rate)

4. ENU_EM_LATCH_DATA_TX / RX
   └─> Latch data via DSP_CR3 (SW_Latch1, SW_Latch2 bits)

5. ENU_EM_SEND_READ_REQ / WAIT_FOR_RESPONSE
   └─> Now safe to read data registers (0x20 to 0x8B)
```

### Key Configuration Values:
- **DSP_CR3**: 0x04E00000 (SAG_TIME_THR = 0x4E0)
- **DFE_CR1/2**: 0x00000300 (Enable channels, Gain = 2)
- **US_REG1**: 0x00000700 (CRC enabled, polynomial 0x07)
- **US_REG2**: 0x00000683 (Baud rate for 9600)

## Files Modified

1. **SMU_Code/Core/ASW/energy_meters/energy_meters.h**
   - Added initialization states to EnuEnrgyMeterState enum
   - Added `energy_meters_is_initialized()` function prototype

2. **SMU_Code/Core/ASW/energy_meters/energy_meters.c**
   - Completely rewrote state machine handler
   - Added reset, configuration, and latch states
   - Added initialization status tracking
   - Fixed byte order in write function

3. **SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_hal.h**
   - Corrected register addresses (16-bit addressing)
   - Added DSP_CR3 control bit definitions
   - Added default configuration values
   - Added data register range definitions

## Usage

The state machine is now fully automatic:

```c
// In your main loop or periodic task (call every 10-50ms)
void periodic_task(void)
{
    energy_meters_handler();  // Handles init + continuous reading

    // Check if initialization complete
    if (energy_meters_is_initialized())
    {
        // Safe to use read data now
        u32 last_value = energy_meters_get_last_value();
    }
}
```

## Testing Recommendations

1. Monitor initialization sequence with debugger
2. Verify DSP_CR3 reset command succeeds
3. Check all 21 configuration registers are written
4. Confirm data latch before first read
5. Validate CRC on received data

## Reference

Implementation based on sampleCode branch:
- `/Core/STPM34/STPM34.c` - Complete initialization reference
- `/Core/Src/main.c` - Hardware setup and configuration examples
