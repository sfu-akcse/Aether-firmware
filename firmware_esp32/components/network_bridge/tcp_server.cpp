/**
 * @file      tcp_server.cpp
 * @brief     FreeRTOS Network Task for handling Host PC communication.
 *
 * @author    [Your Name]
 * @date      2026-07-16
 *
 * @details   This file contains the socket listener that receives JSON 
 * coordinate payloads from the Host PC's Python script and 
 * pushes them to the Motor Control queue.
 *
 * @note      Hardware Dependency: Requires Wi-Fi to be connected via 
 * wifi_manager before start_tcp_listener_task() is called.
 */
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

// standard LwIP Socket libraries
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "tcp_server.h"