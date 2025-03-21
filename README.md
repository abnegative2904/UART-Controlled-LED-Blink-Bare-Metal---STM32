# UART-Controlled LED Blink (Bare Metal) - STM32

##  Project Overview
This project demonstrates **bare-metal programming** on an **STM32 microcontroller**, where an LED blinks based on a number received via **UART (USART1)**. The user inputs a number (1-999) through a serial terminal (like PuTTY), and the LED blinks accordingly.

---

## Hardware Requirements
- **STM32 Microcontroller** (e.g., STM32F103C8 "Blue Pill")
- **USB-to-Serial Converter (TTL)** (e.g., CP2102, FTDI, CH340)
- **LED + Resistor** (Optional if using an onboard LED)
- **Connecting Wires**
- **PC with Serial Terminal (e.g., PuTTY, RealTerm, Tera Term)**

---

## 🔌 Wiring Diagram
| STM32 Pin  | USB-to-Serial Converter |
|------------|------------------------|
| **PA9 (USART1_TX)**  | **RX (Receive)** |
| **PA10 (USART1_RX)** | **TX (Transmit)** |
| **GND**  | **GND** |

| STM32 Pin  | LED Connection |
|------------|---------------|
| **PB2**    | **LED Anode (+)** |
| **GND**    | **LED Cathode (-) via Resistor** |

---

##  How It Works
https://github.com/user-attachments/assets/eeee19ed-d96d-4e71-b7d6-799734db91c7
1. **UART Setup:**  
   - STM32 initializes USART1 at **9600 baud rate**.
   - Sends a **"Ready to receive blink count"** message.

2. **User Input via UART:**  
   - User types a number (`1-999`) in the serial terminal.
   - STM32 reads and **converts input to an integer**.

3. **LED Blinking Logic:**  
   - If the number is valid, the **LED blinks** that many times.
   - If another input is received while blinking, it **waits** until blinking is complete.

---


