# STPM34 Implementation Comparison: Our Code vs ST Sample Code

## Executive Summary

Compared our STPM34 implementation with the official sample code from the `smapleCode` branch. Found **4 critical differences** that may affect communication.

---

## ✅ What We Got RIGHT

### 1. **Dual-Address Frame Format** ✅
**Sample Code:**
```c
sTxData[0] = readAddress;
sTxData[1] = writeAddress;
sTxData[2] = writeData & 0xFF;
sTxData[3] = writeData >> 8;
sTxData[4] = STPM_CRC8(sTxData, 4);  // 5th byte
```

**Our Code:**
```c
txBuf[0] = readAddr;    // Read address
txBuf[1] = writeAddr;   // Write address
txBuf[2] = data & 0xFF; // Data LOW
txBuf[3] = (data >> 8); // Data HIGH
// Missing: CRC byte!
```

**Status:** ✅ Frame structure is correct, but missing CRC (see issue #1 below)

### 2. **State Machine Sequence** ✅
Both implementations use the same sequence:
1. RESET_CHIP (write to DSP_CR3 with SW_Reset bit)
2. WRITE_CONFIG_REGS (write all config registers 0x00 to 0x29)
3. LATCH_DATA (write to DSP_CR3 with SW_Latch bits)
4. READ_DATA_REGS (read data registers starting from 0x40)

### 3. **Little Endian Data Format** ✅
Both use LSB-first format for 16-bit data.

### 4. **Transaction End / Chip Select Toggling** ✅
We correctly call `transaction_end()` after each RX state (though sample code doesn't explicitly show CS toggling, your logic analyzer confirmed it's needed).

---

## ❌ CRITICAL DIFFERENCES (Potential Issues)

### Issue #1: Missing CRC Byte ⚠️ **CRITICAL**

**Sample Code:**
- Frame size: **5 bytes** (4 data + 1 CRC)
- Always calculates CRC using polynomial-based algorithm
- Validates received CRC: `if (STPM_CRC8(RxData, 4) == RxData[4])`

**Our Code:**
- Frame size: **4 bytes** (no CRC)
- No CRC calculation or validation

**Impact:**
- STPM34 expects 5-byte frames with CRC
- **Without CRC, STPM34 may reject ALL commands**
- This could explain why you're not getting responses!

**CRC Algorithm from Sample Code:**
```c
uint8_t STPM_CRC8(const uint8_t *data, uint8_t length) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= bitReverse(data[i]);
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07; // Polynomial
            } else {
                crc <<= 1;
            }
        }
    }
    return bitReverse(crc);
}

uint8_t bitReverse(uint8_t byte) {
    byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4);
    byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2);
    byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1);
    return byte;
}
```

---

### Issue #2: Read Address During Configuration Writes

**Sample Code - Read Addresses During Write Operations:**
```c
// Reset chip
STPM_transmit(0x05, 0x05, resetValue);  // Read addr = 0x05

// Write config registers
STPM_transmit(0x01, currentAddr, configValue);  // Read addr = 0x01

// Latch data
STPM_transmit(0xFF, 0x05, latchValue);  // Read addr = 0xFF

// Read data
STPM_transmit(dataAddr, 0xFF, 0xFFFF);  // Write addr = 0xFF
```

**Our Code:**
```c
// Reset: Uses readAddr from previous operation or 0xFF
// Config write: Uses 0xFF
// Latch: Uses 0xFF
// Read: Uses register address
```

**Impact:**
- Using specific read addresses (0x05, 0x01) during writes may be intentional
- Address 0x05 = DSP_CR3H (high byte of DSP_CR3)
- Address 0x01 = DSP_CR1H
- This might be part of the delayed response protocol

---

### Issue #3: Delayed Response Handling ⚠️ **IMPORTANT**

**Sample Code Pattern:**
```c
void handleReadDataRegsRXState(uint8_t Index) {
    // Save response to PREVIOUS read address, not current!
    if (STPM_SM[Index].lastRW_Address >= DATA_REGS_START_ADDRESS) {
        regArray[Index * 140 + (STPM_SM[Index].lastRW_Address)] = ...;
    }

    // Now move to next address
    STPM_SM[Index].lastRW_Address = STPM_SM[Index].RW_Address;
    STPM_SM[Index].RW_Address += 2;
}
```

**What This Means:**
- When you send a read command for address 0x40, the response contains data from the **PREVIOUS** command
- First read returns garbage/default data
- Second read returns data from first address
- Need to send a dummy read at the beginning!

**Our Code:**
- We assume immediate response (response matches current request)
- We don't track `lastRW_Address`

**Impact:**
- **We're interpreting responses incorrectly!**
- Data from address 0x40 is actually stored when we receive response to address 0x42 request

---

### Issue #4: UART Mode - DMA vs IT

**Sample Code:**
```c
HAL_UART_Receive_IT(STPM_UART_Inst, RxData, 5);
HAL_UART_Transmit_IT(STPM_UART_Inst, sTxData, 5);
```

**Our Code:**
```c
HAL_UART_Receive_DMA(&ENERGY_METER_UART, gEnergyRxData, 5);  // Should be 5!
HAL_UART_Transmit_DMA(&ENERGY_METER_UART, gEnergyTxData, size);
```

**Impact:**
- Both DMA and IT modes should work (both trigger same callbacks)
- **But we're using size=4, should be 5 to account for CRC!**

---

## 📋 Recommended Fixes (Priority Order)

### **Priority 1: Add CRC Support** 🔴 CRITICAL
This is likely the main reason STPM34 isn't responding!

1. Add CRC calculation functions (`STPM_CRC8`, `bitReverse`)
2. Change frame size from 4 to 5 bytes
3. Calculate and append CRC before transmission
4. Validate CRC in received frames

**Files to modify:**
- `energy_meter_dll.h` - Change `STPM34_FRAME_SIZE` from 4 to 5
- `energy_meter_dll.c` - Add CRC functions, append CRC to TX, validate RX CRC
- `energy_meters.c` - Update all frame building to include CRC

---

### **Priority 2: Implement Delayed Response Handling** 🟡 HIGH

1. Add `lastReadAddress` tracking to state machine
2. Store received data to **previous** address, not current
3. Send dummy read at beginning of read sequence

**Example:**
```c
// First read (dummy)
sendReadRequest(0x40);
receiveResponse();  // Ignore this data (garbage)

// Second read
sendReadRequest(0x42);
receiveResponse();  // This contains data from 0x40!
```

---

### **Priority 3: Use Correct Read Addresses During Writes** 🟡 MEDIUM

Update write operations to use specific read addresses:
```c
// Reset chip
txBuf[0] = 0x05;  // Read DSP_CR3H
txBuf[1] = 0x04;  // Write DSP_CR3
txBuf[2] = resetValue & 0xFF;
txBuf[3] = resetValue >> 8;
txBuf[4] = CRC;

// Config writes
txBuf[0] = 0x01;  // Read DSP_CR1H
txBuf[1] = configAddr;
...
```

---

## 🔍 Sample Code Observations

### Register Addressing
Sample code uses **byte addressing** (0x00, 0x01, 0x02...) but our implementation uses **word addressing** (0x00, 0x02, 0x04...).

**Sample code addresses:**
- DSP_CR3H_ADDRESS = 0x05 (high byte)
- Config registers: 0x00 to 0x29 (byte-addressed)
- Data registers: 0x40 to 0x8B (byte-addressed)

**Our addresses:**
- DSP_CR3 = 0x04 (word-addressed)
- Config registers increment by 2

**Possible explanation:** Sample code might be using SPI in 8-bit mode, we're using UART in 16-bit word mode.

---

## 📊 Comparison Summary Table

| Feature | Sample Code | Our Code | Status |
|---------|-------------|----------|--------|
| Frame Format | Dual-address | Dual-address | ✅ Match |
| Frame Size | 5 bytes | 4 bytes | ❌ Missing CRC |
| CRC Calculation | Yes (polynomial) | No | ❌ Critical |
| CRC Validation | Yes | No | ❌ Critical |
| State Sequence | Reset→Config→Latch→Read | Same | ✅ Match |
| Delayed Response | Handled | Not handled | ❌ Important |
| UART Mode | IT | DMA | ✅ Both OK |
| Callbacks | Yes | Yes | ✅ Match |
| Chip Select Toggle | Assumed | Explicit | ✅ We're better |
| Read Addr During Write | Specific (0x01, 0x05) | Generic (0xFF) | ⚠️ Different |

---

## 🎯 Next Steps

**Immediate Actions:**
1. **Add CRC support** - This is almost certainly why STPM34 isn't responding
2. **Change frame size to 5 bytes** in all relevant definitions
3. **Test with logic analyzer** - You should see 5-byte frames and STPM34 should respond

**Follow-up Actions:**
4. Implement delayed response handling
5. Use correct read addresses during config writes
6. Verify register addressing (byte vs word)

**Testing:**
- After adding CRC, check logic analyzer:
  - TX should show 5 bytes with valid CRC
  - RX should show 5 bytes (STPM34 response)
  - CS should toggle between transactions

---

## 📚 Reference

**Sample Code Location:** `smapleCode` branch
- `Core/STPM34/STPM34.c` - Main implementation
- `Core/STPM34/STPM34.h` - Header and definitions
- `Core/STPM34/STPM34_define.h` - Register definitions

**Our Implementation:**
- `SMU_Code/Core/ASW/energy_meters/energy_meters.c`
- `SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_dll.c`
- `SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_hal.h`
