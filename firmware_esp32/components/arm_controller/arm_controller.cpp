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
    // --------------------------------------------------
    // Safety check
    // --------------------------------------------------

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
        "Moving arm synchronously:");

    ESP_LOGI(
        TAG,
        "Shoulder -> %d",
        shoulderPosition);

    ESP_LOGI(
        TAG,
        "Elbow -> %d",
        elbowPosition);


    // --------------------------------------------------
    // Prepare synchronized command
    // --------------------------------------------------

    u8 ids[2] =
    {
        SHOULDER_ID,
        ELBOW_ID
    };


    s16 positions[2] =
    {
        static_cast<s16>(shoulderPosition),
        static_cast<s16>(elbowPosition)
    };


    u16 speeds[2] =
    {
        SPEED,
        SPEED
    };


    u8 accelerations[2] =
    {
        ACCELERATION,
        ACCELERATION
    };


    // --------------------------------------------------
    // Send BOTH servo commands together
    // --------------------------------------------------

    bus.SyncWritePosEx(
        ids,
        2,
        positions,
        speeds,
        accelerations
    );


    ESP_LOGI(
        TAG,
        "Synchronized command sent");


    // --------------------------------------------------
    // Wait for shoulder
    // --------------------------------------------------

    if (!waitForPosition(
            SHOULDER_ID,
            shoulderPosition,
            "Shoulder"))
    {
        return false;
    }


    // --------------------------------------------------
    // Wait for elbow
    // --------------------------------------------------

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

bool ArmController::commandAngles(
    float shoulderDegrees,
    float elbowDegrees)
{
    const int shoulderPosition =
        shoulderAngleToPosition(
            shoulderDegrees);

    const int elbowPosition =
        elbowAngleToPosition(
            elbowDegrees);


    if (!positionsAreSafe(
            shoulderPosition,
            elbowPosition))
    {
        ESP_LOGE(
            TAG,
            "Live command rejected by safety limits");

        return false;
    }


    u8 ids[2] =
    {
        SHOULDER_ID,
        ELBOW_ID
    };


    s16 positions[2] =
    {
        static_cast<s16>(
            shoulderPosition),

        static_cast<s16>(
            elbowPosition)
    };


    u16 speeds[2] =
    {
        SPEED,
        SPEED
    };


    u8 accelerations[2] =
    {
        ACCELERATION,
        ACCELERATION
    };


    bus.SyncWritePosEx(
        ids,
        2,
        positions,
        speeds,
        accelerations
    );


    ESP_LOGI(
        TAG,
        "Live target: "
        "shoulder %.2f -> %d, "
        "elbow %.2f -> %d",
        shoulderDegrees,
        shoulderPosition,
        elbowDegrees,
        elbowPosition);


    // IMPORTANT:
    // No waitForPosition() here.
    return true;
}

// // --------------------------------------------------
// // Move to joint angles using smaller intermediate
// // position steps for gentler large movements.
// // --------------------------------------------------
// bool ArmController::moveToAnglesSmooth(
//     float shoulderDegrees,
//     float elbowDegrees,
//     int maxStepCounts)
// {
//     if (maxStepCounts <= 0)
//     {
//         ESP_LOGE(
//             TAG,
//             "maxStepCounts must be positive");

//         return false;
//     }


//     // --------------------------------------------------
//     // Convert final angles to calibrated servo positions
//     // --------------------------------------------------

//     const int targetShoulder =
//         shoulderAngleToPosition(
//             shoulderDegrees);

//     const int targetElbow =
//         elbowAngleToPosition(
//             elbowDegrees);


//     // Final target must still satisfy our normal limits.
//     if (!positionsAreSafe(
//             targetShoulder,
//             targetElbow))
//     {
//         ESP_LOGE(
//             TAG,
//             "Smooth movement target is unsafe");

//         return false;
//     }


//     // --------------------------------------------------
//     // Read where the physical arm currently is
//     // --------------------------------------------------

//     const int startShoulder =
//         getShoulderPosition();

//     const int startElbow =
//         getElbowPosition();


//     if (
//         startShoulder < 0 ||
//         startElbow < 0)
//     {
//         ESP_LOGE(
//             TAG,
//             "Could not read current arm position");

//         return false;
//     }


//     const int shoulderDifference =
//         targetShoulder - startShoulder;

//     const int elbowDifference =
//         targetElbow - startElbow;


//     const int shoulderDistance =
//         std::abs(shoulderDifference);

//     const int elbowDistance =
//         std::abs(elbowDifference);


//     // --------------------------------------------------
//     // Determine how many intermediate steps are needed
//     //
//     // Example:
//     // shoulder needs 900 counts
//     // maxStepCounts = 100
//     //
//     // → about 9 steps
//     // --------------------------------------------------

//     const int largestDistance =
//         shoulderDistance > elbowDistance
//             ? shoulderDistance
//             : elbowDistance;


//     int steps =
//         (largestDistance + maxStepCounts - 1)
//         / maxStepCounts;


//     if (steps < 1)
//     {
//         steps = 1;
//     }


//     ESP_LOGI(
//         TAG,
//         "Smooth joint movement:");

//     ESP_LOGI(
//         TAG,
//         "Shoulder: %d -> %d",
//         startShoulder,
//         targetShoulder);

//     ESP_LOGI(
//         TAG,
//         "Elbow: %d -> %d",
//         startElbow,
//         targetElbow);

//     ESP_LOGI(
//         TAG,
//         "Intermediate steps = %d",
//         steps);


//     // --------------------------------------------------
//     // Move both joints proportionally toward final target
//     // --------------------------------------------------

//     for (
//         int step = 1;
//         step <= steps;
//         step++)
//     {
//         const float progress =
//             static_cast<float>(step)
//             / static_cast<float>(steps);


//         const int nextShoulder =
//             static_cast<int>(
//                 std::lround(
//                     startShoulder
//                     + shoulderDifference
//                     * progress
//                 )
//             );


//         const int nextElbow =
//             static_cast<int>(
//                 std::lround(
//                     startElbow
//                     + elbowDifference
//                     * progress
//                 )
//             );


//         ESP_LOGI(
//             TAG,
//             "Smooth step %d/%d: "
//             "shoulder=%d elbow=%d",
//             step,
//             steps,
//             nextShoulder,
//             nextElbow);


//         if (!moveToPositions(
//                 nextShoulder,
//                 nextElbow))
//         {
//             ESP_LOGE(
//                 TAG,
//                 "Smooth movement failed "
//                 "at step %d/%d",
//                 step,
//                 steps);

//             return false;
//         }
//     }


//     ESP_LOGI(
//         TAG,
//         "Smooth movement complete");


//     return true;
// }

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