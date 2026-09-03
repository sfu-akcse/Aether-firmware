#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

// Input modes for controlling the robot arm
typedef enum {
    INPUT_MODE_MEDIAPIPE,    // Camera/hand tracking control
    INPUT_MODE_POTENTIOMETER // Manual potentiometer control
} input_mode_t;

// GPIO pin for the mode toggle switch (configurable)
#define INPUT_SELECTOR_SWITCH_PIN GPIO_NUM_4

// Debounce time in milliseconds
#define INPUT_SELECTOR_DEBOUNCE_MS 200

// Initialize the input selector with ISR-based switching
void input_selector_init(void);

// Get the current input mode (thread-safe)
input_mode_t input_selector_get_mode(void);

// Request a mode change via the queue (call from any task or ISR)
void input_selector_request_mode(input_mode_t mode);