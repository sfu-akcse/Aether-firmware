#include <cmath>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "arm_controller.h"
#include "tcp_receiver.h"
#include "wifi_sta.h"


static const char *TAG = "aether";


// --------------------------------------------------
// Control settings
// --------------------------------------------------

// Ignore very small changes.
// Helps prevent tiny noisy updates from constantly
// commanding the servos.
static constexpr float COMMAND_DEADBAND_DEG = 1.0f;


// If no new command arrives for this long,
// park the arm vertically.
static constexpr int COMMAND_TIMEOUT_MS = 5000;


// Run the live control loop every 50 ms.
//
// 50 ms = 20 Hz
static constexpr int CONTROL_PERIOD_MS = 50;


// --------------------------------------------------
// Main
// --------------------------------------------------

extern "C" void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting Aether continuous arm control");


    // --------------------------------------------------
    // 1. Wi-Fi
    // --------------------------------------------------

    if (wifi_sta_start() != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi startup failed");

        return;
    }


    ESP_LOGI(
        TAG,
        "Wi-Fi ready");


    // --------------------------------------------------
    // 2. TCP receiver
    // --------------------------------------------------

    if (tcp_receiver_start() != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "TCP receiver startup failed");

        return;
    }


    ESP_LOGI(
        TAG,
        "TCP receiver ready");


    // --------------------------------------------------
    // 3. Arm controller
    // --------------------------------------------------

    ArmController arm;


    if (!arm.begin())
    {
        ESP_LOGE(
            TAG,
            "Arm initialization failed");

        return;
    }


    ESP_LOGI(
        TAG,
        "Arm ready");


    ESP_LOGI(
        TAG,
        "Initial shoulder position = %d",
        arm.getShoulderPosition());


    ESP_LOGI(
        TAG,
        "Initial elbow position = %d",
        arm.getElbowPosition());


    // --------------------------------------------------
    // 4. Command tracking
    // --------------------------------------------------

    AetherJointCommand command;


    float lastShoulder = 0.0f;
    float lastElbow = 0.0f;


    bool haveLastCommand = false;

    bool parkedFromTimeout = false;


    TickType_t lastCommandTime =
        xTaskGetTickCount();


    TickType_t lastWakeTime =
        xTaskGetTickCount();


    ESP_LOGI(
        TAG,
        "Waiting for remote joint commands...");


    // --------------------------------------------------
    // 5. Continuous live-control loop
    // --------------------------------------------------

    while (true)
    {
        // --------------------------------------------------
        // Check for newest command
        //
        // 0 means:
        // don't block waiting for one.
        //
        // If nothing is available, continue with the loop.
        // --------------------------------------------------

        if (
            tcp_receiver_get_command(
                &command,
                0
            ) == pdTRUE
        )
        {
            // We received a fresh command.
            lastCommandTime =
                xTaskGetTickCount();


            parkedFromTimeout =
                false;


            // --------------------------------------------------
            // Deadband
            // --------------------------------------------------

            bool shouldSend =
                true;


            if (haveLastCommand)
            {
                const float shoulderChange =
                    std::fabs(
                        command.shoulder_deg
                        - lastShoulder
                    );


                const float elbowChange =
                    std::fabs(
                        command.elbow_deg
                        - lastElbow
                    );


                if (
                    shoulderChange
                        < COMMAND_DEADBAND_DEG
                    &&
                    elbowChange
                        < COMMAND_DEADBAND_DEG
                )
                {
                    shouldSend =
                        false;
                }
            }


            // --------------------------------------------------
            // Send latest target to servos
            // --------------------------------------------------

            if (shouldSend)
            {
                ESP_LOGI(
                    TAG,
                    "==================================");

                ESP_LOGI(
                    TAG,
                    "NEW LIVE TARGET");

                ESP_LOGI(
                    TAG,
                    "Shoulder = %.2f deg",
                    command.shoulder_deg);

                ESP_LOGI(
                    TAG,
                    "Elbow = %.2f deg",
                    command.elbow_deg);

                ESP_LOGI(
                    TAG,
                    "==================================");


                /*
                 * IMPORTANT:
                 *
                 * commandAngles() sends the target
                 * to the servos and immediately returns.
                 *
                 * It does NOT wait for the arm to finish.
                 */
                if (arm.commandAngles(
                        command.shoulder_deg,
                        command.elbow_deg))
                {
                    lastShoulder =
                        command.shoulder_deg;


                    lastElbow =
                        command.elbow_deg;


                    haveLastCommand =
                        true;


                    ESP_LOGI(
                        TAG,
                        "Live target accepted");
                }
                else
                {
                    ESP_LOGE(
                        TAG,
                        "Live target rejected");
                }
            }
        }


        // --------------------------------------------------
        // 6. Command timeout
        // --------------------------------------------------

        if (
            haveLastCommand &&
            !parkedFromTimeout
        )
        {
            const TickType_t elapsed =
                xTaskGetTickCount()
                - lastCommandTime;


            if (
                elapsed >=
                pdMS_TO_TICKS(
                    COMMAND_TIMEOUT_MS)
            )
            {
                ESP_LOGW(
                    TAG,
                    "Command timeout - parking arm");


                /*
                 * park() is intentionally blocking.
                 *
                 * That's okay here because we've already
                 * decided that remote commands have stopped.
                 */
                if (arm.park())
                {
                    ESP_LOGI(
                        TAG,
                        "Arm parked after timeout");


                    parkedFromTimeout =
                        true;


                    haveLastCommand =
                        false;
                }
                else
                {
                    ESP_LOGE(
                        TAG,
                        "Timeout park failed");
                }
            }
        }


        // --------------------------------------------------
        // 7. Keep control loop at approximately 20 Hz
        // --------------------------------------------------

        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(
                CONTROL_PERIOD_MS)
        );
    }
}