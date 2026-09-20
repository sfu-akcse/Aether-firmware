#include "mode_controller.h"
#include "input_selector.h"
#include "pot_reader.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "mode_controller";

static joint_command_t current_command = {};
static joint_command_t mediapipe_command = {};
static SemaphoreHandle_t command_mutex = NULL;

// Map a potentiometer value (0-4095) to a joint angle range
static int16_t map_pot_to_angle(int raw, int16_t min_angle, int16_t max_angle) {
    return min_angle + (int16_t)((int32_t)(raw) * (max_angle - min_angle) / 4095);
}

// Joint limits for potentiometer mode (min, max for each joint)
static const int16_t joint_limits[NUM_JOINTS][2] = {
    {0, 4095},    // Joint 0 - base rotation
    {1024, 3072}, // Joint 1 - shoulder
    {1024, 3072}, // Joint 2 - elbow
    {0, 4095},    // Joint 3 - wrist rotation
    {1024, 3072}, // Joint 4 - wrist pitch
    {1500, 2500}, // Joint 5 - gripper
};

// Task that continuously produces joint commands based on current mode
static void mode_controller_task(void *pvParameters) {
    while (1) {
        input_mode_t mode = input_selector_get_mode();

        if (mode == INPUT_MODE_POTENTIOMETER) {
            // Read pot and map to all joints
            // For now, single pot controls one joint at a time
            // Future: multiple pots or cycling through joints
            int pot_val = pot_reader_get_value();

            joint_command_t cmd = {};
            for (int i = 0; i < NUM_JOINTS; i++) {
                cmd.angles[i] = map_pot_to_angle(
                    pot_val,
                    joint_limits[i][0],
                    joint_limits[i][1]
                );
            }
            cmd.valid = true;

            xSemaphoreTake(command_mutex, portMAX_DELAY);
            current_command = cmd;
            xSemaphoreGive(command_mutex);

        } else if (mode == INPUT_MODE_MEDIAPIPE) {
            // Use the latest mediapipe data that was fed in
            xSemaphoreTake(command_mutex, portMAX_DELAY);
            if (mediapipe_command.valid) {
                current_command = mediapipe_command;
            }
            xSemaphoreGive(command_mutex);
        }

        // Run at 20Hz
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void mode_controller_init(void) {
    command_mutex = xSemaphoreCreateMutex();
    if (command_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create command mutex");
        return;
    }

    current_command.valid = false;
    mediapipe_command.valid = false;

    xTaskCreate(
        mode_controller_task,
        "mode_ctrl",
        4096,
        NULL,
        4,     // Between pot_reader (3) and input_selector (5)
        NULL
    );

    ESP_LOGI(TAG, "Mode controller initialized with %d joints", NUM_JOINTS);
}

joint_command_t mode_controller_get_command(void) {
    joint_command_t cmd;
    xSemaphoreTake(command_mutex, portMAX_DELAY);
    cmd = current_command;
    xSemaphoreGive(command_mutex);
    return cmd;
}

void mode_controller_feed_mediapipe(const joint_command_t *cmd) {
    xSemaphoreTake(command_mutex, portMAX_DELAY);
    mediapipe_command = *cmd;
    xSemaphoreGive(command_mutex);
}