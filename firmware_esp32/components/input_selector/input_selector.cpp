#include "input_selector.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "input_selector";

// Pin config — adjust these to match your wiring
#define BUTTON_PIN GPIO_NUM_4    // Button input
#define LED_PIN    GPIO_NUM_2    // Built-in LED on most ESP32 boards

static bool led_state = false;
static int64_t last_isr_time = 0;

// ISR — hardware interrupt, fires when button is pressed
static void IRAM_ATTR button_isr_handler(void *arg) {
    int64_t now = esp_timer_get_time();

    // Debounce — ignore presses within 200ms
    if (now - last_isr_time < 200000) {
        return;
    }
    last_isr_time = now;

    // Toggle LED
    led_state = !led_state;
    gpio_set_level(LED_PIN, led_state ? 1 : 0);
}

void input_selector_init(void) {
    // Configure LED as output
    gpio_config_t led_conf = {};
    led_conf.pin_bit_mask = (1ULL << LED_PIN);
    led_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&led_conf);
    gpio_set_level(LED_PIN, 0);

    // Configure button as input with hardware interrupt
    gpio_config_t btn_conf = {};
    btn_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    btn_conf.mode = GPIO_MODE_INPUT;
    btn_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    btn_conf.intr_type = GPIO_INTR_NEGEDGE;  // Trigger on button press (falling edge)
    gpio_config(&btn_conf);

    // Attach the ISR handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    ESP_LOGI(TAG, "Hardware interrupt initialized — button on GPIO %d, LED on GPIO %d",
             BUTTON_PIN, LED_PIN);
}