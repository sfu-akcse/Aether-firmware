# Aether-firmware

Welcome to the firmware repository for the Aether Robotic Arm. This codebase manages the ROS 2 environment, bridging high-level computer vision telemetry with low-level hardware actuation over TCP/IP sockets.

## Acknowledgments

* **ST3215 Motor Driver:** The low-level serial communication logic used in the `aether_motor_control` package relies on the [SCServo_Linux](https://github.com/adityakamath/SCServo_Linux) library, written by Aditya Kamath. This provides a robust, POSIX-compliant C++ port of the official Feetech/Waveshare SDK.
