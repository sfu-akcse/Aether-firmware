#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    float shoulder_deg;
    float elbow_deg;

} AetherJointCommand;


/*
 * Start background TCP receiver task.
 */
esp_err_t tcp_receiver_start(void);


/*
 * Wait for the latest parsed joint command.
 *
 * Returns pdTRUE when a command was received.
 */
BaseType_t tcp_receiver_get_command(
    AetherJointCommand *command,
    TickType_t ticks_to_wait
);


#ifdef __cplusplus
}
#endif