#include "input_selector.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "input_selector";

// Default to mediapipe mode
static input_mode_t current_mode = INPUT_MODE_MEDIAPIPE;

void input_selector_init(void) {
    current_mode = INPUT_MODE_MEDIAPIPE;
    ESP_LOGI(TAG, "Input selector initialized, default mode: MEDIAPIPE");
}

input_mode_t input_selector_get_mode(void) {
    return current_mode;
}

void input_selector_set_mode(input_mode_t mode) {
    current_mode = mode;
    if (mode == INPUT_MODE_MEDIAPIPE) {
        ESP_LOGI(TAG, "Switched to MEDIAPIPE mode");
    } else {
        ESP_LOGI(TAG, "Switched to POTENTIOMETER mode");
    }
}
