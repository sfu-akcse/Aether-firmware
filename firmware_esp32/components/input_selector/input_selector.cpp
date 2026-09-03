#include "input_selector.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

static const char *TAG = "input_selector";

static input_mode_t current_mode = INPUT_MODE_MEDIAPIPE;
static QueueHandle_t mode_queue = NULL;
static int64_t last_isr_time = 0;

// ISR handler — runs instantly when the switch pin changes
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    int64_t now = esp_timer_get_time();

    // Debounce: ignore interrupts that fire within 200ms of the last one
    if (now - last_isr_time < INPUT_SELECTOR_DEBOUNCE_MS * 1000) {
        return;
    }
    last_isr_time = now;

    // Toggle the mode
    input_mode_t new_mode = (current_mode == INPUT_MODE_MEDIAPIPE)
        ? INPUT_MODE_POTENTIOMETER
        : INPUT_MODE_MEDIAPIPE;

    // Send to queue from ISR (uses the ISR-safe version)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(mode_queue, &new_mode, &xHigherPriorityTaskWoken);

    // Yield to higher priority task if one was woken
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Task that processes mode change requests from the queue
static void input_selector_task(void *pvParameters) {
    input_mode_t requested_mode;

    while (1) {
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

    // Create the mode queue
    mode_queue = xQueueCreate(5, sizeof(input_mode_t));
    if (mode_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create mode queue");
        return;
    }

    // Configure the switch GPIO pin
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << INPUT_SELECTOR_SWITCH_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;  // Trigger on button press (falling edge)
    gpio_config(&io_conf);

    // Install the GPIO ISR service and attach our handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_SELECTOR_SWITCH_PIN, gpio_isr_handler, NULL);

    // Start the task that processes mode changes
    xTaskCreate(
        input_selector_task,
        "input_selector",
        2048,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "Input selector initialized with ISR on GPIO %d, default mode: MEDIAPIPE",
             INPUT_SELECTOR_SWITCH_PIN);
}

input_mode_t input_selector_get_mode(void) {
    return current_mode;
}

void input_selector_request_mode(input_mode_t mode) {
    if (mode_queue != NULL) {
        xQueueSend(mode_queue, &mode, 0);
    }
}