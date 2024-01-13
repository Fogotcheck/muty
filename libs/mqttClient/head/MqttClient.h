#ifndef __MqttClient_h__
#define __MqttClient_h__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>

#define CLIENT_SUB_EVENT_BIT (1 << 0)
esp_err_t InitMqttClient(void);
#endif //__MqttClient_h__