# UGV Controller

This repository contains the code and documentation for an **Unmanned Ground Vehicle (UGV) Controller**, designed to control a UGV (UNSW) remotely using TCP/IP and autonomously.

## 🚗 Project Purpose

The **UGV Controller** is built to:
- Facilitate seamless communication between control components.
- Enable autonomous or semi-autonomous navigation for a UGV.
- Provide a robust foundation for adding advanced features, such as sensor integration and path planning.

## 🛠️ Features

- **Direct Hardware Control**: Uses low-level code to interact with UGV motors and sensors.
- **Communication Protocols**: Handles data transmission and reception for remote operation.
- **Modular Design**: Simplified addition of new features or hardware interfaces.
- **Lightweight Implementation**: Built without external frameworks for optimized performance.

## 📂 Project Structure

```plaintext
UGV_Controller/
├── src/                # Source code files
│   ├── controller.cpp  # Main control logic
│   ├── motor_driver.cpp# Motor control interface
│   ├── sensor_input.cpp# Sensor data handling
│   └── utils.cpp       # Utility functions for calculations and logging
├── include/            # Header files for modularization
│   ├── controller.h
│   ├── motor_driver.h
│   ├── sensor_input.h
│   └── utils.h
├── config/             # Configuration files
│   └── parameters.cfg  # Adjustable control parameters
├── docs/               # Documentation and diagrams
│   └── system_architecture.md
├── README.md           # Project documentation
└── LICENSE             # License for the repository
