#pragma once

#include <stdint.h>

// Number of joints on the robot arm
#define NUM_JOINTS 6

// Joint angle data shared across modes
typedef struct {
    int16_t angles[NUM_JOINTS];  // Position for each joint (0-4095 servo range)
    bool valid;                   // Whether this data is ready to use
} joint_command_t;