#ifndef __WiFiKernel_h__
#define __WiFiKernel_h__
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "lwip/inet.h"
#include "FreeRTOS/freertos.h"
#include "freertos/event_groups.h"

esp_err_t InitWifiKernel(void);
#endif //__WiFiKernel_h__