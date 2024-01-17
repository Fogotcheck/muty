#ifndef __MqttClient_h__
#define __MqttClient_h__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>

#define MOSQUITTO_SERVER_URL "mqtt://192.168.0.108"
#define ESP32_CLIENT_NAME "ESP32_muty"
#define TOPIC_DEFAULT_VAL "/muty/Reg/"
#define MQTT_UPDATE_POSTFIX "_upd"
#define MQTT_COMMAND_POSTFIX "_com"
#define MQTT_TOPIC_NAME_MAX_LEN 32
#define MQTT_DEFAULT_PERIOD_THREAD 10000
typedef struct MqttPubSubData
{
    esp_mqtt_client_handle_t client;
    uint32_t NumReg;
    uint8_t SubFlag;
} MqttPubSubData_t;

enum MQTT_COMMAND_TYPE
{
    MQTT_START_THREAD,
    MQTT_SUSPEND_THREAD,
    MQTT_CHANGE_PERIOD,
};

esp_err_t InitMqttClient(void);
int MqttRegHandler(void *param);
#endif //__MqttClient_h__