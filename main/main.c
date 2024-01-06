#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "VRKernel.h"
#include "WiFiKernel.h"
/* Private function prototypes -----------------------------------------------*/
/*Mutex*/
int CreateMyMutex(void **MutexHandel);
int RemoveMyMutex(void **MutexHandel);
int LockMyMutex(void *MutexHandel);
int UnlockMyMutex(void *MutexHandel);

int PrintHandlerForRead(void *param);
int PrintHandlerForWrite(void *param);
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

    VirtualRegsHandlers_t HandlersForUser0;
    HandlersForUser0.HandForRead = PrintHandlerForRead;
    HandlersForUser0.HandForWrite = PrintHandlerForWrite;
    SetHandlerForReg(SYS_COMPILE_TIME, &HandlersForUser0);
    InitWifiKernel();
    uint32_t TmpRegs = 0;
    while (1)
    {
        ReadVirtualReg(SYS_COMPILE_TIME, &TmpRegs);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
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