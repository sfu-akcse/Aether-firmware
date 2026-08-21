#include "potentiometer.h"

static const char *TAG = "POTENTIOMETER";
static adc_oneshot_unit_handle_t adc1_handle;

void initialize_adc_hardware() 
{
    ESP_LOGI(TAG, "Initializing ADC for Potentiometer...");

    // Initialize the ADC Unit 1
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));
}

void configure_potentiometer(adc_channel_t channel) 
{
    ESP_LOGI(TAG, "Configuring Potentiometer on Channel %d", channel);
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12, // 11dB attenuation allows reading full 0-3.3V range
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));

    ESP_LOGI(TAG, "potentiometer channel %d initialized", channel);
}

int get_pot_value(adc_channel_t channel) 
{
    int adc_raw_value;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &adc_raw_value));
        
    return adc_raw_value;
}

void get_pot_value_task(void* arg) 
{
    adc_channel_t channel = *(adc_channel_t*)arg;
    while(1) {
        int adc_value;
        vTaskDelay(pdMS_TO_TICKS(500));
        adc_value = get_pot_value(channel);
        ESP_LOGI(TAG, "Potentiometer Raw Value: %d", adc_value);
    }
}