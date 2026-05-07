# STM32 HAL Projects

This repository contains STM32 projects generated using STM32CubeMX / STM32CubeIDE with HAL drivers.

The projects mainly focus on learning and experimenting with embedded peripherals, communication protocols, timers, cryptography concepts, and AUTOSAR crypto stack basics.

## Important Notes

- Most of the source code is generated using STM32CubeMX / STM32CubeIDE.
- Only the application-level code has been modified and implemented manually.
- For the MCP2515 CAN controller projects, the driver is taken from an external source and integrated into the project.

---

# Covered Topics

## Basic Embedded Programs
- LED Blinking
- Button Interfacing
- UART Communication
- ADC Single Channel
- ADC Multi Channel
- ADC + LCD Interface

## Communication Protocols
- UART
- CAN Internal Peripheral
- MCP2515 External CAN Controller

## Timer and PWM
- Timer Interrupt
- Timer Polling
- PWM Generation
- Servo Motor Control

## Graphics and RGB
- RGB LED Control
- RGB PWM Rainbow Effects

## Sensors
- Ultrasonic Sensor using Timer Input Capture
- Ultrasonic Sensor using Timer Polling

## Cryptography and Security
- TRNG (True Random Number Generator)
- DRBG (Deterministic Random Bit Generator)
- HMAC DRBG
- AUTOSAR Crypto Demo

---

# Tools and Environment

- STM32CubeIDE
- STM32CubeMX
- STM32 HAL Drivers
- Git & SourceTree
- OpenSSL (for crypto verification)

---

# Hardware Used

- STM32F407 Discovery Board
- MCP2515 CAN Module
- Basic Embedded Peripherals and Sensors

---

# AI Usage

All projects in this repository are developed with the assistance of AI tools for:
- Concept understanding
- Debugging support
- Driver implementation guidance
- Architecture discussions
- Learning acceleration
- Code review and experimentation

The primary goal is educational learning, experimentation, and improving embedded software development skills.

---

# Repository Purpose

This repository is mainly intended for:
- Embedded systems learning
- STM32 HAL practice
- Peripheral driver understanding
- Automotive and cybersecurity learning
- Cryptographic experimentation

---

# Disclaimer

These projects are primarily developed for learning and experimentation purposes.
Some projects may still be under development or refactoring.
