#include "VRKernel.h"

void SetCompileTime(char *t, char *d);

int InitKernalRegsMap(VirtualMemoryConfig_t *VirtualMemConfig, VirtualPortConfig_t *VirtualPortConfigure)
{
    if (InitVirtualMemoryBuf(VirtualMemConfig))
    {
        return -1;
    }
    if (InitPortVirtualRegs(VirtualPortConfigure))
    {
        return -1;
    }

    VAL(SYS_MEM_TYPE) = VIRTUAL_MEMORY_TYPE;
    VAL(SYS_MEM_SIZE) = VirtMem.VirtMemSize;
    SetCompileTime(__TIME__, __DATE__);
    return 0;
}

int SetHandlerForReg(uint32_t RegAddress, VirtualRegsHandlers_t *Handlers)
{
    if ((VirtHandlers[COUNT(RegAddress)].HandForRead == NULL) &&
        (VirtHandlers[COUNT(RegAddress)].HandForWrite == NULL) &&
        (VirtHandlers[COUNT(RegAddress)].ErrHandler == NULL))
    {
        VAL(SYS_HANDLERS_COUNT)++;
    }

    VirtHandlers[COUNT(RegAddress)].HandForRead = Handlers->HandForRead;
    VirtHandlers[COUNT(RegAddress)].HandForWrite = Handlers->HandForWrite;
    VirtHandlers[COUNT(RegAddress)].ErrHandler = Handlers->ErrHandler;
    return 0;
}

int GetHandlerForReg(uint32_t RegAddress, VirtualRegsHandlers_t **Handlers)
{
    *Handlers = &VirtHandlers[COUNT(RegAddress)];
    return VAL(SYS_HANDLERS_COUNT);
}

void SetCompileTime(char *t, char *d)
{
#define __MONTH__(date) (                             \
    date[2] == 'n'   ? (date[1] == 'a' ? "01" : "06") \
    : date[2] == 'b' ? "02"                           \
    : date[2] == 'r' ? (date[0] == 'M' ? "03" : "04") \
    : date[2] == 'y' ? "05"                           \
    : date[2] == 'l' ? "07"                           \
    : date[2] == 'g' ? "08"                           \
    : date[2] == 'p' ? "09"                           \
    : date[2] == 't' ? "10"                           \
    : date[2] == 'v' ? "11"                           \
                     : "12")

    char day[3], month[3], year[3], hour[3];
    int DayNum, MonthNum, YearNum, HourNum;

    // "10"
    hour[0] = t[0];
    hour[1] = t[1];
    hour[2] = 0;

    day[0] = d[4];
    day[1] = d[5];
    day[2] = 0;

    strcpy(month, __MONTH__(d));

    year[0] = d[9];
    year[1] = d[10];
    year[2] = 0;

    // "10" => 0x10
    sscanf(hour, "%x", &HourNum);
    sscanf(day, "%x", &DayNum);
    sscanf(month, "%x", &MonthNum);
    sscanf(year, "%x", &YearNum);

    VAL(SYS_COMPILE_TIME) = (HourNum << 8 | DayNum);
    VAL(SYS_COMPILE_TIME) <<= 16;
    VAL(SYS_COMPILE_TIME) |= (MonthNum << 8 | YearNum);
}

void DeInitKernalRegsMap(void)
{
    DeInitPortVirtualRegs();
    DeInitVirtualMemoryBuf();
}