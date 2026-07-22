#include "tcp_receiver.h"

#include <cerrno>
#include <cstring>
#include <string>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "sdkconfig.h"

#ifndef CONFIG_AETHER_TCP_HOST
#define CONFIG_AETHER_TCP_HOST "10.0.0.20"
#endif

namespace {

constexpr char TAG[] = "tcp_receiver";
constexpr int RECEIVE_BUFFER_SIZE = 512;
constexpr int TASK_STACK_SIZE = 6144;

void receive_stream(int client_socket)
{
    std::string pending;
    pending.reserve(RECEIVE_BUFFER_SIZE);
    char buffer[RECEIVE_BUFFER_SIZE];
    bool discarding_oversized_line = false;

    while (true) {
        const int received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (received == 0) {
            ESP_LOGI(TAG, "Laptop disconnected");
            return;
        }
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            ESP_LOGE(TAG, "Receive failed: errno %d (%s)", errno, std::strerror(errno));
            return;
        }

        for (int index = 0; index < received; ++index) {
            const char byte = buffer[index];
            if (byte == '\n') {
                if (discarding_oversized_line) {
                    ESP_LOGW(TAG, "Discarded oversized JSON line");
                } else if (!pending.empty()) {
                    if (pending.back() == '\r') {
                        pending.pop_back();
                    }
                    if (!pending.empty()) {
                        ESP_LOGI(TAG, "JSON: %s", pending.c_str());
                    }
                }
                pending.clear();
                discarding_oversized_line = false;
                continue;
            }

            if (discarding_oversized_line) {
                continue;
            }
            if (pending.size() >= CONFIG_AETHER_TCP_MAX_LINE_LENGTH) {
                pending.clear();
                discarding_oversized_line = true;
                continue;
            }
            pending.push_back(byte);
        }
    }
}

int connect_to_laptop()
{
    const int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client_socket < 0) {
        ESP_LOGE(TAG, "Could not create socket: errno %d", errno);
        return -1;
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, CONFIG_AETHER_TCP_HOST, &address.sin_addr) != 1) {
        ESP_LOGE(TAG, "Invalid laptop IPv4 address: %s", CONFIG_AETHER_TCP_HOST);
        close(client_socket);
        return -1;
    }
    address.sin_port = htons(CONFIG_AETHER_TCP_PORT);

    ESP_LOGI(TAG, "Connecting to laptop TCP sender at %s:%d", CONFIG_AETHER_TCP_HOST,
             CONFIG_AETHER_TCP_PORT);
    if (connect(client_socket, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
        ESP_LOGW(TAG, "Connection failed: errno %d (%s); retrying", errno, std::strerror(errno));
        close(client_socket);
        return -1;
    }

    return client_socket;
}

void receiver_task(void *)
{
    while (true) {
        const int client_socket = connect_to_laptop();
        if (client_socket < 0) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        ESP_LOGI(TAG, "Connected to laptop TCP sender");
        receive_stream(client_socket);
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

}  // namespace

extern "C" esp_err_t tcp_receiver_start(void)
{
    const BaseType_t created =
        xTaskCreate(receiver_task, "tcp_receiver", TASK_STACK_SIZE, nullptr, 5, nullptr);
    if (created != pdPASS) {
        ESP_LOGE(TAG, "Could not create receiver task");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
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