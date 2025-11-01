# STPM34 Hardware Requirements - ST Official Documentation

## 🔴 CRITICAL: Why You're Not Seeing RX Signal

Based on ST Microelectronics official documentation, the root cause is most likely:

**The STPM34 is locked in the WRONG communication mode (SPI instead of UART)**

---

## ST Official UART Mode Selection Requirements

### 1. **SCS Pin State During Power-On** ⚠️ CRITICAL

**From ST Documentation:**
> "To select UART peripheral at startup, **SCS signal must be set HIGH before VCC and EN rise**. SCS must be maintained HIGH until a couple of clock periods (16 MHz) reaches the device. Then, the device is locked in UART mode until a reset by EN pulse or a new power-on sequence is performed."

**For SPI Mode (what might be happening):**
> "If the SCS is set LOW and EN pin goes from low to high, the STPM32 communicates by SPI mode."

### 2. **Mode Lock Behavior**

Once the mode is selected at power-on:
- ✅ Mode is **LOCKED** until power cycle or EN reset
- ❌ **Cannot change mode via software**
- ❌ Sending UART commands to chip in SPI mode = NO RESPONSE

---

## Your Current Problem - Root Cause Analysis

### What's Happening in Your Code

**In `gpio.c` (lines 179-195):**
```c
void MX_EnergyMeter_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOB clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin : PB1 (Energy Meter Chip Select) */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  // ⚠️ No pull-up!
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Set chip select high (inactive) initially */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);  // ⚠️ Too late!
}
```

### The Problem Timeline

```
Time 0: Board powers on
    ↓
    VCC rises
    EN rises
    PB1 (SCS) state = UNDEFINED (floating or LOW)  ⚠️ PROBLEM!
    ↓
    STPM34 sees: VCC ✓  EN ✓  SCS LOW (or floating)
    ↓
    STPM34 Decision: "Lock into SPI mode"  ❌
    ↓
Time 1: STM32 boots, runs main()
    ↓
    MX_EnergyMeter_GPIO_Init() called
    ↓
    PB1 (SCS) set HIGH
    ↓
    ⚠️ TOO LATE! STPM34 already locked in SPI mode!
    ↓
Time 2: Your code sends UART frames
    ↓
    STPM34 listening for SPI frames  ❌
    ↓
    No match → No response → RX line stays quiet
```

---

## Hardware Requirements Checklist

### ✅ What You MUST Have

| Requirement | Status | Notes |
|-------------|--------|-------|
| **SCS pin HIGH at power-on** | ❌ FAIL | Currently undefined/floating |
| **SCS HIGH before VCC rise** | ❌ FAIL | GPIO configured after power-on |
| **SCS HIGH before EN rise** | ❌ FAIL | No EN pin control in code |
| **SCS maintained HIGH during first 16MHz clocks** | ❌ FAIL | Can't guarantee timing |
| **VCC stable** | ❓ Unknown | Check with scope |
| **EN pin properly sequenced** | ❓ Unknown | No EN pin in code - check hardware |
| **UART TX/RX pins configured** | ✅ PASS | PA0=TX, PA1=RX (in usart.c) |
| **Baud rate 9600** | ✅ PASS | Configured correctly |
| **9600 baud default mode** | ✅ PASS | STPM34 default |

---

## Solutions

### Solution 1: Hardware Pull-Up Resistor ⭐ RECOMMENDED

**Add external hardware pull-up resistor to SCS pin:**

```
VCC (3.3V)
    |
   [10kΩ]  ← Pull-up resistor
    |
PB1 (SCS) ────→ STPM34 SCS pin
```

**Why this works:**
- ✅ SCS is HIGH from the moment power is applied
- ✅ SCS is HIGH when VCC rises
- ✅ SCS is HIGH when EN rises
- ✅ STPM34 locks into UART mode
- ✅ STM32 can still control SCS (push-pull output overrides pull-up)

**Implementation:**
1. Solder 10kΩ resistor between PB1 and 3.3V
2. Verify with multimeter (should measure ~3.3V when unpowered)
3. Power cycle the board
4. Test communication

---

### Solution 2: Software Pull-Up (Less Reliable)

**Modify `gpio.c`:**

```c
void MX_EnergyMeter_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIOB clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure GPIO pin : PB1 (Energy Meter Chip Select) */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // ✅ Add internal pull-up
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Set chip select high (inactive) initially */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}
```

**Limitations:**
- ⚠️ Internal pull-up only active AFTER GPIO init
- ⚠️ Still a race condition with power-on
- ⚠️ Weaker than external resistor (~40kΩ vs 10kΩ)
- ✅ May work if STM32 boots faster than STPM34

---

### Solution 3: EN Pin Control (If Available)

**If your hardware has EN pin connected to STM32:**

1. Keep EN LOW initially
2. Configure SCS HIGH
3. Wait 100ms
4. Pulse EN LOW→HIGH to reset STPM34
5. STPM34 sees SCS HIGH during EN rise
6. Locks into UART mode

**Code example:**
```c
void MX_EnergyMeter_GPIO_Init(void)
{
    // Configure SCS as output HIGH
    GPIO_InitStruct.Pin = GPIO_PIN_1;  // SCS
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

    // Configure EN as output LOW initially
    GPIO_InitStruct.Pin = GPIO_PIN_X;  // EN (unknown pin)
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOx, GPIO_PIN_X, GPIO_PIN_RESET);

    HAL_Delay(100);  // Ensure SCS is stable

    // Pulse EN to reset STPM34 with SCS HIGH
    HAL_GPIO_WritePin(GPIOx, GPIO_PIN_X, GPIO_PIN_SET);
    HAL_Delay(10);
}
```

**Note:** You need to identify if EN pin is connected to STM32

---

## Additional ST Requirements

### SYN Signal (Optional but Recommended)

**From ST Community:**
> "The init sequence at power up: the SCS signal must be at high level when power supply and EN signal rise (to select UART mode) and then, you must correctly reset the chipset, using SYN and SCS signals"

**SYN Pin Usage:**
- Can be used for hardware latch (pulse on SYN)
- Recommended for DSP startup in harsh environments
- Can be used with SCS for reset operation

**Check your hardware:** Is SYN pin connected to STM32?

---

### UART Configuration (Already Correct)

✅ **Baud Rate:** 9600 (default for STPM34)
✅ **Data Bits:** 8
✅ **Parity:** None
✅ **Stop Bits:** 1
✅ **MSB First:** Yes (per ST docs)
✅ **Frame:** 5 bytes `[ReadAddr][WriteAddr][DataLow][DataHigh][CRC]`

---

## Testing Procedure

### Step 1: Verify Current Mode

**Power off the board completely**

**Check SCS pin with multimeter:**
```
Measure voltage on PB1 (SCS) while powered off:
- If ~0V → Pin is floating or pulled low → STPM34 will enter SPI mode ❌
- If ~3.3V → Pin has pull-up → STPM34 will enter UART mode ✅
```

### Step 2: Add Hardware Pull-Up

If voltage is 0V:
1. Add 10kΩ resistor between PB1 and 3.3V
2. Verify voltage is now 3.3V (powered off)
3. Power cycle the board completely
4. Test communication

### Step 3: Verify with Logic Analyzer

**After hardware fix:**
```
Expected (UART mode):
CS:  ‾‾‾|_____|‾‾‾‾‾|_____|‾‾‾‾‾
TX:  ---<5bytes>---<5bytes>---<5bytes>---
RX:  ---<5bytes>---<5bytes>---<5bytes>---  ← Should appear!

Still wrong (SPI mode):
CS:  ‾‾‾|_____|‾‾‾‾‾|_____|‾‾‾‾‾
TX:  ---<5bytes>---<5bytes>---<5bytes>---
RX:  __________________________________ ← No signal (mode mismatch)
```

---

## ST Documentation References

### Official Documents

1. **STPM32/STPM33/STPM34 Datasheet**
   - URL: `https://www.st.com/resource/en/datasheet/stpm32.pdf`
   - Section: Communication Interface / UART Mode Selection

2. **UM2092 - Basic Metrology Firmware**
   - URL: `https://www.st.com/resource/en/user_manual/um2092-basic-metrology-firmware-for-the-stm32f103rd-and-stpm32-devices-stmicroelectronics.pdf`
   - Section: STPM32 Initialization

3. **UM2066 - Getting Started with STPM3x**
   - Details on power-on sequence and mode selection

### ST Community Forum Insights

**Key Quote:**
> "The SCS signal must be at high level when power supply and EN signal rise (to select UART mode)"

**Common Issue:**
> "Whether SYN, SCS pin connection is required when using STPM32 UART"
> Answer: Yes, SCS must be HIGH at power-on for UART mode

---

## Summary

### 🔴 Root Cause
**STPM34 is locked in SPI mode because SCS was not HIGH during power-on**

### ✅ Solution
**Add 10kΩ pull-up resistor from PB1 (SCS) to VCC (3.3V)**

### 🧪 Test
**Power cycle completely → Check logic analyzer for RX responses**

---

## Quick Diagnostic

**Ask yourself:**

1. ❓ Is there a pull-up resistor on SCS pin? **NO → Add one!**
2. ❓ Does SCS measure 3.3V when board is powered off? **NO → Add resistor!**
3. ❓ Did you power cycle after recent code changes? **NO → Power cycle!**
4. ❓ Is EN pin controlled by software or hardware? **Check schematic**
5. ❓ Is SYN pin connected? **Check schematic**

**Most likely fix:** Add hardware pull-up resistor to SCS pin and power cycle!
