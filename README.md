# Aether-firmware

Welcome to the firmware repository for the Aether Robotic Arm. This codebase manages the ROS 2 environment, bridging high-level computer vision telemetry with low-level hardware actuation over TCP/IP sockets.

## Acknowledgments

* **ST3215 Motor Driver:** The servo protocol code under `firmware_esp32/components/STServo` is based on the [SCServo_Linux](https://github.com/adityakamath/SCServo_Linux) library, written by Aditya Kamath. This provides a robust, POSIX-compliant C++ port of the official Feetech/Waveshare SDK. The hardware communication layer in this repository has been adapted to use the ESP-IDF UART driver for the ESP32.