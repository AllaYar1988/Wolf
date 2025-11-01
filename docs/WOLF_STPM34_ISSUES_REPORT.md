# Wolf STPM34 Driver - Issues Report & Fixes

**Date:** 2025-11-01
**Analysis:** Comparison with ST Official STPM Library
**Status:** Critical Issues Identified - Requires Immediate Fixes

---

## Executive Summary

The Wolf STPM34 driver contains **8 significant issues** when compared with ST Microelectronics' official STPM library. The most critical issues involve:

1. **Only reading 16 bits instead of 32 bits** from registers (causing completely wrong measurements)
2. **Only writing 16 bits instead of 32 bits** to registers (causing incomplete configuration)
3. **Incorrect register addresses** (reading from wrong memory locations)
4. **Missing bit-packed RMS value extraction** (V/I RMS packed in same register)

**Bottom Line:** The driver will NOT work correctly until at least the 4 critical issues (Priority 1) are fixed.

---

## Issue #1: Only Reading 16 Bits Instead of 32 Bits ⚠️ CRITICAL

### Severity: **CRITICAL**

### Location
- **File:** `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/energy_meters.c`
- **Line:** 637 (function `energy_meters_parse_response`)

### Description

The STPM34 returns 32-bit register values in its response frame (4 data bytes + 1 CRC byte = 5 total bytes). Wolf's code only parses the first 16 bits (2 bytes), losing the upper 16 bits entirely.

**Response Frame Format:**
```
[Byte0][Byte1][Byte2][Byte3][CRC]
  ↑      ↑      ↑      ↑     ↑
 Bit0-7 Bit8-15 Bit16-23 Bit24-31 CRC-8

Data is 32 bits (4 bytes), little-endian
```

### Wolf's Buggy Code

```c
// Lines 633-640 - energy_meters.c
static u32 energy_meters_parse_response(u8 *rxBuf) {
    u32 value;

    /* Extract first 16-bit word (little endian: LOW byte, HIGH byte) */
    value = ((u32) rxBuf[0]) | ((u32) rxBuf[1] << 8);  // ❌ WRONG!

    return value;
}
```

**This only reads `rxBuf[0]` and `rxBuf[1]`, ignoring `rxBuf[2]` and `rxBuf[3]`!**

### ST Official Approach

```c
// From metrology_hal.c - Metro_HAL_Stpm_Read()
/* Now Retreive RX data (one frame of 4 bytes : one U32): memcpy from RX buffer */
memcpy(p_read_data+(k*4), p_Metro_Device_Config[in_Metro_Device_Id].STPM_com.pRxReadBuf, 4);
```

ST copies **4 bytes** of data, not 2.

### Impact

- ✅ All power measurements are **completely wrong**
- ✅ All energy readings are **truncated to 16 bits** (wrapping at 65536)
- ✅ All voltage readings are **missing upper 16 bits**
- ✅ All current readings are **only partial values**
- ✅ System shows incorrect values, making power management decisions based on garbage data

**Example:**
- Actual power register value: `0x00012C40` (76,864 in decimal)
- Wolf reads: `0x2C40` (11,328 in decimal) ❌ **85% error!**

### Recommended Fix

```c
/**
 * @brief Parse STPM34 response frame (FIXED)
 *
 * Extracts 32-bit data value from response frame.
 * Response format: [Byte0][Byte1][Byte2][Byte3][CRC]
 * Data is little-endian (LSB first).
 *
 * @param[in] rxBuf Pointer to received buffer (must be at least 5 bytes)
 * @return Extracted 32-bit value from data bytes
 */
static u32 energy_meters_parse_response(u8 *rxBuf) {
    u32 value;

    /* Extract full 32-bit word from 4 data bytes
     * STPM34 response frame: [Byte0][Byte1][Byte2][Byte3][CRC]
     * Data is little-endian: LSB first, MSB last
     */
    value = ((u32)rxBuf[0]) |        // Bits 0-7 (LSB)
            ((u32)rxBuf[1] << 8) |   // Bits 8-15
            ((u32)rxBuf[2] << 16) |  // Bits 16-23
            ((u32)rxBuf[3] << 24);   // Bits 24-31 (MSB)

    return value;
}
```

### Testing After Fix

```c
// Test with known register read
uint32_t test_value;
energy_meters_read_register(STPM34_REG_DSP_CR3, &test_value);

// DSP_CR3 default value should be 0x04E00000
// If you get 0x0000, the old code is still running
// If you get 0x04E00000, the fix works!
printf("DSP_CR3 = 0x%08X (expected: 0x04E00000)\n", test_value);
```

---

## Issue #2: Only Writing 16 Bits Instead of 32 Bits ⚠️ CRITICAL

### Severity: **CRITICAL**

### Location
- **File:** `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/energy_meters.c`
- **Lines:** 142-143, 198-199, 242-243 (reset, config, latch operations)

### Description

STPM34 registers are 32-bit but are accessed as **two consecutive 16-bit halves**. Each 32-bit write requires **TWO frames**: one for the lower 16 bits (address N), one for the upper 16 bits (address N+1).

Wolf sends only ONE frame per register write, leaving the upper 16 bits unwritten (defaulting to 0 or previous value).

### Wolf's Buggy Code

```c
// Lines 131-145 - energy_meters.c (RESET_CHIP_TX state)
{
    u32 dsp_cr3_value = STPM34_DSP_CR3_DEFAULT |
                        STPM34_DSP_CR3_SW_RESET |
                        STPM34_DSP_CR3_SW_LATCH1 |
                        STPM34_DSP_CR3_SW_LATCH2;  // Value: 0x04E1C000

    u8 txBuf[STPM34_FRAME_SIZE];
    txBuf[0] = 0x05;
    txBuf[1] = STPM34_REG_DSP_CR3;  // Address: 0x04
    txBuf[2] = dsp_cr3_value & 0xFF;        // Only bits 0-7: 0x00  ❌
    txBuf[3] = (dsp_cr3_value >> 8) & 0xFF; // Only bits 8-15: 0xC0 ❌
    txBuf[4] = crc_stpm3x(txBuf, 4);

    // ❌ Missing second frame for bits 16-31!
}
```

**This writes:**
- Address 0x04: `0xC000` (bits 0-15) ✅
- Address 0x05: **NOT WRITTEN** ❌ (should be `0x04E1` for bits 16-31)

**Result:** DSP_CR3 becomes `0x????C000` instead of `0x04E1C000` → Reset/latch bits may not work!

### ST Official Approach

```c
// From metrology_hal.c - Metro_HAL_Stpm_write()

/* One block is an U32 but STPM need to write in two times ( 2 U16 )*/
/* Nb blocks * 2 to multiply by 2 the write requests */
if (nb_blocks==0) {
    nb_blocks_tmp = 1;
} else {
    nb_blocks_tmp = nb_blocks*2;  // *** Multiply by 2 ***
}

for (k=0; k<nb_blocks_tmp; k++) {
    frame_with_CRC[0] = 0xff;              /* No read requested */
    frame_with_CRC[1] = (*in_p_data) + k;  /* Write Address (incremented for each U16) */
    frame_with_CRC[2] = *(p_writePointer); /* DATA LSB */
    frame_with_CRC[3] = *(++p_writePointer); /* DATA MSB */

    /* Increment Pointer to next U16 data for the next loop */
    p_writePointer++;

    // Send frame, wait for response, repeat for second half...
}
```

ST writes **TWO frames** per 32-bit register:
1. Frame 1: Address N → lower 16 bits
2. Frame 2: Address N+1 → upper 16 bits

### Impact

- ✅ **Chip reset may not work** (SW_RESET bit is in upper half at bit 14)
- ✅ **Data latch may not work** (SW_LATCH1/LATCH2 bits are bits 15-16)
- ✅ **All configuration registers are half-written** (gain, calibration, thresholds)
- ✅ **Chip is in undefined/incorrect state**

**Example:**
- Trying to write DSP_CR3 = `0x04E1C000`
- Wolf writes: `0x????C000` (upper half missing) ❌
- Reset bit (bit 14 = `0x4000`) is in the **upper half** → never gets written!

### Recommended Fix

```c
/**
 * @brief Write 32-bit value to STPM34 register (FIXED)
 *
 * STPM34 requires TWO 16-bit writes per 32-bit register.
 * Address N receives bits 0-15, address N+1 receives bits 16-31.
 *
 * @param[in] addr Register address (base address for lower 16 bits)
 * @param[in] value 32-bit value to write
 * @return 1 if successful, 0 if failed
 */
static u8 energy_meters_write_register_32bit(u8 addr, u32 value) {
    u8 txBuf[STPM34_FRAME_SIZE];
    EnuEnergyMeterStatus status;

    /* ========== FRAME 1: Write lower 16 bits to address N ========== */
    txBuf[0] = 0xFF;   /* No read */
    txBuf[1] = addr;   /* Write address (LSB half) */
    txBuf[2] = (value) & 0xFF;        /* Data bits 0-7 */
    txBuf[3] = (value >> 8) & 0xFF;   /* Data bits 8-15 */
    txBuf[4] = crc_stpm3x(txBuf, 4);  /* CRC */

    energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

    /* Wait for response */
    gEnergyMeterTimeout = 0;
    do {
        status = energy_meters_process_response();
        gEnergyMeterTimeout++;
        if (gEnergyMeterTimeout > ENERGY_METER_TIMEOUT) {
            energy_meter_dll_transaction_end();
            return 0; /* Timeout */
        }
    } while (status == ENU_EM_STATUS_IDLE);

    energy_meter_dll_transaction_end();

    /* Small delay between frames */
    for (volatile int i = 0; i < 1000; i++);

    /* ========== FRAME 2: Write upper 16 bits to address N+1 ========== */
    txBuf[0] = 0xFF;       /* No read */
    txBuf[1] = addr + 1;   /* Write address+1 (MSB half) */
    txBuf[2] = (value >> 16) & 0xFF;  /* Data bits 16-23 */
    txBuf[3] = (value >> 24) & 0xFF;  /* Data bits 24-31 */
    txBuf[4] = crc_stpm3x(txBuf, 4);  /* CRC */

    energy_meter_dll_transaction_send(txBuf, STPM34_FRAME_SIZE);

    /* Wait for response */
    gEnergyMeterTimeout = 0;
    do {
        status = energy_meters_process_response();
        gEnergyMeterTimeout++;
        if (gEnergyMeterTimeout > ENERGY_METER_TIMEOUT) {
            energy_meter_dll_transaction_end();
            return 0; /* Timeout */
        }
    } while (status == ENU_EM_STATUS_IDLE);

    energy_meter_dll_transaction_end();

    return 1; /* Success */
}
```

**Usage in state machine:**

```c
case ENU_EM_RESET_CHIP_TX:
    /* Send software reset command via DSP_CR3 register */
{
    u32 dsp_cr3_value = STPM34_DSP_CR3_DEFAULT |
                        STPM34_DSP_CR3_SW_RESET |
                        STPM34_DSP_CR3_SW_LATCH1 |
                        STPM34_DSP_CR3_SW_LATCH2;

    /* Use new 32-bit write function */
    if (energy_meters_write_register_32bit(STPM34_REG_DSP_CR3, dsp_cr3_value)) {
        state = ENU_EM_WRITE_CONFIG_TX;
    } else {
        state = ENU_EM_STOP; /* Write failed */
    }
}
    break;
```

### Testing After Fix

```c
// Test write then read-back
uint32_t write_value = 0x04E00000;  // Default DSP_CR3 value
uint32_t read_value;

energy_meters_write_register_32bit(STPM34_REG_DSP_CR3, write_value);

// Wait for write to complete
HAL_Delay(10);

// Read back
energy_meters_read_register(STPM34_REG_DSP_CR3, &read_value);

// Verify
if (read_value == write_value) {
    printf("✅ 32-bit write/read SUCCESS!\n");
} else {
    printf("❌ Mismatch: wrote 0x%08X, read 0x%08X\n", write_value, read_value);
}
```

---

## Issue #3: Incorrect Register Addresses ⚠️ CRITICAL

### Severity: **CRITICAL**

### Location
- **File:** `/home/user/Wolf/SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_hal.h`
- **Lines:** 133-144 (Power and RMS register definitions)

### Description

Wolf's register addresses don't match ST's official STPM34 register map. Power, energy, and RMS values are located at completely different addresses than Wolf assumes.

### Wolf's Incorrect Addresses

```c
/* Wolf's definitions - WRONG */
#define STPM34_REG_CH1_ACTIVE_POWER     0x28    // ❌ WRONG!
#define STPM34_REG_CH1_REACTIVE_POWER   0x29    // ❌ WRONG!
#define STPM34_REG_CH1_APPARENT_RMS     0x2A    // ❌ WRONG!
#define STPM34_REG_CH2_ACTIVE_POWER     0x2B    // ❌ WRONG!
#define STPM34_REG_CH2_REACTIVE_POWER   0x2C    // ❌ WRONG!
#define STPM34_REG_CH2_APPARENT_RMS     0x2D    // ❌ WRONG!

#define STPM34_REG_CH1_VOLTAGE_RMS      0x2F    // ❌ WRONG!
#define STPM34_REG_CH1_CURRENT_RMS      0x2E    // ❌ WRONG!
#define STPM34_REG_CH2_VOLTAGE_RMS      0x31    // ❌ WRONG!
#define STPM34_REG_CH2_CURRENT_RMS      0x30    // ❌ WRONG!
```

### ST Official Addresses (from stpm_metrology.h)

```c
/* ST's official register map - CORRECT */

/* ============ Channel 1 Data Registers ============ */
#define STPM_CH1_REG1   0x54  /* CH1 Active Energy (32-bit) */
#define STPM_CH1_REG2   0x56  /* CH1 Fundamental Energy (32-bit) */
#define STPM_CH1_REG3   0x58  /* CH1 Reactive Energy (32-bit) */
#define STPM_CH1_REG4   0x5A  /* CH1 Apparent RMS Energy (32-bit) */
#define STPM_CH1_REG5   0x5C  /* CH1 Active Power (32-bit, 29-bit data) */
#define STPM_CH1_REG6   0x5E  /* CH1 Fundamental Power (32-bit) */
#define STPM_CH1_REG7   0x60  /* CH1 Reactive Power (32-bit, 29-bit data) */
#define STPM_CH1_REG8   0x62  /* CH1 Apparent RMS Power (32-bit) */
#define STPM_CH1_REG9   0x64  /* CH1 Apparent Vectorial Power (32-bit) */
#define STPM_CH1_REG10  0x66  /* CH1 Momentary Active Power (32-bit) */
#define STPM_CH1_REG11  0x68  /* CH1 Momentary Fundamental Power (32-bit) */
#define STPM_CH1_REG12  0x6A  /* CH1 AH Accumulator (32-bit) */

/* ============ Channel 2 Data Registers ============ */
#define STPM_CH2_REG1   0x6C  /* CH2 Active Energy */
#define STPM_CH2_REG2   0x6E  /* CH2 Fundamental Energy */
#define STPM_CH2_REG3   0x70  /* CH2 Reactive Energy */
#define STPM_CH2_REG4   0x72  /* CH2 Apparent RMS Energy */
#define STPM_CH2_REG5   0x74  /* CH2 Active Power */
#define STPM_CH2_REG6   0x76  /* CH2 Fundamental Power */
#define STPM_CH2_REG7   0x78  /* CH2 Reactive Power */
#define STPM_CH2_REG8   0x7A  /* CH2 Apparent RMS Power */
#define STPM_CH2_REG9   0x7C  /* CH2 Apparent Vectorial Power */
#define STPM_CH2_REG10  0x7E  /* CH2 Momentary Active Power */
#define STPM_CH2_REG11  0x80  /* CH2 Momentary Fundamental Power */
#define STPM_CH2_REG12  0x82  /* CH2 AH Accumulator */

/* ============ DSP Registers (for RMS values) ============ */
#define STPM_DSP_REG14  0x48  /* Contains V1_RMS (15-bit) and C1_RMS (17-bit) - PACKED! */
#define STPM_DSP_REG15  0x4A  /* Contains V2_RMS (15-bit) and C2_RMS (17-bit) - PACKED! */

/* Bitmasks for extracting RMS values from DSP_REG14/15 */
#define BIT_MASK_STPM_DATA_VRMS      0x00007FFF  /* Bits 0-14: Voltage RMS */
#define BIT_MASK_STPM_DATA_C_RMS     0xFFFF8000  /* Bits 15-31: Current RMS */
#define BIT_MASK_STPM_DATA_C_RMS_SHIFT  15       /* Shift for current RMS */
```

### Comparison Table

| Measurement | Wolf Address | ST Official Address | Difference |
|-------------|--------------|---------------------|------------|
| CH1 Active Power | 0x28 ❌ | 0x5C ✅ | Off by 52 bytes! |
| CH1 Reactive Power | 0x29 ❌ | 0x60 ✅ | Off by 55 bytes! |
| CH1 Voltage RMS | 0x2F ❌ | 0x48 (packed) ✅ | Wrong address + packed |
| CH1 Current RMS | 0x2E ❌ | 0x48 (packed) ✅ | Wrong address + packed |
| CH2 Active Power | 0x2B ❌ | 0x74 ✅ | Off by 73 bytes! |

### Impact

- ✅ **Reading from completely wrong memory locations**
- ✅ **Getting garbage data, reserved values, or unrelated registers**
- ✅ **All measurements are invalid**
- ✅ **No correlation between readings and actual power/voltage/current**

**Example:**
- Wolf tries to read "Channel 1 Active Power" from address 0x28
- Address 0x28 is actually **US_REG3** (UART/SPI Status Register)!
- Reading completely unrelated data!

### Recommended Fix

Replace all register definitions in `energy_meter_hal.h` with correct addresses:

```c
/* ========================================================================
 * STPM34 Register Addresses - CORRECTED TO MATCH ST OFFICIAL
 * ======================================================================== */

/* ============ Configuration Registers (Write during init) ============ */
#define STPM34_REG_DSP_CR1          0x00    /* DSP Control Register 1 */
#define STPM34_REG_DSP_CR2          0x02    /* DSP Control Register 2 */
#define STPM34_REG_DSP_CR3          0x04    /* DSP Control Register 3 (Reset, Latch) */
#define STPM34_REG_DSP_CR4          0x06    /* DSP Control Register 4 */
#define STPM34_REG_DFE_CR1          0x18    /* Digital Front End Control 1 (CH1 gain) */
#define STPM34_REG_DFE_CR2          0x1A    /* Digital Front End Control 2 (CH2 gain) */
#define STPM34_REG_US_REG1          0x24    /* UART/SPI config 1 (CRC enable) */
#define STPM34_REG_US_REG2          0x26    /* UART/SPI config 2 (baud rate) */
#define STPM34_CONFIG_REGS_END      0x29    /* Last configuration register */

/* ============ Channel 1 Data Registers ============ */
#define STPM34_REG_CH1_ACTIVE_ENERGY    0x54    /* CH1_REG1 - Active Energy */
#define STPM34_REG_CH1_FUND_ENERGY      0x56    /* CH1_REG2 - Fundamental Energy */
#define STPM34_REG_CH1_REACTIVE_ENERGY  0x58    /* CH1_REG3 - Reactive Energy */
#define STPM34_REG_CH1_APPARENT_ENERGY  0x5A    /* CH1_REG4 - Apparent RMS Energy */
#define STPM34_REG_CH1_ACTIVE_POWER     0x5C    /* CH1_REG5 - Active Power */
#define STPM34_REG_CH1_FUND_POWER       0x5E    /* CH1_REG6 - Fundamental Power */
#define STPM34_REG_CH1_REACTIVE_POWER   0x60    /* CH1_REG7 - Reactive Power */
#define STPM34_REG_CH1_APPARENT_RMS     0x62    /* CH1_REG8 - Apparent RMS Power */
#define STPM34_REG_CH1_APPARENT_VEC     0x64    /* CH1_REG9 - Apparent Vectorial Power */
#define STPM34_REG_CH1_MOMENTARY_POW    0x66    /* CH1_REG10 - Momentary Active Power */
#define STPM34_REG_CH1_AH_ACCUM         0x6A    /* CH1_REG12 - AH Accumulator */

/* ============ Channel 2 Data Registers ============ */
#define STPM34_REG_CH2_ACTIVE_ENERGY    0x6C    /* CH2_REG1 - Active Energy */
#define STPM34_REG_CH2_FUND_ENERGY      0x6E    /* CH2_REG2 - Fundamental Energy */
#define STPM34_REG_CH2_REACTIVE_ENERGY  0x70    /* CH2_REG3 - Reactive Energy */
#define STPM34_REG_CH2_APPARENT_ENERGY  0x72    /* CH2_REG4 - Apparent RMS Energy */
#define STPM34_REG_CH2_ACTIVE_POWER     0x74    /* CH2_REG5 - Active Power */
#define STPM34_REG_CH2_FUND_POWER       0x76    /* CH2_REG6 - Fundamental Power */
#define STPM34_REG_CH2_REACTIVE_POWER   0x78    /* CH2_REG7 - Reactive Power */
#define STPM34_REG_CH2_APPARENT_RMS     0x7A    /* CH2_REG8 - Apparent RMS Power */
#define STPM34_REG_CH2_APPARENT_VEC     0x7C    /* CH2_REG9 - Apparent Vectorial Power */
#define STPM34_REG_CH2_MOMENTARY_POW    0x7E    /* CH2_REG10 - Momentary Active Power */
#define STPM34_REG_CH2_AH_ACCUM         0x82    /* CH2_REG12 - AH Accumulator */

/* ============ DSP Registers (RMS values are PACKED) ============ */
#define STPM34_REG_DSP_REG14            0x48    /* V1_RMS and C1_RMS (packed) */
#define STPM34_REG_DSP_REG15            0x4A    /* V2_RMS and C2_RMS (packed) */

/* Bitmasks for extracting packed RMS values */
#define STPM34_VRMS_MASK                0x00007FFF  /* Bits 0-14: Voltage RMS */
#define STPM34_CRMS_MASK                0xFFFF8000  /* Bits 15-31: Current RMS */
#define STPM34_CRMS_SHIFT               15          /* Shift for current RMS */

#define STPM34_DATA_REGS_START          0x48    /* First data register */
#define STPM34_DATA_REGS_END            0x8B    /* Last data register */
```

### Update State Machine Register Reading

```c
// In energy_meters_handler() - ENU_EM_SEND_READ_REQ state
// Update register cycling to use correct addresses

case ENU_EM_WAIT_FOR_RESPONSE:
    status = energy_meters_process_response();

    if (status == ENU_EM_STATUS_SUCCESS) {
        energy_meter_dll_transaction_end();

        if (gStoreResponseData && gPreviousRegister >= STPM34_DATA_REGS_START) {
            /* Store response to PREVIOUS register address */
            energy_meters_update_cached_values(gPreviousRegister, gLastReadValue);
        }

        /* Update tracking */
        gPreviousRegister = gCurrentRegister;
        gStoreResponseData = 1;

        /* Cycle through measurement registers with CORRECT addresses */
        switch (gCurrentRegister) {
            case STPM34_REG_CH1_ACTIVE_POWER:      // 0x5C
                gCurrentRegister = STPM34_REG_CH1_REACTIVE_POWER;  // 0x60
                break;
            case STPM34_REG_CH1_REACTIVE_POWER:    // 0x60
                gCurrentRegister = STPM34_REG_DSP_REG14;  // 0x48 (V/I RMS packed)
                break;
            case STPM34_REG_DSP_REG14:             // 0x48
                gCurrentRegister = STPM34_REG_CH2_ACTIVE_POWER;  // 0x74
                break;
            case STPM34_REG_CH2_ACTIVE_POWER:      // 0x74
                gCurrentRegister = STPM34_REG_CH2_REACTIVE_POWER;  // 0x78
                break;
            case STPM34_REG_CH2_REACTIVE_POWER:    // 0x78
                gCurrentRegister = STPM34_REG_DSP_REG15;  // 0x4A
                break;
            case STPM34_REG_DSP_REG15:             // 0x4A
                /* Wrap back to start */
                gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;  // 0x5C
                break;
            default:
                gCurrentRegister = STPM34_REG_CH1_ACTIVE_POWER;  // 0x5C
                break;
        }

        state = ENU_EM_SEND_READ_REQ;
    }
    break;
```

---

## Issue #4: RMS Values Are Bit-Packed, Not Separate Registers ⚠️ CRITICAL

### Severity: **CRITICAL**

### Location
- **File:** `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/energy_meters.c`
- **Functions:** `energy_meters_update_cached_values()`, `energy_meters_read_rms_voltage()`, `energy_meters_read_rms_current()`

### Description

STPM34 **packs voltage RMS (15 bits) and current RMS (17 bits) into a SINGLE 32-bit register** to save space. They are NOT in separate registers as Wolf assumes.

- **DSP_REG14 (0x48)** contains BOTH V1_RMS and C1_RMS
- **DSP_REG15 (0x4A)** contains BOTH V2_RMS and C2_RMS

Wolf tries to read them from separate (wrong) addresses and doesn't extract the bit fields.

### Register Bit Layout

```
DSP_REG14 (Address 0x48) - Channel 1 RMS values:
┌─────────────────────────────────────────────────────────────┐
│ 31  30  29  28  27  26  ...  17  16  15  14  ...  2   1   0 │
├──────────────────────────────┬──────────────────────────────┤
│      C1_RMS (17 bits)        │      V1_RMS (15 bits)        │
│    Bits 31-15 (current)      │    Bits 14-0 (voltage)       │
└──────────────────────────────┴──────────────────────────────┘

To extract:
  V1_RMS = DSP_REG14 & 0x00007FFF           (mask bits 0-14)
  C1_RMS = (DSP_REG14 & 0xFFFF8000) >> 15   (mask bits 15-31, shift right)
```

### ST Official Approach

```c
/* From stpm_metrology.h */
#define BIT_MASK_STPM_DATA_VRMS       0x00007FFF  /* Bits 0-14 */
#define BIT_MASK_STPM_DATA_C1_RMS     0xFFFF8000  /* Bits 15-31 */
#define BIT_MASK_STPM_DATA_C_RMS_SHIFT   15

/* Usage in ST code */
uint32_t dsp_reg14_value;  // Read from address 0x48

/* Extract voltage RMS */
uint16_t voltage_rms = (dsp_reg14_value & BIT_MASK_STPM_DATA_VRMS);

/* Extract current RMS */
uint32_t current_rms = (dsp_reg14_value & BIT_MASK_STPM_DATA_C1_RMS)
                        >> BIT_MASK_STPM_DATA_C_RMS_SHIFT;
```

### Impact

- ✅ **Voltage RMS readings are invalid** (reading wrong register or mixed data)
- ✅ **Current RMS readings are invalid** (reading wrong register or mixed data)
- ✅ **Power calculations based on V/I RMS are all wrong**

### Recommended Fix

Update `energy_meters_update_cached_values()` to handle packed RMS:

```c
/**
 * @brief Update cached measurement values (FIXED for packed RMS)
 */
static void energy_meters_update_cached_values(u8 regAddr, u32 rawValue) {
    uint32_t voltage_raw, current_raw;

    switch (regAddr) {
        case STPM34_REG_CH1_ACTIVE_POWER:      // 0x5C
            gCh1ActivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_CH1_REACTIVE_POWER:    // 0x60
            gCh1ReactivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_DSP_REG14:             // 0x48 - PACKED V1/C1 RMS
            /* Extract voltage RMS from bits 0-14 */
            voltage_raw = rawValue & STPM34_VRMS_MASK;
            gCh1RmsVoltage = energy_meters_convert_voltage(voltage_raw);

            /* Extract current RMS from bits 15-31 */
            current_raw = (rawValue & STPM34_CRMS_MASK) >> STPM34_CRMS_SHIFT;
            gCh1RmsCurrent = energy_meters_convert_current(current_raw);
            break;

        case STPM34_REG_CH2_ACTIVE_POWER:      // 0x74
            gCh2ActivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_CH2_REACTIVE_POWER:    // 0x78
            gCh2ReactivePower = energy_meters_convert_power(rawValue);
            break;

        case STPM34_REG_DSP_REG15:             // 0x4A - PACKED V2/C2 RMS
            /* Extract voltage RMS from bits 0-14 */
            voltage_raw = rawValue & STPM34_VRMS_MASK;
            gCh2RmsVoltage = energy_meters_convert_voltage(voltage_raw);

            /* Extract current RMS from bits 15-31 */
            current_raw = (rawValue & STPM34_CRMS_MASK) >> STPM34_CRMS_SHIFT;
            gCh2RmsCurrent = energy_meters_convert_current(current_raw);
            break;

        default:
            /* Unknown register - do nothing */
            break;
    }
}
```

---

## Issue #5: Power Registers Use 29-Bit Data, Not 24-Bit 🟡 MEDIUM

### Severity: **MEDIUM**

### Location
- **File:** `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/energy_meters.c`
- **Function:** `energy_meters_convert_to_signed()` (line 651)

### Description

Power registers (Active, Reactive, Apparent) contain 29-bit signed values with 3 padding bits in the upper positions. Wolf's code assumes 24-bit values.

### ST Official Bit Layout

```c
/* From stpm_metrology.h */
#define BIT_MASK_STPM_3BIT_PADDING          0xE0000000  /* Bits 29-31: Padding */
#define BIT_MASK_STPM_PRIM_CURR_ACTIVE_POW  0x1FFFFFFF  /* Bits 0-28: 29-bit power */

/* Power register structure:
┌─────────────────────────────────────────────────────────────┐
│ 31  30  29 │ 28  27  ...  2   1   0                         │
├────────────┼───────────────────────────────────────────────┤
│  Padding   │  29-bit signed power value                     │
│  (bits 31-29)│  (bits 28-0, sign bit is bit 28)              │
└────────────┴───────────────────────────────────────────────┘
*/
```

### Wolf's Current Approach (24-bit)

```c
// Lines 651-668 - energy_meters.c
static int32_t energy_meters_convert_to_signed(u32 rawValue) {
    int32_t signedValue;

    /* Check if sign bit (bit 23) is set */
    if (rawValue & 0x800000) {  // ❌ Checking bit 23 instead of bit 28
        /* Negative value - extend sign to 32 bits */
        signedValue = (int32_t)(rawValue | 0xFF000000);
    } else {
        /* Positive value */
        signedValue = (int32_t)rawValue;
    }

    return signedValue;
}
```

### Impact

- ⚠️ Power values above 2^23 will be incorrectly sign-extended
- ⚠️ Range is limited to ±8.4 million instead of ±268 million
- ⚠️ Large power values may wrap incorrectly

### Recommended Fix

```c
/**
 * @brief Convert 29-bit power value to signed 32-bit integer (FIXED)
 *
 * STPM34 power registers use 29-bit two's complement with 3 padding bits.
 *
 * @param[in] rawValue Raw 32-bit value from power register
 * @return Signed 32-bit integer value
 */
static int32_t energy_meters_convert_power_to_signed(u32 rawValue) {
    int32_t signedValue;

    /* Mask to 29 bits (ignore padding bits 31-29) */
    rawValue &= 0x1FFFFFFF;

    /* Check if sign bit (bit 28) is set */
    if (rawValue & 0x10000000) {
        /* Negative value - extend sign to 32 bits */
        signedValue = (int32_t)(rawValue | 0xE0000000);
    } else {
        /* Positive value */
        signedValue = (int32_t)rawValue;
    }

    return signedValue;
}

/**
 * @brief Convert raw power value to Watts (FIXED for 29-bit)
 */
static float energy_meters_convert_power(u32 rawValue) {
    /* Use 29-bit conversion for power */
    int32_t signedValue = energy_meters_convert_power_to_signed(rawValue);

    float power_mW = (float)signedValue * STPM34_POWER_LSB_MW;
    float power_W = (power_mW / 1000.0f) * gVoltageCal * gCurrentCal;

    return power_W;
}
```

**Keep the existing 24-bit function for voltage/current (they are 24-bit):**

```c
/**
 * @brief Convert 24-bit unsigned value to signed integer
 * (Use for voltage and current, NOT for power)
 */
static int32_t energy_meters_convert_to_signed_24bit(u32 rawValue) {
    int32_t signedValue;

    /* Check if sign bit (bit 23) is set */
    if (rawValue & 0x800000) {
        /* Negative value - extend sign to 32 bits */
        signedValue = (int32_t)(rawValue | 0xFF000000);
    } else {
        /* Positive value */
        signedValue = (int32_t)rawValue;
    }

    return signedValue;
}
```

---

## Issue #6: Missing Timing Delays in Initialization 🟡 MEDIUM

### Severity: **MEDIUM**

### Location
- **File:** `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/energy_meters.c`
- **State Machine:** Initialization sequence

### Description

ST's library includes specific timing requirements for STPM34 startup that may be missing in Wolf's code.

### ST Official Timing Sequence

```c
/* From metrology_hal.c - Metro_HAL_init_device() */

/* After power-on */
Metro_HAL_WaitMicroSecond(1000);       // 1ms delay
Metro_HAL_WaitMicroSecond(35000);      // 35ms stabilization delay

/* Hardware reset via SYN pin (3 pulses) */
Metro_HAL_STPM_SYN_reset_3_pulses();

Metro_HAL_WaitMicroSecond(1000);       // 1ms after reset

/* Chip select pulse */
Metro_HAL_CSS_EXT_Device(GPIO_PIN_RESET);  // CS low
Metro_HAL_WaitMicroSecond(100);             // 100us delay
Metro_HAL_CSS_EXT_Device(GPIO_PIN_SET);     // CS high

Metro_HAL_WaitMicroSecond(20000);      // 20ms before first register access
```

### Impact

- ⚠️ Chip may not be fully initialized before first access
- ⚠️ Communication may be unreliable without proper delays
- ⚠️ Random failures may occur

### Recommended Fix

Add delays to initialization sequence:

```c
case ENU_EM_INIT:
    /* Initialize hardware */
    energy_meters_hal_init();
    energy_meter_dll_receive_init();

    /* Wait for chip to stabilize after power-on */
    /* ST recommends 35ms delay */
    HAL_Delay(35);  // Add this!

    /* Start initialization sequence */
    state = ENU_EM_RESET_CHIP_TX;
    break;

case ENU_EM_RESET_CHIP_RX:
    status = energy_meters_process_response();

    if (status == ENU_EM_STATUS_SUCCESS || status == ENU_EM_STATUS_TIMEOUT) {
        energy_meter_dll_transaction_end();

        /* Wait after reset before configuration */
        HAL_Delay(20);  // Add this!

        gCurrentRegister = 0x00;
        state = ENU_EM_WRITE_CONFIG_TX;
    }
    break;
```

---

## Issue #7: Missing Hardware Reset via SYN Pin 🟡 MEDIUM

### Severity: **MEDIUM** (OPTIONAL - software reset may be sufficient)

### Description

ST's library uses the SYN pin for hardware reset with a 3-pulse sequence. Wolf only uses software reset via DSP_CR3.

### ST Official Approach

```c
/* From metrology_hal.c */
void Metro_HAL_STPM_SYN_reset_3_pulses(void) {
    /* Generate 3 pulses on SYN pin */
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(SYN_GPIO_type, SYN_GPIO_pin, GPIO_PIN_SET);
        Metro_HAL_WaitMicroSecond(20);
        HAL_GPIO_WritePin(SYN_GPIO_type, SYN_GPIO_pin, GPIO_PIN_RESET);
        Metro_HAL_WaitMicroSecond(20);
    }
}
```

### Impact

- ⚠️ May have more reliable reset with hardware method
- ⚠️ Software reset should work but is less robust

### Recommended Fix (Optional)

If SYN pin is available on your hardware:

```c
/* In energy_meter_hal.h - add SYN pin definition */
#define ENERGY_METER_SYN_Pin        GPIO_PIN_x
#define ENERGY_METER_SYN_GPIO_Port  GPIOx

/* In energy_meters.c - add hardware reset function */
static void energy_meters_hardware_reset(void) {
    /* Generate 3 pulses on SYN pin */
    for (int i = 0; i < 3; i++) {
        HAL_GPIO_WritePin(ENERGY_METER_SYN_GPIO_Port, ENERGY_METER_SYN_Pin, GPIO_PIN_SET);
        for (volatile int j = 0; j < 2000; j++);  // ~20us delay
        HAL_GPIO_WritePin(ENERGY_METER_SYN_GPIO_Port, ENERGY_METER_SYN_Pin, GPIO_PIN_RESET);
        for (volatile int j = 0; j < 2000; j++);  // ~20us delay
    }
}

/* Use in initialization */
case ENU_EM_INIT:
    energy_meters_hal_init();
    energy_meter_dll_receive_init();

    HAL_Delay(35);  // Stabilization delay

    /* Hardware reset (if SYN pin available) */
    energy_meters_hardware_reset();

    HAL_Delay(1);  // 1ms after reset

    state = ENU_EM_RESET_CHIP_TX;
    break;
```

---

## Issue #8: UART Configuration May Not Match Chip Settings 🟢 LOW

### Severity: **LOW**

### Description

ST explicitly writes baud rate configuration to STPM34's US_REG2 register. Wolf defines the value but it's unclear if it's being written during initialization.

### ST Official Approach

```c
/* Write baud rate to STPM34 */
#define METRO_STPM_UART_BAUDRATE_9600  0x683

p_Metro_Device_Config[id].metro_stpm_reg.UARTSPICR2 = METRO_STPM_UART_BAUDRATE_9600;
Metro_HAL_Stpm_write(id, STPM_USREG2, 1, &value, STPM_WAIT);
```

### Impact

- ⚠️ May work if chip default matches (9600 baud is default)
- ⚠️ Could cause issues if chip is in different baud rate mode

### Recommended Fix

Ensure US_REG2 is written during configuration:

```c
case ENU_EM_WRITE_CONFIG_TX:
{
    u32 reg_value = 0;

    switch (gCurrentRegister) {
        case STPM34_REG_US_REG1:
            reg_value = STPM34_US_REG1_DEFAULT;  // CRC enabled
            break;
        case STPM34_REG_US_REG2:
            reg_value = STPM34_US_REG2_DEFAULT;  // Baud rate 9600
            break;
        // ... other registers
    }

    /* Write register using 32-bit write function */
    energy_meters_write_register_32bit(gCurrentRegister, reg_value);

    state = ENU_EM_WRITE_CONFIG_RX;
}
    break;
```

---

## Priority Summary

### 🚨 Priority 1 - CRITICAL (Fix Immediately)

These **MUST** be fixed for the driver to work at all:

1. ✅ **Issue #1:** Read 32 bits (4 bytes) instead of 16 bits
2. ✅ **Issue #2:** Write 32-bit registers with TWO frames
3. ✅ **Issue #3:** Correct all register addresses (0x5C, 0x60, 0x48, etc.)
4. ✅ **Issue #4:** Extract bit-packed RMS values

**Without these fixes, NO measurements will be correct.**

### ⚠️ Priority 2 - HIGH (Fix Soon)

5. ✅ **Issue #5:** Use 29-bit conversion for power values
6. ✅ **Issue #6:** Add timing delays (35ms startup, 20ms after reset)

### 🟡 Priority 3 - MEDIUM (Nice to Have)

7. ⚠️ **Issue #7:** Hardware reset via SYN pin (optional)
8. ⚠️ **Issue #8:** Verify UART configuration register write

---

## Step-by-Step Fix Guide

### Step 1: Fix Data Parsing (Issue #1)

1. Open `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/energy_meters.c`
2. Find function `energy_meters_parse_response()` (line 633)
3. Replace the function with the fixed version:

```c
static u32 energy_meters_parse_response(u8 *rxBuf) {
    u32 value;

    /* Extract full 32-bit word from 4 data bytes */
    value = ((u32)rxBuf[0]) |       // Bits 0-7
            ((u32)rxBuf[1] << 8) |  // Bits 8-15
            ((u32)rxBuf[2] << 16) | // Bits 16-23
            ((u32)rxBuf[3] << 24);  // Bits 24-31

    return value;
}
```

### Step 2: Fix 32-bit Register Writes (Issue #2)

1. Add new function `energy_meters_write_register_32bit()` (see Issue #2 fix above)
2. Update all write operations in state machine to use new function
3. Replace single-frame writes with two-frame writes

### Step 3: Fix Register Addresses (Issue #3)

1. Open `/home/user/Wolf/SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_hal.h`
2. Find lines 133-144 (register address definitions)
3. Replace ALL register addresses with corrected values (see Issue #3 fix above)
4. Add bit masks for packed RMS values

### Step 4: Fix Packed RMS Extraction (Issue #4)

1. Update `energy_meters_update_cached_values()` to extract packed V/I RMS (see Issue #4 fix above)
2. Update register cycling in state machine to read DSP_REG14 (0x48) instead of separate V/I addresses

### Step 5: Test Basic Communication

```c
/* Test code to verify fixes */
void test_stpm34_basic(void) {
    uint32_t dsp_cr3_read;

    /* Write known value to DSP_CR3 */
    uint32_t dsp_cr3_write = 0x04E00000;
    energy_meters_write_register_32bit(STPM34_REG_DSP_CR3, dsp_cr3_write);
    HAL_Delay(10);

    /* Read back */
    energy_meters_read_register(STPM34_REG_DSP_CR3, &dsp_cr3_read);

    /* Verify */
    if (dsp_cr3_read == dsp_cr3_write) {
        printf("✅ DSP_CR3 write/read OK: 0x%08X\n", dsp_cr3_read);
    } else {
        printf("❌ DSP_CR3 mismatch: wrote 0x%08X, read 0x%08X\n",
               dsp_cr3_write, dsp_cr3_read);
    }
}
```

### Step 6: Test Power Measurements

```c
/* Test power reading */
void test_stpm34_power(void) {
    uint32_t raw_power;
    float active_power_w;

    /* Read channel 1 active power */
    energy_meters_read_register(STPM34_REG_CH1_ACTIVE_POWER, &raw_power);

    printf("Raw power register: 0x%08X\n", raw_power);

    /* Convert */
    int32_t signed_power = energy_meters_convert_power_to_signed(raw_power);
    active_power_w = signed_power * STPM34_POWER_LSB_MW / 1000.0f;

    printf("Active Power CH1: %.2f W\n", active_power_w);
}
```

### Step 7: Test RMS Values

```c
/* Test RMS reading */
void test_stpm34_rms(void) {
    uint32_t dsp_reg14;
    uint32_t voltage_raw, current_raw;
    float voltage_v, current_ma;

    /* Read DSP_REG14 (packed V1/C1 RMS) */
    energy_meters_read_register(STPM34_REG_DSP_REG14, &dsp_reg14);

    printf("DSP_REG14: 0x%08X\n", dsp_reg14);

    /* Extract voltage (bits 0-14) */
    voltage_raw = dsp_reg14 & STPM34_VRMS_MASK;
    voltage_v = voltage_raw * STPM34_VOLTAGE_LSB_MV / 1000.0f;

    /* Extract current (bits 15-31) */
    current_raw = (dsp_reg14 & STPM34_CRMS_MASK) >> STPM34_CRMS_SHIFT;
    current_ma = current_raw * STPM34_CURRENT_LSB_MA;

    printf("CH1 Voltage: %.2f V\n", voltage_v);
    printf("CH1 Current: %.2f mA\n", current_ma);
}
```

---

## Expected Results After Fixes

### Before Fixes:
- ❌ All measurements show incorrect values
- ❌ CRC errors may occur frequently
- ❌ Timeouts common
- ❌ Communication unreliable
- ❌ Power readings completely wrong

### After Fixes:
- ✅ DSP_CR3 read-back matches written value
- ✅ Power measurements show realistic values
- ✅ Voltage RMS shows ~220-240V for AC mains
- ✅ Current RMS shows proportional to load
- ✅ CRC validation passes consistently
- ✅ No timeouts under normal operation

---

## Additional ST Official Library Features Not in Wolf

If you want to add more features from ST's library:

1. **SAG/SWELL Detection** - Voltage drop/surge monitoring
2. **Tamper Detection** - Wrong insertion detection
3. **LED Pulse Output** - Proportional to power consumption
4. **Zero-Crossing Detection** - Phase angle measurements
5. **Temperature Compensation** - Automatic calibration adjustment
6. **Phase Calibration** - Correct phase shifts
7. **Programmable Current Gain** - X2/X4/X8/X16 gain settings
8. **AH Accumulation** - Coulomb counting
9. **Interrupt System** - Real-time event notifications
10. **Multi-Device Support** - Up to 2 STPM34 chips

See the comparison report (`COMPARISON_ST_OFFICIAL_vs_WOLF.md`) for details on these features.

---

## References

- **ST Official Library:** `/home/user/Wolf/Metrology/` (ST_LIB branch)
- **ST Register Map:** `/home/user/Wolf/Metrology/drivers/inc/stpm_metrology.h`
- **STPM34 Datasheet:** STMicroelectronics STPM3x documentation
- **Wolf Driver:** `/home/user/Wolf/SMU_Code/Core/ASW/energy_meters/`

---

## Questions or Need Help?

If issues persist after fixes:

1. **Check CRC calculation** - Verify `crc_stpm3x()` function matches ST's
2. **Verify UART settings** - 9600 baud, 8N1, no parity
3. **Check DMA configuration** - TX/RX DMA working correctly
4. **Measure signals** - Use oscilloscope to verify UART frames
5. **Compare with ST library** - Test with ST's code on same hardware

---

**End of Report**

**Generated:** 2025-11-01
**Status:** Ready for Implementation
