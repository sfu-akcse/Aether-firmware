#include "esp_log.h"
#include "tcp_receiver.h"
#include "wifi_sta.h"

extern "C" void app_main(void)
{
    ESP_LOGI("aether", "Starting Aether Wi-Fi station");
    const esp_err_t result = wifi_sta_start();
    if (result != ESP_OK) {
        ESP_LOGE("aether", "Wi-Fi startup failed: %s", esp_err_to_name(result));
        return;
    }

    ESP_LOGI("aether", "Wi-Fi is ready for network services");
    const esp_err_t receiver_result = tcp_receiver_start();
    if (receiver_result != ESP_OK) {
        ESP_LOGE("aether", "TCP receiver startup failed: %s", esp_err_to_name(receiver_result));
    }
}
