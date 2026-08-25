#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Start a background TCP receiver listening on CONFIG_AETHER_TCP_PORT. */
esp_err_t tcp_receiver_start(void);

#ifdef __cplusplus
}
#endif
