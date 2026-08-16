#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Input modes for controlling the robot arm
typedef enum {
    INPUT_MODE_MEDIAPIPE,    // Camera/hand tracking control
    INPUT_MODE_POTENTIOMETER // Manual potentiometer control
} input_mode_t;

// Initialize the input selector and start its task
void input_selector_init(void);

// Get the current input mode (thread-safe)
input_mode_t input_selector_get_mode(void);

// Request a mode change via the queue (call from any task)
void input_selector_request_mode(input_mode_t mode);

