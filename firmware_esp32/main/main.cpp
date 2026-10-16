#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "arm_controller.h"


static const char *TAG = "MAIN";


// --------------------------------------------------
// One joint-angle waypoint
// --------------------------------------------------

struct JointPoint
{
    const char *name;

    float shoulderDegrees;
    float elbowDegrees;
};


// --------------------------------------------------
// Known-good test path
// --------------------------------------------------

static constexpr JointPoint PATH[] =
{
    {
        "Point 1",
        8.4f,
        -15.0f
    },

    {
        "Point 2",
        18.5f,
        -34.0f
    },

    {
        "Point 3",
        28.6f,
        -45.3f
    }
};


static constexpr int PATH_LENGTH =
    sizeof(PATH) / sizeof(PATH[0]);


extern "C" void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting ArmController multi-angle test");


    ArmController arm;


    // --------------------------------------------------
    // Initialize arm
    // --------------------------------------------------

    if (!arm.begin())
    {
        ESP_LOGE(
            TAG,
            "Arm initialization failed");

        return;
    }


    ESP_LOGI(
        TAG,
        "Initial shoulder position = %d",
        arm.getShoulderPosition());

    ESP_LOGI(
        TAG,
        "Initial elbow position = %d",
        arm.getElbowPosition());


    ESP_LOGI(
        TAG,
        "Test begins in 3 seconds");


    vTaskDelay(
        pdMS_TO_TICKS(3000));


    // --------------------------------------------------
    // Execute angle path
    // --------------------------------------------------

    for (
        int i = 0;
        i < PATH_LENGTH;
        i++)
    {
        ESP_LOGI(
            TAG,
            "==================================");

        ESP_LOGI(
            TAG,
            "Moving to %s",
            PATH[i].name);

        ESP_LOGI(
            TAG,
            "Shoulder angle = %.2f deg",
            PATH[i].shoulderDegrees);

        ESP_LOGI(
            TAG,
            "Elbow angle = %.2f deg",
            PATH[i].elbowDegrees);


        if (!arm.moveToAngles(
                PATH[i].shoulderDegrees,
                PATH[i].elbowDegrees))
        {
            ESP_LOGE(
                TAG,
                "Movement to %s failed",
                PATH[i].name);

            return;
        }


        ESP_LOGI(
            TAG,
            "%s reached",
            PATH[i].name);

        ESP_LOGI(
            TAG,
            "Shoulder readback = %d",
            arm.getShoulderPosition());

        ESP_LOGI(
            TAG,
            "Elbow readback = %d",
            arm.getElbowPosition());


        // Pause so movement is easy to observe
        vTaskDelay(
            pdMS_TO_TICKS(1500));
    }


    // --------------------------------------------------
    // Park vertically
    // --------------------------------------------------

    ESP_LOGI(
        TAG,
        "==================================");

    ESP_LOGI(
        TAG,
        "Angle path complete");

    ESP_LOGI(
        TAG,
        "Parking arm vertically");


    if (!arm.park())
    {
        ESP_LOGE(
            TAG,
            "Failed to park arm");

        return;
    }


    ESP_LOGI(
        TAG,
        "==================================");

    ESP_LOGI(
        TAG,
        "MULTI-ANGLE TEST COMPLETE");

    ESP_LOGI(
        TAG,
        "Final shoulder = %d",
        arm.getShoulderPosition());

    ESP_LOGI(
        TAG,
        "Final elbow = %d",
        arm.getElbowPosition());

    ESP_LOGI(
        TAG,
        "Arm is parked vertically");

    ESP_LOGI(
        TAG,
        "==================================");


    while (true)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }
}