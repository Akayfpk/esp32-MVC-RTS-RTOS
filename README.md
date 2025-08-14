# ESP32 MVC + FreeRTOS Project

## Overview
This project demonstrates an **ESP32 application** using the **Model-View-Controller (MVC)** architecture with **FreeRTOS** to manage concurrent tasks for input handling, sensor monitoring, and display updates.

## Features
- Modular MVC architecture
- FreeRTOS-based multitasking
- Wokwi simulation support
- Real-time button and sensor integration
- Thread-safe shared state management

## Architecture
![Architecture Diagram](diagram.png)

## How It Works
1. **Model** – Holds system state and configuration in thread-safe structures.
2. **View** – Renders system state to the OLED/Serial UI.
3. **Controller** – Processes input events and updates the model.
4. **RTOS Tasks** – Run in parallel for input polling, sensor updates, and UI refresh.

## File Structure
