#include "input_selector.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "input_selector";

static input_mode_t current_mode = INPUT_MODE_MEDIAPIPE;
static QueueHandle_t mode_queue = NULL;

// Task that waits for mode change requests from the queue
static void input_selector_task(void *pvParameters) {
    input_mode_t requested_mode;

    while (1) {
        // Block until a mode change request arrives
        if (xQueueReceive(mode_queue, &requested_mode, portMAX_DELAY) == pdTRUE) {
            if (requested_mode != current_mode) {
                current_mode = requested_mode;

                if (current_mode == INPUT_MODE_MEDIAPIPE) {
                    ESP_LOGI(TAG, "Switched to MEDIAPIPE mode");
                } else {
                    ESP_LOGI(TAG, "Switched to POTENTIOMETER mode");
                }
            }
        }
    }
}

void input_selector_init(void) {
    current_mode = INPUT_MODE_MEDIAPIPE;

    // Create a queue that holds up to 5 mode change requests
    mode_queue = xQueueCreate(5, sizeof(input_mode_t));
    if (mode_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create mode queue");
        return;
    }

    // Start the input selector task
    BaseType_t result = xTaskCreate(
        input_selector_task,   // function to run
        "input_selector",      // task name for debugging
        2048,                  // stack size in bytes
        NULL,                  // no parameters
        5,                     // priority (mid-level)
        NULL                   // don't need task handle
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create input selector task");
        return;
    }

    ESP_LOGI(TAG, "Input selector initialized, default mode: MEDIAPIPE");
}

input_mode_t input_selector_get_mode(void) {
    return current_mode;
}

void input_selector_request_mode(input_mode_t mode) {
    if (mode_queue != NULL) {
        xQueueSend(mode_queue, &mode, 0);
    }
}