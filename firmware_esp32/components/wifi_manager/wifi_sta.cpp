#include "wifi_sta.h"

#include <cstring>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

namespace {

constexpr char TAG[] = "wifi_sta";
constexpr EventBits_t CONNECTED_BIT = BIT0;
constexpr EventBits_t FAILED_BIT = BIT1;

EventGroupHandle_t connection_events;
int retry_count;

void wifi_event_handler(void *, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connecting to \"%s\"...", CONFIG_AETHER_WIFI_STA_SSID);
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        const auto *event = static_cast<wifi_event_sta_disconnected_t *>(event_data);
        if (retry_count < CONFIG_AETHER_WIFI_STA_MAX_RETRY) {
            ++retry_count;
            ESP_LOGW(TAG,
                     "Disconnected (reason %u); retry %d/%d",
                     event->reason,
                     retry_count,
                     CONFIG_AETHER_WIFI_STA_MAX_RETRY);
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        } else {
            xEventGroupSetBits(connection_events, FAILED_BIT);
        }
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const auto *event = static_cast<ip_event_got_ip_t *>(event_data);
        retry_count = 0;
        ESP_LOGI(TAG, "Connected; IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));
        xEventGroupSetBits(connection_events, CONNECTED_BIT);
    }
}

esp_err_t initialize_nvs()
{
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "Could not erase NVS");
        error = nvs_flash_init();
    }
    return error;
}

}  // namespace

extern "C" esp_err_t wifi_sta_start(void)
{
    if (std::strlen(CONFIG_AETHER_WIFI_STA_SSID) == 0) {
        ESP_LOGE(TAG, "Wi-Fi SSID is empty; run 'idf.py menuconfig' first");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(initialize_nvs(), TAG, "Could not initialize NVS");
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "Could not initialize TCP/IP stack");
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "Could not create event loop");

    connection_events = xEventGroupCreate();
    if (connection_events == nullptr) {
        return ESP_ERR_NO_MEM;
    }

    esp_netif_t *station = esp_netif_create_default_wifi_sta();
    if (station == nullptr) {
        return ESP_FAIL;
    }

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&init_config), TAG, "Could not initialize Wi-Fi");
    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr),
        TAG,
        "Could not register Wi-Fi event handler");
    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, nullptr),
        TAG,
        "Could not register IP event handler");

    wifi_config_t station_config = {};
    std::strncpy(reinterpret_cast<char *>(station_config.sta.ssid),
                 CONFIG_AETHER_WIFI_STA_SSID,
                 sizeof(station_config.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char *>(station_config.sta.password),
                 CONFIG_AETHER_WIFI_STA_PASSWORD,
                 sizeof(station_config.sta.password) - 1);
    station_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    station_config.sta.pmf_cfg.capable = true;
    station_config.sta.pmf_cfg.required = false;

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Could not set station mode");
    ESP_RETURN_ON_ERROR(
        esp_wifi_set_config(WIFI_IF_STA, &station_config), TAG, "Could not configure station");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Could not start Wi-Fi");

    const EventBits_t result = xEventGroupWaitBits(
        connection_events, CONNECTED_BIT | FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if ((result & CONNECTED_BIT) != 0) {
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Could not connect to \"%s\"", CONFIG_AETHER_WIFI_STA_SSID);
    return ESP_FAIL;
}

/*
CHANGE WI-FI AND LAPTOP IP, BUILD, FLASH, AND MONITOR

//SET WIFI AND LAPTOP IP
cd ~/Aether-firmware/firmware_esp32

get_idf

// Find the Mac's new Wi-Fi IP address:
ipconfig getifaddr en0

// Open configuration:
idf.py menuconfig

// Update these menuconfig sections:
//
// Aether Wi-Fi station configuration
//   → Wi-Fi SSID
//   → Wi-Fi password
//
// Aether TCP receiver configuration
//   → Laptop IPv4 address
//
// Save with S, confirm, then exit with Q.

idf.py build

// Find the ESP32 serial port:
ls /dev/cu.usb*

//TO RUN

// Enter bootloader mode:
// 1. Hold BOOT.
// 2. Press and release RESET/EN.
// 3. Release BOOT.
// 4. hold BOOT again until writing begins.:
// 5. Run:

cd ~/Aether-firmware/firmware_esp32
get_idf
idf.py build
cd build

"$IDF_PYTHON_ENV_PATH/bin/python" -m esptool \
  --chip esp32s3 \
  -p /dev/cu.usbmodem101 \
  -b 115200 \
  --before no-reset \
  --after hard-reset \
  write-flash "@flash_args"

//Release BOOT when writing begins. When flashing finishes, press RESET normally and monitor it:

cd ~/Aether-firmware/firmware_esp32
idf.py -p /dev/cu.usbmodem101 monitor 

// Expected output:
//
// wifi_sta: Connected; IP address: ...
// tcp_receiver: Connecting to laptop TCP sender at LAPTOP_IP:8765
// tcp_receiver: Connected to laptop TCP sender
// tcp_receiver: JSON: {...}

press reset if saying waiting
ctrl + ] to stop
*/