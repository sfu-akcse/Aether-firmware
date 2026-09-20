#pragma once

#include "joint_types.h"

// Initialize the mode controller (call after input_selector_init and pot_reader_init)
void mode_controller_init(void);

// Get the latest joint command (regardless of which mode produced it)
joint_command_t mode_controller_get_command(void);

// Feed mediapipe joint data into the controller (called by TCP handler)
void mode_controller_feed_mediapipe(const joint_command_t *cmd);