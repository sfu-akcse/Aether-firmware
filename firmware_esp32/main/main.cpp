#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/uart.h"

#include "motor_control.h"


static const char *TAG = "ARM_CALIBRATION";

// UART connection
static constexpr uart_port_t SERVO_UART = UART_NUM_1;
static constexpr int SERVO_TX_PIN = 19;   // D19
static constexpr int SERVO_RX_PIN = 18;   // D18

// Servo IDs
static constexpr int SHOULDER_ID = 1;
static constexpr int ELBOW_ID = 2;

// Reference position
static constexpr int CENTER = 2047;

// VERY small first movement
static constexpr int TEST_OFFSET = 30;

// Therefore:
static constexpr int POSITIVE_TEST = CENTER + TEST_OFFSET; // 2077
static constexpr int NEGATIVE_TEST = CENTER - TEST_OFFSET; // 2017

// Gentle movement settings
static constexpr int SPEED = 150;
static constexpr int ACCELERATION = 20;


void printPosition(
    SMS_STS &bus,
    int id,
    const char *name)
{
    int position = bus.ReadPos(id);

    if (position < 0)
    {
        ESP_LOGW(
            TAG,
            "%s position read FAILED",
            name);
    }
    else
    {
        ESP_LOGI(
            TAG,
            "%s position = %d",
            name,
            position);
    }
}


bool moveServo(
    SMS_STS &bus,
    int id,
    int position,
    const char *name)
{
    ESP_LOGI(
        TAG,
        "%s -> commanding position %d",
        name,
        position);

    int result = bus.WritePosEx(
        id,
        position,
        SPEED,
        ACCELERATION);

    if (result != 1)
    {
        ESP_LOGE(
            TAG,
            "%s movement command FAILED",
            name);

        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(2500));

    printPosition(
        bus,
        id,
        name);

    return true;
}


extern "C" void app_main(void)
{
    ESP_LOGI(
        TAG,
        "===================================");

    ESP_LOGI(
        TAG,
        "Assembled arm direction test");

    ESP_LOGI(
        TAG,
        "===================================");

    SMS_STS bus;

    // Initialize ST3215 communication bus
    if (!setup(
            &bus,
            SERVO_UART,
            SERVO_TX_PIN,
            SERVO_RX_PIN))
    {
        ESP_LOGE(
            TAG,
            "Failed to initialize servo UART");

        return;
    }

    ESP_LOGI(
        TAG,
        "Servo UART initialized");

    /*
     * Wait until both servos respond.
     *
     * This allows the ESP32 to be started first
     * while the 12 V servo supply is still OFF.
     */
    while (true)
    {
        int shoulderPing =
            bus.Ping(SHOULDER_ID);

        vTaskDelay(
            pdMS_TO_TICKS(100));

        int elbowPing =
            bus.Ping(ELBOW_ID);

        if (
            shoulderPing == SHOULDER_ID &&
            elbowPing == ELBOW_ID)
        {
            ESP_LOGI(
                TAG,
                "Both servos detected");

            break;
        }

        ESP_LOGW(
            TAG,
            "Waiting... Shoulder=%s Elbow=%s",
            shoulderPing == SHOULDER_ID
                ? "OK"
                : "FAIL",
            elbowPing == ELBOW_ID
                ? "OK"
                : "FAIL");

        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }

    /*
     * Enable torque.
     */
    bus.EnableTorque(
        SHOULDER_ID,
        1);

    bus.EnableTorque(
        ELBOW_ID,
        1);

    ESP_LOGI(
        TAG,
        "Torque enabled");

    /*
     * First return both motors to our known
     * reference position.
     */
    ESP_LOGI(
        TAG,
        "Returning both joints to centre...");

    bus.WritePosEx(
        SHOULDER_ID,
        CENTER,
        SPEED,
        ACCELERATION);

    vTaskDelay(
        pdMS_TO_TICKS(100));

    bus.WritePosEx(
        ELBOW_ID,
        CENTER,
        SPEED,
        ACCELERATION);

    vTaskDelay(
        pdMS_TO_TICKS(3000));

    printPosition(
        bus,
        SHOULDER_ID,
        "Shoulder");

    printPosition(
        bus,
        ELBOW_ID,
        "Elbow");

    /*
     * =================================================
     * SHOULDER TEST
     * =================================================
     */

    ESP_LOGI(
        TAG,
        "===================================");

    ESP_LOGI(
        TAG,
        "SHOULDER TEST");

    ESP_LOGI(
        TAG,
        "Watch which direction the arm moves.");

    ESP_LOGI(
        TAG,
        "Starting in 3 seconds...");

    vTaskDelay(
        pdMS_TO_TICKS(3000));

    // +30 counts
    if (!moveServo(
            bus,
            SHOULDER_ID,
            POSITIVE_TEST,
            "Shoulder"))
    {
        return;
    }

    // Back to centre
    if (!moveServo(
            bus,
            SHOULDER_ID,
            CENTER,
            "Shoulder"))
    {
        return;
    }

    vTaskDelay(
        pdMS_TO_TICKS(2000));

    // -30 counts
    if (!moveServo(
            bus,
            SHOULDER_ID,
            NEGATIVE_TEST,
            "Shoulder"))
    {
        return;
    }

    // Back to centre
    if (!moveServo(
            bus,
            SHOULDER_ID,
            CENTER,
            "Shoulder"))
    {
        return;
    }

    /*
     * =================================================
     * ELBOW TEST
     * =================================================
     */

    ESP_LOGI(
        TAG,
        "===================================");

    ESP_LOGI(
        TAG,
        "ELBOW TEST");

    ESP_LOGI(
        TAG,
        "Watch which direction the forearm moves.");

    ESP_LOGI(
        TAG,
        "Starting in 3 seconds...");

    vTaskDelay(
        pdMS_TO_TICKS(3000));

    // +30 counts
    if (!moveServo(
            bus,
            ELBOW_ID,
            POSITIVE_TEST,
            "Elbow"))
    {
        return;
    }

    // Back to centre
    if (!moveServo(
            bus,
            ELBOW_ID,
            CENTER,
            "Elbow"))
    {
        return;
    }

    vTaskDelay(
        pdMS_TO_TICKS(2000));

    // -30 counts
    if (!moveServo(
            bus,
            ELBOW_ID,
            NEGATIVE_TEST,
            "Elbow"))
    {
        return;
    }

    // Back to centre
    if (!moveServo(
            bus,
            ELBOW_ID,
            CENTER,
            "Elbow"))
    {
        return;
    }

    ESP_LOGI(
        TAG,
        "===================================");

    ESP_LOGI(
        TAG,
        "CALIBRATION TEST COMPLETE");

    ESP_LOGI(
        TAG,
        "Both joints returned to centre.");

    ESP_LOGI(
        TAG,
        "===================================");

    // Prevent test from repeating
    while (true)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }
}