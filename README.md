# 🚀 Smart Fan & Lighting Controller with FOTA Bootloader (STM32F401CC)

An industrial-grade **ARM Cortex-M4 Bare-Metal Embedded System** integrating a custom **Firmware Over-The-Air (FOTA)** bootloader, multi-layered software architecture (`MCAL`, `HAL`, `APP`), wireless IoT connectivity via **ESP32**, interactive **TFT Graphical UI**, **NEC Infrared Remote Control**, and **8-bit R2R DAC Audio Feedback**.

---

## 📖 Table of Contents
1. [Project Overview](#-project-overview)
2. [System Architecture & Layered Design](#-system-architecture--layered-design)
3. [Hardware Components & Peripherals](#-hardware-components--peripherals)
4. [FOTA Bootloader & Flash Memory Layout](#-fota-bootloader--flash-memory-layout)
5. [Application Features & Operation Modes](#-application-features--operation-modes)
6. [Repository Directory Structure](#-repository-directory-structure)
7. [Author](#-author)

---

## 📌 Project Overview
Updating firmware on deployed embedded devices traditionally requires physical access and wired programmers (ST-Link/JTAG). This project solves that challenge by implementing a **Custom Bootloader** capable of receiving new firmware images wirelessly over Wi-Fi via an **ESP32 bridge**, parsing **Intel HEX** records in real time, erasing/programming internal Flash sectors on the **STM32F401CC**, and executing a seamless jump to the updated user application.

The main application serves as a **Smart Room Automation Controller**, managing multi-speed fan control, lighting states, graphical status rendering on a TFT screen, 8x8 LED Matrix animations, 7-Segment numeric displays, and voice/tone alerts.

---

## 🏗️ System Architecture & Layered Design
The software is engineered completely from scratch in **Bare-Metal C** following a strict modular, layered architecture for portability and maintainability:

### 1. MCAL (Microcontroller Abstraction Layer)
Direct register-level drivers for **STM32F401CC** internal peripherals:
* **`RCC` (Reset & Clock Control):** Configures system clock (`HSI` / `HSE` / `PLL`) and enables peripheral bus clocks (`AHB1`, `APB1`, `APB2`).
* **`GPIO` (General Purpose I/O):** Manages pin modes (Input, Output, Alternate Function), output types (Push-Pull, Open-Drain), speed, and pull-up/pull-down resistors.
* **`NVIC` & `EXTI` (Nested Vectored Interrupt Controller & External Interrupts):** Handles interrupt priorities, enabling/disabling IRQs, and edge-triggered external lines (used for IR receiver decoding).
* **`STK` (SysTick Timer):** Provides accurate microsecond/millisecond delays, pulse-width timing measurement for NEC IR protocol, and bootloader timeout management.
* **`USART` (Universal Synchronous/Asynchronous Receiver/Transmitter):** Handles high-speed serial communication with the ESP32 module to receive Intel `.hex` firmware records line-by-line.
* **`SPI` (Serial Peripheral Interface):** High-speed full-duplex/simplex master communication to drive the RGB TFT display.
* **`FMI` (Flash Memory Interface):** Unlocks STM32 embedded Flash, performs sector-by-sector erase (`Sector 1` to `Sector 5`), and writes 16-bit half-words / 32-bit words directly into Flash addresses during FOTA updates.
* **`DMA` (Direct Memory Access):** Offloads data transfer tasks from the CPU to optimize display and peripheral performance.

### 2. HAL (Hardware Abstraction Layer)
External device drivers built on top of MCAL:
* **`TFT` (Thin-Film Transistor Display):** Renders UI screens, system status icons, and text over SPI using `ILI9341` / `ST7735` initialization sequences and pixel-mapping commands.
* **`IR` (NEC Infrared Receiver):** Decodes 32-bit NEC IR frames (Start bit, Address, Inverted Address, Command, Inverted Command) by measuring falling-edge time intervals via `EXTI` and `SysTick`.
* **`LEDMRX` (8x8 LED Matrix):** Multiplexes row/column pins using shift registers (`STP` - Serial-to-Parallel `74HC595`) or direct GPIO scanning to display scrolling text and fan/light icons.
* **`SEVENSEG` (7-Segment Display):** Displays current fan speed levels (`0` to `3`) and system state codes.
* **`DAC` (8-Bit R2R Resistor Ladder DAC):** Outputs digital audio arrays at an 8 kHz sampling rate using periodic timer interrupts to generate voice notifications and alert tones.
* **`ESP` (ESP32 Wi-Fi Bridge Interface):** Handles command synchronization and packet acknowledgment (`ok`) during wireless firmware streaming.

### 3. APP (Application & Bootloader Layer)
* **`Bootloader` & `HexParser`:** Validates incoming Intel HEX records, extracts target Flash memory addresses and data payloads, flashes the application bank, and jumps to the main program.
* **`Smart Controller Application`:** Integrates all HAL modules to control fan speeds, lighting relays/LEDs, graphical menus, and audio feedback in real time.

---

## 🛠️ Hardware Components & Peripherals
| Component | Interface / Peripheral | Role in System |
| :--- | :--- | :--- |
| **STM32F401CCU6 (Black Pill)** | ARM Cortex-M4 (84 MHz) | Main System Microcontroller & FOTA Target |
| **ESP32 Module** | `USART1` (TX/RX) | Wi-Fi Connectivity & Cloud Firmware Downloader |
| **TFT Color Display** | `SPI1` + Control GPIOs (`A0`, `RST`) | Graphical User Interface & Mode Visualization |
| **IR Receiver (`TSOP1738`)** | `EXTI` + `SysTick` | Wireless Remote Control Input (NEC Protocol) |
| **8x8 LED Matrix** | `GPIO` / `74HC595` Shift Register | Visual Animations & Status Patterns |
| **7-Segment Display** | `GPIO` / BCD Decoder | Numeric Fan Speed & State Indicator |
| **R2R DAC + Audio Amplifier** | 8-Pin Parallel `GPIO` + `SysTick` | Voice Prompts & System Alert Tones |
| **DC Fan & Lighting Unit** | `GPIO` / PWM / Driver Circuit | Physical Actuators Controlled by User |

---

## 🔄 FOTA Bootloader & Flash Memory Layout

### Flash Memory Partitioning (STM32F401CC - 256 KB Flash)
| Memory Region | Sector(s) | Start Address | Size | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Custom Bootloader** | Sector 0 | `0x0800 0000` | 16 KB | Initializes UART, FMI, STK, and manages FOTA updates |
| **User Application** | Sector 1 – Sector 5 | `0x0800 4000` | 240 KB | Main Smart Fan & Lighting Controller Firmware |

### Bootloader Execution Sequence
1. **Boot & Timeout Window:** On power-up or reset, the MCU starts at `0x08000000` (Bootloader). It initializes `RCC`, `GPIO`, `USART1`, `FMI`, and starts a **SysTick timeout counter** (e.g., 15 seconds) waiting for data from the ESP32.
2. **Firmware Reception & Parsing (`HexParser`):**
   * If the ESP32 sends an update request, the timeout is canceled.
   * On the first record received, the bootloader erases the Application Flash Region (`Sector 1` onwards).
   * The ESP32 sends the `.hex` file line-by-line (`:LLAAAATT[DD...]CC`).
   * **Record Type `00` (Data Record):** Extracts byte count (`LL`), 16-bit low address (`AAAA`), and data bytes (`DD`), then flashes them into `0x08004000 + offset` using `FMI_voidFlashWrite()`.
   * **Record Type `04` (Extended Linear Address):** Updates the upper 16 bits of the 32-bit Flash base address (`0x0800`).
   * **Record Type `01` (End of File):** Signals completion of flashing.
   * After flashing each record, STM32 sends `"ok"` over UART to request the next line from ESP32.
3. **Context Switch & Jump to Application:**
   Once the update finishes (or if the timeout expires with no new update), the bootloader jumps to the main application at `0x08004000`:
   * Relocates the Vector Table Offset Register: `SCB->VTOR = 0x08004000;`
   * Reads the application's Main Stack Pointer (`MSP`) from `*(volatile uint32_t*)0x08004000`.
   * Reads the application's `Reset_Handler` address from `*(volatile uint32_t*)0x08004004` and jumps to it via a function pointer.

---

## ⚡ Application Features & Operation Modes
* **Remote-Controlled Smart Fan:** Adjust fan states (ON/OFF and multi-level speed control) using the IR remote; current speed is reflected immediately on the **7-Segment Display** and **TFT Screen**.
* **Smart Lighting Control:** Toggle room lighting states with instant graphical updates on the **TFT Display**.
* **Dynamic LED Matrix Display:** Shows custom graphical patterns and scrolling status indicators.
* **Acoustic Feedback (DAC):** Plays recorded digital audio clips/alerts when switching modes or welcoming the user.

---

## 📂 Repository Directory Structure
* 📁 **`Codes/`** — Complete Bare-Metal source code organized into `MCAL`, `HAL`, `APP`, `Bootloader`, and `HexParser` modules.
* 📄 **`Grad_Proj_FOTA_SYSTEM2.pdf`** — Comprehensive project documentation, schematic diagrams, hardware connections, and architectural details[cite: 1].

---

## 👨‍💻 Author
**Seif Hussein Alshabory**  
*Mechatronics & Automotive Embedded Software Engineer*  
🔗 **GitHub:** [s-hussein10](https://github.com/s-hussein10)  
🔗 **Portfolio:** [s-hussein10.github.io](https://s-hussein10.github.io)
