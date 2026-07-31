#pragma once

// Input modes for controlling the robot arm
typedef enum {
    INPUT_MODE_MEDIAPIPE,    // Camera/hand tracking control
    INPUT_MODE_POTENTIOMETER // Manual potentiometer control
} input_mode_t;

// Initialize the input selector
void input_selector_init(void);

// Get the current input mode
input_mode_t input_selector_get_mode(void);

// Set the input mode
void input_selector_set_mode(input_mode_t mode);
