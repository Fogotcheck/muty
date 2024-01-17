#include "MqttClient.h"
#include "WifiConfig.h"
#include "VRKernel.h"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void MqttPubSubThread(void *arg);
void MqttPeriodHandlerThread(void *arg);
int GetCountVirtRegs(uint32_t *CountRegs);
int MqttCheckData(char *data, int DataSize);

static const char *TAG = "MQTT";
QueueHandle_t MqttPeriod;
QueueHandle_t MqttQueuePub;
TaskHandle_t PeriodTaskHandel;
/*todo make local -> PerioHandel Thread used */
esp_mqtt_client_handle_t ClientIit;

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
    ClientIit = esp_mqtt_client_init(&mqtt_cfg);
    if (ClientIit == NULL)
    {
        return ESP_FAIL;
    }
    uint32_t CountRegs = 0;
    if (GetCountVirtRegs(&CountRegs) != 0)
    {
        return ESP_FAIL;
    }

    MqttQueuePub = xQueueCreate((CountRegs << 1), sizeof(MqttPubSubData_t));
    xTaskCreate(MqttPubSubThread, "PubTask", 4096, NULL, 4, NULL);

    esp_mqtt_client_register_event(ClientIit, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(ClientIit);

    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    MqttPubSubData_t MqttPubSubHand = {0};
    MqttPubSubHand.client = client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        uint32_t CountVirtRegs = 0;
        if (GetCountVirtRegs(&CountVirtRegs) != 0)
        {
            break;
        }
        MqttPubSubHand.SubFlag = 1;
        for (uint32_t i = 0; i < CountVirtRegs; i++)
        {
            MqttPubSubHand.NumReg = (i << 2);
            xQueueSend(MqttQueuePub, (void *)&MqttPubSubHand, (TickType_t)10);
        }

        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
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
        break;
    case MQTT_EVENT_DATA:
    {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        if (MqttCheckData(event->data, event->data_len) != 0)
        {
            break;
        }
        uint32_t NumReg = 0;
        uint32_t RegVal = 0;

        sscanf(event->topic, TOPIC_DEFAULT_VAL "0x%08lx", &NumReg);
        sscanf(event->data, "0x%08lx" MQTT_COMMAND_POSTFIX, &RegVal);
        WriteVirtualReg(NumReg, &RegVal);
        MqttPubSubHand.SubFlag = 0;
        MqttPubSubHand.NumReg = NumReg;
        xQueueSend(MqttQueuePub, (void *)&MqttPubSubHand, (TickType_t)10);
    }
    break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            vTaskSuspend(PeriodTaskHandel);
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

int MqttCheckData(char *data, int DataSize)
{
    char TmpBuf[MQTT_TOPIC_NAME_MAX_LEN] = {0};
    if (DataSize > sizeof(TmpBuf))
    {
        return -1;
    }

    memcpy(TmpBuf, data, DataSize);
    if (strstr(TmpBuf, MQTT_COMMAND_POSTFIX) == NULL)
    {
        return -1;
    }
    return 0;
}

void MqttPubSubThread(void *arg)
{
    char TopicName[MQTT_TOPIC_NAME_MAX_LEN] = {0};
    char TopicData[MQTT_TOPIC_NAME_MAX_LEN] = {0};
    MqttPubSubData_t MqttPubSubHand = {0};
    uint32_t ret;

    uint32_t RegVal = 0;
    while (1)
    {
        ret = xQueueReceive(MqttQueuePub, (void *)&MqttPubSubHand, portMAX_DELAY);
        if (ret != pdTRUE)
        {
            continue;
        }

        if (ReadVirtualReg(MqttPubSubHand.NumReg, &RegVal) != 0)
        {
            continue;
        }

        sprintf(TopicName, "%s0x%08lx", TOPIC_DEFAULT_VAL, MqttPubSubHand.NumReg);
        sprintf(TopicData, "0x%08lx%s", RegVal, MQTT_UPDATE_POSTFIX);
        esp_mqtt_client_publish(MqttPubSubHand.client, TopicName, TopicData, 0, 2, 0);

        if (MqttPubSubHand.SubFlag == 1)
        {
            esp_mqtt_client_subscribe(MqttPubSubHand.client, TopicName, 2);
        }
    }
}

int GetCountVirtRegs(uint32_t *CountRegs)
{
    uint32_t SizeVirtRegBuf = 0;

    ReadVirtualReg(SYS_MEM_SIZE, &SizeVirtRegBuf);
    if (SizeVirtRegBuf == 0)
    {
        return -1;
    }
    *CountRegs = SizeVirtRegBuf / VIRT_REG_USER_WORD_SIZE;
    return 0;
}

int MqttRegHandler(void *param)
{
    uint32_t *val = (uint32_t *)param;
    uint8_t command = (uint8_t)((uint32_t)*val >> 24);
    uint16_t NewPeriod = (uint16_t)*val;
    BaseType_t ret = 0;
    static uint8_t FlagMqttPeriodThread = 0;
    if (FlagMqttPeriodThread == 0)
    {
        MqttPeriod = xQueueCreate(5, sizeof(uint16_t));
        ret = xTaskCreate(MqttPeriodHandlerThread, "MqttPeriodTask", 2048, NULL, 4, &PeriodTaskHandel);
        if (ret != pdPASS)
        {
            return -1;
        }
        FlagMqttPeriodThread = 1;
        ret = xQueueSend(MqttPeriod, &NewPeriod, 5);
        if (ret != pdTRUE)
        {
            return -1;
        }
        return 0;
    }
    switch (command)
    {
    case MQTT_START_THREAD:
    {
        vTaskResume(PeriodTaskHandel);
        ret = xQueueSend(MqttPeriod, &NewPeriod, 5);
        if (ret != pdTRUE)
        {
            return -1;
        }
        break;
    }
    case MQTT_SUSPEND_THREAD:
    {
        vTaskSuspend(PeriodTaskHandel);
        break;
    }
    case MQTT_CHANGE_PERIOD:
    {
        ret = xQueueSend(MqttPeriod, &NewPeriod, 5);
        if (ret != pdTRUE)
        {
            return -1;
        }
        break;
    }
    default:
        return -1;
    }

    return 0;
}

void MqttPeriodHandlerThread(void *arg)
{
    uint16_t period = MQTT_DEFAULT_PERIOD_THREAD;
    BaseType_t ret = 0;
    MqttPubSubData_t MqttPubSubHand = {0};
    MqttPubSubHand.client = ClientIit;
    MqttPubSubHand.SubFlag = 0;
    uint32_t CountVirtRegs = 0;
    while (1)
    {
        ret = xQueueReceive(MqttPeriod, &period, (period >> 1) / portTICK_PERIOD_MS);
        if (ret == pdTRUE)
        {
            if (period < MQTT_DEFAULT_PERIOD_THREAD)
            {
                period = MQTT_DEFAULT_PERIOD_THREAD;
            }
        }
        if (GetCountVirtRegs(&CountVirtRegs) != 0)
        {
            break;
        }
        for (uint32_t i = 0; i < CountVirtRegs; i++)
        {
            MqttPubSubHand.NumReg = (i << 2);
            xQueueSend(MqttQueuePub, (void *)&MqttPubSubHand, (TickType_t)10);
        }
        printf("Mqtt thread\r\n");
        vTaskDelay((period >> 1) / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}