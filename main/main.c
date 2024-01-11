#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "VRKernel.h"
#include "WiFiKernel.h"
#include "MqttClient.h"
#include "CMotor.h"

/* Private typedef -----------------------------------------------------------*/
#define GPIO_GLED_PIN 38
/* END Private typedef */

/* Private function prototypes -----------------------------------------------*/
/*Mutex*/
int CreateMyMutex(void **MutexHandel);
int RemoveMyMutex(void **MutexHandel);
int LockMyMutex(void *MutexHandel);
int UnlockMyMutex(void *MutexHandel);

int PrintHandlerForRead(void *param);
int PrintHandlerForWrite(void *param);

void LedToggleThread(void *arg);
void TestMotorThread(void *arg);
/* END Private function prototypes */

/* Private variables ---------------------------------------------------------*/
uint8_t RegsMap[VIRTUAL_MEMORY_SIZE];
/*END Private variables */

void app_main(void)
{
    VirtualMemoryConfig_t MemConfig;
    MemConfig.BufRegSize = VIRTUAL_MEMORY_SIZE;
    MemConfig.pBufReg = RegsMap;
    VirtualPortConfig_t PortConfig;

    // PortConfig.VirtDelay=vTaskDelay;
    PortConfig.VirtualCreateMutex = CreateMyMutex;
    PortConfig.VirtualRemoveMutex = RemoveMyMutex;
    PortConfig.VirtualLockMutex = LockMyMutex;
    PortConfig.VirtualUnlockMutex = UnlockMyMutex;
    if (InitKernalRegsMap(&MemConfig, &PortConfig))
    {
        printf("Error::init regs map\r\n");
    }

    VirtualRegsHandlers_t Version;
    Version.HandForRead = PrintHandlerForRead;
    Version.HandForWrite = PrintHandlerForWrite;
    SetHandlerForReg(SYS_COMPILE_TIME, &Version);

    uint32_t version = 0;
    ReadVirtualReg(SYS_COMPILE_TIME, &version);
    if ((InitWifiKernel() != ESP_OK) || (InitMqttClient() != ESP_OK))
    {
        while (1)
        {
            printf("error wifi accert\r\n");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

    InitCMotor();
    VirtualRegsHandlers_t Motor;
    Motor.ErrHandler = CMotorErrorsHandler;
    Motor.HandForRead = PrintHandlerForRead;
    Motor.HandForWrite = UpdateValLMotor;
    SetHandlerForReg(REG_LMOTOR, &Motor);

    Motor.HandForWrite = UpdateValRMotor;
    SetHandlerForReg(REG_RMOTOR, &Motor);

    xTaskCreate(LedToggleThread, "LedTask", 2596, NULL, 5, NULL);
    // xTaskCreate(TestMotorThread, "MotorTask", 2048, NULL, 4, NULL);

    vTaskDelete(NULL);
    while (1)
    {
        printf("error it's will never start\r\n");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void LedToggleThread(void *arg)
{
    uint8_t LedState = 0;
    uint16_t pin = GPIO_GLED_PIN;
    gpio_config_t LedGpioConf = {};
    LedGpioConf.intr_type = GPIO_INTR_DISABLE;
    LedGpioConf.mode = GPIO_MODE_OUTPUT;
    LedGpioConf.pin_bit_mask = (1ULL << pin);
    LedGpioConf.pull_down_en = 0;
    LedGpioConf.pull_up_en = 0;
    gpio_reset_pin(pin);

    if (gpio_config(&LedGpioConf) != ESP_OK)
    {
        printf("Err\r\n");
    }
    while (1)
    {
        gpio_set_level(pin, LedState);
        LedState = LedState > 0 ? 0 : 1;
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

int CreateMyMutex(void **MutexHandel)
{
    SemaphoreHandle_t *handel = (SemaphoreHandle_t *)*MutexHandel;
    *handel = xSemaphoreCreateMutex();
    if (*handel == NULL)
    {
        return 1;
    }
    return 0;
}

int RemoveMyMutex(void **MutexHandel)
{
    SemaphoreHandle_t *handel = (SemaphoreHandle_t *)*MutexHandel;
    vSemaphoreDelete(*handel);
    *MutexHandel = NULL;
    return 0;
}

int LockMyMutex(void *MutexHandel)
{
    SemaphoreHandle_t *handel = (SemaphoreHandle_t *)MutexHandel;

    if (xSemaphoreTake(*handel, portMAX_DELAY) == pdTRUE)
    {
        return 0;
    }
    return 1;
}

int UnlockMyMutex(void *MutexHandel)
{
    SemaphoreHandle_t *handel = (SemaphoreHandle_t *)MutexHandel;
    if (xSemaphoreGive(*handel) == pdTRUE)
    {
        return 0;
    }
    return 0;
}

void TestMotorThread(void *arg)
{
    uint32_t reg = 0;
    int16_t TmpDuty = 0;
    while (1)
    {
        if (TmpDuty > 8192)
        {
            TmpDuty = -8192;
        }
        reg = (uint16_t)TmpDuty;
        uint32_t tmp = 0;
        ReadVirtualReg(REG_LMOTOR, &tmp);
        ReadVirtualReg(REG_RMOTOR, &tmp);
        WriteVirtualReg(REG_LMOTOR, &reg);
        WriteVirtualReg(REG_RMOTOR, &reg);
        TmpDuty += 64;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

int PrintHandlerForRead(void *param)
{
    uint32_t *val = (uint32_t *)param;

    printf("read::0x%8lx\r\n", *val);
    return 0;
}

int PrintHandlerForWrite(void *param)
{
    uint32_t *val = (uint32_t *)param;

    printf("write::0x%8lx\r\n", *val);
    return 0;
}