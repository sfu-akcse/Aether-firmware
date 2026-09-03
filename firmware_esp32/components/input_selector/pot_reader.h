#pragma once

#include "driver/adc.h"

// ADC channel for the potentiometer (configurable)
#define POT_ADC_CHANNEL ADC1_CHANNEL_0  // GPIO 36

// Initialize the potentiometer reader task
void pot_reader_init(void);

// Get the latest potentiometer value (0-4095)
int pot_reader_get_value(void);