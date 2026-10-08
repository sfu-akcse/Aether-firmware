#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/uart.h"

#include "motor_control.h"


static const char *TAG = "IK_PATH_TEST";


// --------------------------------------------------
// UART
// --------------------------------------------------

static constexpr uart_port_t SERVO_UART = UART_NUM_1;

static constexpr int SERVO_TX_PIN = 19;
static constexpr int SERVO_RX_PIN = 18;


// --------------------------------------------------
// Servo IDs
// --------------------------------------------------

static constexpr int SHOULDER_ID = 1;
static constexpr int ELBOW_ID = 2;


// --------------------------------------------------
// Calibrated starting pose
// --------------------------------------------------

static constexpr int SHOULDER_START = 2030;
static constexpr int ELBOW_START = 2085;


// --------------------------------------------------
// Motion settings
// --------------------------------------------------

static constexpr int SPEED = 120;
static constexpr int ACCELERATION = 15;


// --------------------------------------------------
// Position monitoring
// --------------------------------------------------

static constexpr int POSITION_TOLERANCE = 10;
static constexpr int POLL_INTERVAL_MS = 100;
static constexpr int MOVEMENT_TIMEOUT_MS = 15000;


// --------------------------------------------------
// One IK path point
// --------------------------------------------------

struct IKPoint
{
    int z;
    int y;

    int shoulder_position;
    int elbow_position;
};


// --------------------------------------------------
// IK-generated path
//
// L1 = 135 mm
// L2 = 85 mm
// --------------------------------------------------

static constexpr IKPoint PATH[] =
{
    {
        218,
        10,
        2125,
        2255
    },

    {
        210,
        20,
        2240,
        2471
    },

    {
        110,
        170,
        2884,
        2624
    },

    {
        7,
        220,
        3034,
        2085
    }
};


static constexpr int PATH_LENGTH =
    sizeof(PATH) / sizeof(PATH[0]);


// --------------------------------------------------
// Wait until servo actually reaches target
// --------------------------------------------------

bool waitForPosition(
    SMS_STS &bus,
    int id,
    int target,
    const char *name)
{
    for (
        int elapsed = 0;
        elapsed <= MOVEMENT_TIMEOUT_MS;
        elapsed += POLL_INTERVAL_MS)
    {
        int position = bus.ReadPos(id);

        if (position >= 0)
        {
            int error = position - target;

            if (error < 0)
            {
                error = -error;
            }

            if (elapsed % 500 == 0)
            {
                ESP_LOGI(
                    TAG,
                    "%s current=%d target=%d",
                    name,
                    position,
                    target);
            }

            if (error <= POSITION_TOLERANCE)
            {
                ESP_LOGI(
                    TAG,
                    "%s reached target: %d",
                    name,
                    position);

                return true;
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }

    int finalPosition =
        bus.ReadPos(id);

    ESP_LOGE(
        TAG,
        "%s did not reach target %d",
        name,
        target);

    ESP_LOGE(
        TAG,
        "%s final readback = %d",
        name,
        finalPosition);

    return false;
}


// --------------------------------------------------
// Move both joints to one path point
// --------------------------------------------------

bool moveToPoint(
    SMS_STS &bus,
    const IKPoint &point,
    int pointNumber)
{
    ESP_LOGI(TAG, "==================================");

    ESP_LOGI(
        TAG,
        "Moving to path point %d",
        pointNumber);

    ESP_LOGI(
        TAG,
        "Target coordinate: z=%d mm, y=%d mm",
        point.z,
        point.y);

    ESP_LOGI(
        TAG,
        "Shoulder target = %d",
        point.shoulder_position);

    ESP_LOGI(
        TAG,
        "Elbow target = %d",
        point.elbow_position);

    ESP_LOGI(TAG, "==================================");


    // --------------------------------------------------
    // Command both joints
    // --------------------------------------------------

    if (bus.WritePosEx(
            SHOULDER_ID,
            point.shoulder_position,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Shoulder command failed");

        return false;
    }


    vTaskDelay(
        pdMS_TO_TICKS(100));


    if (bus.WritePosEx(
            ELBOW_ID,
            point.elbow_position,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Elbow command failed");

        return false;
    }


    // --------------------------------------------------
    // Wait until both joints actually arrive
    // --------------------------------------------------

    if (!waitForPosition(
            bus,
            SHOULDER_ID,
            point.shoulder_position,
            "Shoulder"))
    {
        return false;
    }


    if (!waitForPosition(
            bus,
            ELBOW_ID,
            point.elbow_position,
            "Elbow"))
    {
        return false;
    }


    int shoulderReadback =
        bus.ReadPos(SHOULDER_ID);

    int elbowReadback =
        bus.ReadPos(ELBOW_ID);


    ESP_LOGI(
        TAG,
        "Point %d reached",
        pointNumber);

    ESP_LOGI(
        TAG,
        "Shoulder readback = %d",
        shoulderReadback);

    ESP_LOGI(
        TAG,
        "Elbow readback = %d",
        elbowReadback);


    // Small pause before next point
    vTaskDelay(
        pdMS_TO_TICKS(1500));


    return true;
}


// --------------------------------------------------
// Main
// --------------------------------------------------

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "==================================");
    ESP_LOGI(TAG, "Aether 3-Point IK Path Test");
    ESP_LOGI(TAG, "==================================");

    ESP_LOGI(
        TAG,
        "L1 = 135 mm");

    ESP_LOGI(
        TAG,
        "L2 = 85 mm");


    SMS_STS bus;


    // --------------------------------------------------
    // Initialize servo bus
    // --------------------------------------------------

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


    // --------------------------------------------------
    // Wait for servo power
    // --------------------------------------------------

    ESP_LOGI(
        TAG,
        "Waiting for servos...");


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


    // --------------------------------------------------
    // Enable torque
    // --------------------------------------------------

    bus.EnableTorque(
        SHOULDER_ID,
        1);

    bus.EnableTorque(
        ELBOW_ID,
        1);


    // --------------------------------------------------
    // Establish known starting pose
    // --------------------------------------------------

    ESP_LOGI(
        TAG,
        "Returning arm to straight starting pose");


    if (bus.WritePosEx(
            ELBOW_ID,
            ELBOW_START,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Elbow start command failed");

        return;
    }


    if (!waitForPosition(
            bus,
            ELBOW_ID,
            ELBOW_START,
            "Elbow"))
    {
        return;
    }


    if (bus.WritePosEx(
            SHOULDER_ID,
            SHOULDER_START,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Shoulder start command failed");

        return;
    }


    if (!waitForPosition(
            bus,
            SHOULDER_ID,
            SHOULDER_START,
            "Shoulder"))
    {
        return;
    }


    ESP_LOGI(
        TAG,
        "Straight starting pose confirmed");

    ESP_LOGI(
        TAG,
        "Path begins in 3 seconds");

    ESP_LOGI(
        TAG,
        "Be ready to remove 12 V power if needed");


    vTaskDelay(
        pdMS_TO_TICKS(3000));


    // --------------------------------------------------
    // Execute path
    // --------------------------------------------------

    for (
        int i = 0;
        i < PATH_LENGTH;
        i++)
    {
        if (!moveToPoint(
                bus,
                PATH[i],
                i + 1))
        {
            ESP_LOGE(
                TAG,
                "Path stopped at point %d",
                i + 1);

            return;
        }
    }


    // --------------------------------------------------
    // Finished
    // --------------------------------------------------

    ESP_LOGI(TAG, "==================================");

    ESP_LOGI(
        TAG,
        "IK PATH COMPLETE");

    ESP_LOGI(
        TAG,
        "Final shoulder = %d",
        bus.ReadPos(SHOULDER_ID));

    ESP_LOGI(
        TAG,
        "Final elbow = %d",
        bus.ReadPos(ELBOW_ID));

    ESP_LOGI(TAG, "==================================");


    // Hold final pose
    while (true)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }
}