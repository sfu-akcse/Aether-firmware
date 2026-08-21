#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_check.h"

void initialize_adc_hardware();
void configure_potentiometer(adc_channel_t channel);
int get_pot_value(adc_channel_t channel);
void get_pot_value_task(void* arg);