#include "pot_reader.h"
#include "input_selector.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "pot_reader";

static int current_value = 0;
static adc_oneshot_unit_handle_t adc_handle = NULL;

// Task that continuously reads the potentiometer when in potentiometer mode
static void pot_reader_task(void *pvParameters) {
    int raw_value = 0;

    while (1) {
        // Only read the pot when we're actually in potentiometer mode
        if (input_selector_get_mode() == INPUT_MODE_POTENTIOMETER) {
            if (adc_oneshot_read(adc_handle, POT_ADC_CHANNEL, &raw_value) == ESP_OK) {
                current_value = raw_value;
                ESP_LOGD(TAG, "Potentiometer value: %d", current_value);
            }
        }

        // Read every 50ms (~20Hz)
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void pot_reader_init(void) {
    // Configure ADC unit
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&unit_cfg, &adc_handle);

    // Configure the ADC channel
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,    // Full 0-3.3V range
        .bitwidth = ADC_BITWIDTH_12,  // 12-bit resolution (0-4095)
    };
    adc_oneshot_config_channel(adc_handle, POT_ADC_CHANNEL, &chan_cfg);

    // Start the reader task
    xTaskCreate(
        pot_reader_task,
        "pot_reader",
        2048,
        NULL,
        3,           // Lower priority than input_selector
        NULL
    );

    ESP_LOGI(TAG, "Potentiometer reader initialized on ADC channel %d", POT_ADC_CHANNEL);
}

int pot_reader_get_value(void) {
    return current_value;
}