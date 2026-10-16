#include "arm_controller.h"

#include <cmath>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"


static const char *TAG = "ARM_CONTROLLER";


// --------------------------------------------------
// Constructor
// --------------------------------------------------

ArmController::ArmController()
{
}


// --------------------------------------------------
// Initialize arm
// --------------------------------------------------

bool ArmController::begin()
{
    ESP_LOGI(
        TAG,
        "Initializing arm controller");


    // Initialize UART servo bus
    if (!setup(
            &bus,
            SERVO_UART,
            SERVO_TX_PIN,
            SERVO_RX_PIN))
    {
        ESP_LOGE(
            TAG,
            "Failed to initialize servo UART");

        return false;
    }


    ESP_LOGI(
        TAG,
        "Waiting for shoulder and elbow servos...");


    // Wait until both servos respond
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


    ESP_LOGI(
        TAG,
        "Both servos detected");


    // Enable torque
    if (bus.EnableTorque(
            SHOULDER_ID,
            1) != 1)
    {
        ESP_LOGE(
            TAG,
            "Failed to enable shoulder torque");

        return false;
    }


    if (bus.EnableTorque(
            ELBOW_ID,
            1) != 1)
    {
        ESP_LOGE(
            TAG,
            "Failed to enable elbow torque");

        return false;
    }


    ESP_LOGI(
        TAG,
        "Arm controller ready");


    ESP_LOGI(
        TAG,
        "Current shoulder = %d",
        getShoulderPosition());


    ESP_LOGI(
        TAG,
        "Current elbow = %d",
        getElbowPosition());


    return true;
}


// --------------------------------------------------
// Check raw servo safety
// --------------------------------------------------

bool ArmController::positionsAreSafe(
    int shoulderPosition,
    int elbowPosition)
{
    if (
        shoulderPosition < SHOULDER_MIN ||
        shoulderPosition > SHOULDER_MAX)
    {
        ESP_LOGE(
            TAG,
            "Unsafe shoulder position: %d "
            "(allowed %d-%d)",
            shoulderPosition,
            SHOULDER_MIN,
            SHOULDER_MAX);

        return false;
    }


    if (
        elbowPosition < ELBOW_MIN ||
        elbowPosition > ELBOW_MAX)
    {
        ESP_LOGE(
            TAG,
            "Unsafe elbow position: %d "
            "(allowed %d-%d)",
            elbowPosition,
            ELBOW_MIN,
            ELBOW_MAX);

        return false;
    }


    return true;
}


// --------------------------------------------------
// Wait until servo arrives
// --------------------------------------------------

bool ArmController::waitForPosition(
    int id,
    int target,
    const char *name)
{
    for (
        int elapsed = 0;
        elapsed <= MOVEMENT_TIMEOUT_MS;
        elapsed += POLL_INTERVAL_MS)
    {
        int position =
            bus.ReadPos(id);


        if (position >= 0)
        {
            int error =
                std::abs(
                    position - target
                );


            if (elapsed % 500 == 0)
            {
                ESP_LOGI(
                    TAG,
                    "%s current=%d target=%d error=%d",
                    name,
                    position,
                    target,
                    error);
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


    ESP_LOGE(
        TAG,
        "%s failed to reach target %d",
        name,
        target);


    return false;
}


// --------------------------------------------------
// Move using raw servo positions
// --------------------------------------------------

bool ArmController::moveToPositions(
    int shoulderPosition,
    int elbowPosition)
{
    if (!positionsAreSafe(
            shoulderPosition,
            elbowPosition))
    {
        ESP_LOGE(
            TAG,
            "Movement rejected by safety limits");

        return false;
    }


    ESP_LOGI(
        TAG,
        "Moving arm:");

    ESP_LOGI(
        TAG,
        "Shoulder -> %d",
        shoulderPosition);

    ESP_LOGI(
        TAG,
        "Elbow -> %d",
        elbowPosition);


    // Command shoulder
    if (bus.WritePosEx(
            SHOULDER_ID,
            shoulderPosition,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Shoulder movement command failed");

        return false;
    }


    // Very small bus spacing
    vTaskDelay(
        pdMS_TO_TICKS(100));


    // Command elbow
    if (bus.WritePosEx(
            ELBOW_ID,
            elbowPosition,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Elbow movement command failed");

        return false;
    }


    // Wait until both reach the target
    if (!waitForPosition(
            SHOULDER_ID,
            shoulderPosition,
            "Shoulder"))
    {
        return false;
    }


    if (!waitForPosition(
            ELBOW_ID,
            elbowPosition,
            "Elbow"))
    {
        return false;
    }


    ESP_LOGI(
        TAG,
        "Arm movement complete");


    return true;
}


// --------------------------------------------------
// Shoulder angle -> servo count
// --------------------------------------------------

int ArmController::shoulderAngleToPosition(
    float degrees)
{
    return static_cast<int>(
        std::lround(
            SHOULDER_CENTER
            +
            SHOULDER_DIRECTION
            * degrees
            * COUNTS_PER_DEGREE
        )
    );
}


// --------------------------------------------------
// Elbow angle -> servo count
// --------------------------------------------------

int ArmController::elbowAngleToPosition(
    float degrees)
{
    return static_cast<int>(
        std::lround(
            ELBOW_CENTER
            +
            ELBOW_DIRECTION
            * degrees
            * COUNTS_PER_DEGREE
        )
    );
}


// --------------------------------------------------
// Move using theoretical joint angles
// --------------------------------------------------

bool ArmController::moveToAngles(
    float shoulderDegrees,
    float elbowDegrees)
{
    int shoulderPosition =
        shoulderAngleToPosition(
            shoulderDegrees);

    int elbowPosition =
        elbowAngleToPosition(
            elbowDegrees);


    ESP_LOGI(
        TAG,
        "Joint-angle command:");

    ESP_LOGI(
        TAG,
        "Shoulder = %.2f deg -> %d",
        shoulderDegrees,
        shoulderPosition);

    ESP_LOGI(
        TAG,
        "Elbow = %.2f deg -> %d",
        elbowDegrees,
        elbowPosition);


    return moveToPositions(
        shoulderPosition,
        elbowPosition
    );
}


// --------------------------------------------------
// Park arm vertically
// --------------------------------------------------

bool ArmController::park()
{
    ESP_LOGI(
        TAG,
        "Parking arm vertically");


    /*
     * Straighten elbow first.
     *
     * This matches the park procedure we already
     * tested successfully.
     */
    if (bus.WritePosEx(
            ELBOW_ID,
            ELBOW_PARK,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Elbow park command failed");

        return false;
    }


    if (!waitForPosition(
            ELBOW_ID,
            ELBOW_PARK,
            "Elbow"))
    {
        return false;
    }


    /*
     * Then raise shoulder to vertical.
     */
    if (bus.WritePosEx(
            SHOULDER_ID,
            SHOULDER_PARK,
            SPEED,
            ACCELERATION) != 1)
    {
        ESP_LOGE(
            TAG,
            "Shoulder park command failed");

        return false;
    }


    if (!waitForPosition(
            SHOULDER_ID,
            SHOULDER_PARK,
            "Shoulder"))
    {
        return false;
    }


    ESP_LOGI(
        TAG,
        "Arm parked");


    return true;
}


// --------------------------------------------------
// Position readback
// --------------------------------------------------

int ArmController::getShoulderPosition()
{
    return bus.ReadPos(
        SHOULDER_ID
    );
}


int ArmController::getElbowPosition()
{
    return bus.ReadPos(
        ELBOW_ID
    );
}