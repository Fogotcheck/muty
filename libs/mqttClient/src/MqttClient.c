#include "MqttClient.h"
#include "WifiConfig.h"
#include "VRKernel.h"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void SetTopicInString(char **TopicName, char **TopicVal, uint16_t NumReg);
void WriteReadPubInReg(esp_mqtt_event_handle_t event, uint32_t *NumReg);
void UpdqteAndSubAllReg(void);
void UpdateReg(uint32_t NumReg);
void UpdateAllReg(void);
char TopicNameBuf[32];
char TopicValBuf[32];

esp_mqtt_client_handle_t client;

static const char *TAG = "MQTT";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

esp_err_t InitMqttClient(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MOSQUITTO_SERVER_URL,
        .broker.address.port = 1883,
        .credentials.username = ESP32_CLIENT_NAME,
        .credentials.authentication.password = ESP_WIFI_PASS,
        .buffer.out_size = 4096,
        .buffer.size = 4096,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL)
    {
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    uint32_t NumReg = 0;
    static uint8_t SwitchFlagNeedUpdate = 0;
    static uint32_t ErrCount = 0;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        SwitchFlagNeedUpdate = 0;
        UpdqteAndSubAllReg();
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        SwitchFlagNeedUpdate = 0;
        ErrCount = 0;
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        if (SwitchFlagNeedUpdate == 0)
        {
            WriteReadPubInReg(event, &NumReg);
            UpdateReg(NumReg);
            SwitchFlagNeedUpdate = 1;
            break;
        }
        else
        {
            ErrCount++;
        }
        if (ErrCount >= CLIENT_NO_PUB_ERR)
        {
            SwitchFlagNeedUpdate = 0;
        }

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void SetTopicInString(char **TopicName, char **TopicVal, uint16_t NumReg)
{
    *TopicName = *TopicVal = NULL;

    char tmp[32] = {0};
    memset(TopicNameBuf, 0, sizeof(TopicNameBuf));
    memset(TopicValBuf, 0, sizeof(TopicValBuf));
    memcpy(TopicNameBuf, TOPIC_DEFAULT_VAL, sizeof(TOPIC_DEFAULT_VAL));

    uint32_t RegVal = 0;
    if (ReadVirtualReg(NumReg, &RegVal))
    {
        return;
    }

    __itoa(RegVal, tmp, 16);
    memcpy(TopicValBuf, tmp, strlen(tmp));
    memset(tmp, 0, sizeof(tmp));

    __itoa(NumReg, tmp, 10);
    strcat(TopicNameBuf, tmp);

    *TopicName = TopicNameBuf;
    *TopicVal = TopicValBuf;
}

void WriteReadPubInReg(esp_mqtt_event_handle_t event, uint32_t *NumReg)
{
    char TmpBuf[(sizeof(TOPIC_DEFAULT_VAL) << 1)] = {0};
    memcpy(TmpBuf, event->topic, event->topic_len);
    if (strstr(TmpBuf, TOPIC_DEFAULT_VAL) == NULL)
    {
        return;
    }

    *NumReg = atoi(&TmpBuf[sizeof(TOPIC_DEFAULT_VAL) - 1]);

    memset(TmpBuf, 0, sizeof(TmpBuf));
    memcpy(TmpBuf, event->data, event->data_len);
    uint32_t RegVal = atoi(TmpBuf);

    WriteVirtualReg(*NumReg, &RegVal);
}

void UpdqteAndSubAllReg(void)
{
    char *TopicName = NULL;
    char *TopicValue = NULL;
    uint32_t RegSize = 0;
    ReadVirtualReg(SYS_MEM_SIZE, &RegSize);

    for (uint16_t i = 0; i < RegSize; i++)
    {
        SetTopicInString(&TopicName, &TopicValue, i);
        if ((TopicName == NULL) || (TopicValue == NULL))
        {
            continue;
        }

        esp_mqtt_client_publish(client, TopicName, TopicValue, 0, 0, 0);
        esp_mqtt_client_subscribe(client, TopicName, 2);
    }
}

void UpdateAllReg(void)
{
    uint32_t RegSize = 0;
    ReadVirtualReg(SYS_MEM_SIZE, &RegSize);

    for (uint16_t i = 0; i < RegSize; i++)
    {
        UpdateReg(i);
    }
}

void UpdateReg(uint32_t NumReg)
{
    char *TopicName = NULL;
    char *TopicValue = NULL;
    SetTopicInString(&TopicName, &TopicValue, NumReg);
    if ((TopicName == NULL) || (TopicValue == NULL))
    {
        return;
    }
    esp_mqtt_client_publish(client, TopicName, TopicValue, 0, 2, 0);
}