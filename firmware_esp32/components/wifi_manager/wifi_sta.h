#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize station mode and block until the configured access point assigns
 * an IP address, or until the retry limit is reached.
 */
esp_err_t wifi_sta_start(void);

#ifdef __cplusplus
}
#endif
