#include "MqttClient.h"
#include "WifiConfig.h"
#include "VRKernel.h"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void PubSubRegThread(void *arg);

void SetTopicInString(char **TopicName, char **TopicVal, uint16_t NumReg);

char TopicNameBuf[32];
char TopicValBuf[32];
EventGroupHandle_t PubSubEvent;
esp_mqtt_client_handle_t client;
uint32_t PubSubCount = 0;

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
    printf("Mqtt client start\r\n");
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
    printf("Mqtt conf::\r\nlimit:\t%llu,out_size\t%d,size::\t%d\r\n", mqtt_cfg.outbox.limit, mqtt_cfg.buffer.out_size, mqtt_cfg.buffer.size);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    PubSubEvent = xEventGroupCreate();
    xTaskCreate(PubSubRegThread, "PubSubTask", 4096, NULL, 5, NULL);

    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        char *TopicName = NULL;
        char *TopicValue = NULL;

        SetTopicInString(&TopicName, &TopicValue, PubSubCount);
        if ((TopicName == NULL) || (TopicValue == NULL))
        {
            break;
        }

        esp_mqtt_client_publish(client, TopicName, TopicValue, 0, 1, 0);
        esp_mqtt_client_subscribe(client, TopicName, 0);

        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        PubSubCount = 0;
        break;
    case MQTT_EVENT_SUBSCRIBED:
        PubSubCount += 4;
        xEventGroupSetBits(PubSubEvent, CLIENT_SUB_EVENT_BIT);
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        PubSubCount = 0;
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
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

    __itoa(NumReg, tmp, 16);
    strcat(TopicNameBuf, tmp);

    *TopicName = TopicNameBuf;
    *TopicVal = TopicValBuf;
}

void PubSubRegThread(void *arg)
{
    char *TopicName = NULL;
    char *TopicValue = NULL;
    while (1)
    {
        xEventGroupWaitBits(PubSubEvent, CLIENT_SUB_EVENT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

        SetTopicInString(&TopicName, &TopicValue, PubSubCount);
        if ((TopicName == NULL) || (TopicValue == NULL))
        {
            PubSubCount = 0;
            continue;
        }

        esp_mqtt_client_publish(client, TopicName, TopicValue, 0, 1, 0);
        esp_mqtt_client_subscribe(client, TopicName, 0);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
