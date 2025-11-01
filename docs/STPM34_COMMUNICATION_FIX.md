# STPM34 Communication Fix - Complete Solution

## Problem Statement
Logic analyzer showed:
- ✅ TX line had data (MCU sending)
- ❌ RX line had NO signal (STPM34 not responding)
- ❌ Chip Select stuck LOW continuously

## Root Causes Identified

After analyzing sampleCode branch, three critical issues were found:

### Issue 1: Chip Select Stuck LOW ✅ FIXED
**Problem:** `energy_meter_dll_transaction_end()` was never called
**Impact:** CS stayed LOW, STPM34 couldn't process commands
**Solution:** Added `transaction_end()` after each state transition

**Before:**
```c
CS: _____________________ (stuck LOW)
```

**After:**
```c
CS: ___|‾‾‾|___|‾‾‾|___|‾‾‾ (toggles properly)
```

### Issue 2: Wrong Frame Format ✅ FIXED
**Problem:** Used single-address format instead of STPM34's dual-address protocol
**Impact:** STPM34 couldn't understand commands

**STPM34 Protocol:** `[ReadAddr][WriteAddr][DataLow][DataHigh][CRC]`

**Correct Frame Formats:**

| Operation | Byte 0 | Byte 1 | Byte 2-3 | Purpose |
|-----------|--------|--------|----------|---------|
| **Read Data** | `addr` | `0xFF` | `0xFFFF` | Read from addr, no write |
| **Write Config** | `0x01` | `addr` | `data` | Read 0x01, write to addr |
| **Reset Chip** | `0x05` | `0x04` | `data` | Read 0x05, write DSP_CR3 |
| **Latch Data** | `0xFF` | `0x04` | `data` | No read, write DSP_CR3 |

**Changes Made:**
```c
// OLD (WRONG):
txBuf[0] = addr | STPM34_READ_BIT;
txBuf[1] = dataLow;
txBuf[2] = dataHigh;

// NEW (CORRECT):
txBuf[0] = readAddr;   // First operation
txBuf[1] = writeAddr;  // Second operation
txBuf[2] = dataLow;
txBuf[3] = dataHigh;
```

**Response Format:**
```c
// OLD (WRONG): Extract 24-bit value from bytes 1-3
value = (rxBuf[1] << 16) | (rxBuf[2] << 8) | rxBuf[3];

// NEW (CORRECT): Extract 16-bit word (little endian)
value = rxBuf[0] | (rxBuf[1] << 8);
```

### Issue 3: DMA with Completion Callbacks ✅ FIXED
**Problem:** DMA completion flags not being set
**Impact:** State machine couldn't detect when TX/RX completed

**Changes:**
1. Using `HAL_UART_Transmit_DMA` and `HAL_UART_Receive_DMA` (DMA mode as requested)
2. Added completion flags: `gTxComplete`, `gRxComplete`
3. Added UART callbacks to set flags (DMA triggers these callbacks on completion)
4. DMA uses interrupts internally for completion notification

**Added to main.c:**
```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        extern volatile u8 gTxComplete;
        gTxComplete = 1;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        extern volatile u8 gRxComplete;
        gRxComplete = 1;
    }
}
```

**Note:** DMA mode is used per requirements. DMA internally uses interrupts to trigger these callbacks when transfers complete, which is normal HAL behavior.

## Files Modified

1. **SMU_Code/Core/ASW/energy_meters/energy_meters.c**
   - Added `transaction_end()` after all RX states
   - Inline TX frame building for reset/config/latch (correct read addresses)
   - Fixed response parsing to 16-bit little endian

2. **SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_dll.c**
   - Using `HAL_UART_Transmit_DMA` / `HAL_UART_Receive_DMA` (DMA mode)
   - Added `gTxComplete` and `gRxComplete` flags
   - Updated `energy_meter_dll_receive()` to check `gRxComplete`
   - DMA completion triggers callbacks to set flags

3. **SMU_Code/Core/BSW/HAL/energy_meter_hal/energy_meter_dll.h**
   - Exported `gTxComplete` and `gRxComplete` flags

4. **SMU_Code/Core/Src/main.c**
   - Added `HAL_UART_TxCpltCallback`
   - Added `HAL_UART_RxCpltCallback`
   - Included `energy_meter_dll.h`

## Expected Behavior After Fix

### Logic Analyzer Should Show:

```
CS:  ‾‾‾|_____|‾‾‾‾‾|_____|‾‾‾‾‾|_____|‾‾
TX:  ---<RESET>----<CFG1>----<CFG2>----
RX:  ---<RESP>-----<RESP>----<RESP>----  ← NOW VISIBLE!
```

### Communication Sequence:

1. **Reset Phase**
   - TX: `[0x05][0x04][DSP_CR3_value][CRC]`
   - RX: `[data_low][data_high][data2_low][data2_high][CRC]`
   - CS: HIGH after response

2. **Config Write Phase** (21 registers)
   - TX: `[0x01][reg_addr][reg_value][CRC]`
   - RX: `[data_low][data_high][data2_low][data2_high][CRC]`
   - CS: HIGH between each register

3. **Latch Phase**
   - TX: `[0xFF][0x04][DSP_CR3_latch][CRC]`
   - RX: `[data_low][data_high][data2_low][data2_high][CRC]`
   - CS: HIGH after response

4. **Read Phase** (continuous)
   - TX: `[reg_addr][0xFF][0xFF][0xFF][CRC]`
   - RX: `[data_low][data_high][data2_low][data2_high][CRC]`
   - CS: HIGH between each read

## Testing Checklist

- [ ] Logic analyzer shows CS toggling (not stuck LOW)
- [ ] TX frames match new two-address format
- [ ] RX line shows responses from STPM34
- [ ] Initialization completes successfully
- [ ] `energy_meters_is_initialized()` returns 1
- [ ] CRC validation passes
- [ ] No timeout errors during init

## Debugging Tips

If still no RX response:
1. Check UART4 baud rate = 9600
2. Verify STPM34 power supply is stable
3. Check TX/RX wiring (not swapped)
4. Verify STPM34 crystal oscillator is running
5. Check if STPM34 firmware is loaded (some require bootloader)

## References

- Implementation based on: `smapleCode` branch
- STPM34 datasheet: `docs/stpm32.pdf`
- Frame format reference: `smapleCode:Core/STPM34/STPM34.c`
