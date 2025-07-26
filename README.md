# Design of an Enhanced Format Preserving Encryption (eFPE) for Lightweight Embedded devices   
Lightweight format‑preserving encryption of numeric strings with Morse‑code LED feedback, implemented on an NXP LPC2138 (ARM7) and simulated in Proteus.

This project implements the eFPE (embedded Format‑Preserving Encryption) algorithm from our ICCCNT 2025 paper:
> **“eFPE: Design, Implementation, and Evaluation of a Lightweight Format‑Preserving Encryption Algorithm for Embedded Systems,”**  
> Nishant V H et al., 16th IEEE ICCCNT 2025.  
---

## 🚀 Features

- **Format‑Preserving Encryption (eFPE):**  
  - Encrypts/decrypts even‑length decimal strings without padding  
  - 8‑round Feistel network with lightweight PRF (S‑box, XOR, rotate)  
- **User I/O:**  
  - **LED blinks** each digit of ciphertext/plaintext in Morse code  
  - (Optional) add a keypad & LCD driver in `encrypt_morse.c` if you port to hardware  
- **Embedded‑Friendly:**  
  - Fits in ≈4.7 kB ROM & 1.3 kB RAM on LPC2138  
- **Simulation:**  
  - Full Proteus co‑simulation with `.pdsprj` and `.hex`  

---

## ⚙️ Getting Started

### Prerequisites

- **Keil μVision** (ARM7 toolchain)  
- **Proteus Design Suite** (for `.pdsprj` simulation)  
- (Optional) LPC2138 dev‑board + LED/keypad/LCD wiring  

### Build & Flash

1. Open your Keil project (e.g. `Encrypt_Morse_Code.uvproj`) or create a new one and add `encrypt_morse.c`.  
2. Set target MCU to **LPC2138**.  
3. Compile → generate `morsecode.hex`.  
4. Flash `morsecode.hex` onto your board (JTAG, ISP, etc.).

### Proteus Simulation

1. Open `IoT_EL/morse_code.pdsprj` in Proteus.  
2. Point the microcontroller’s firmware to `IoT_EL/morsecode.hex`.  
3. Run simulation:  
   - Observe LED toggling in Morse for each digit of your test strings.

---

## 🔍 How It Works

1. **Mode Select** (in hardware you could use a button (here, it's configured to 0)):  
   - Toggle between **Encrypt** and **Decrypt**.  
2. **Enter Data** (over UART or keypad):  
   - Must be an **even‑length** decimal string.  
3. **Process**:  
   - Feistel rounds apply the lightweight PRF to encrypt/decrypt.  
4. **Output**:  
   - LED blinks each resulting digit in Morse code (dot = short blink, dash = long).

---

## ⚖️ Performance

| Component         | ROM (bytes) | RAM (bytes) |
|-------------------|-----------:|-----------:|
| Full `encrypt_morse.c` |      ~4 700 |     ~1 300 |
| eFPE core only    |      ~3 550 |       ~116  |

---

## 📖 Reference
 
> “eFPE: Design, Implementation, and Evaluation of a Lightweight Format‑Preserving Encryption Algorithm for Embedded Systems,”  
> 16th IEEE ICCCNT 2025.  
---


