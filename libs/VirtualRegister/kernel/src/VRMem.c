#include "VRMem.h"

VirtualMemory_t VirtMem;
VirtualRegsHandlers_t VirtHandlers[VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE];

#if USE_MUTEX == 1
MutexWordSize_t MutexVirtMem[VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE];
#endif

void PrintMem(void);
int InitVirtualMemoryBuf(VirtualMemoryConfig_t *VirtualMemConfig)
{
    if (VirtualMemConfig->BufRegSize != VIRTUAL_MEMORY_SIZE)
    {
        return -1;
    }
#if VIRTUAL_MEMORY_TYPE == 0
    if (VirtualMemConfig->pBufReg == NULL)
    {
        return -1;
    }
    VirtMem.VirtMem = VirtualMemConfig->pBufReg;
#elif VIRTUAL_MEMORY_TYPE == 1
    if ((VirtualMemConfig->VirtMalloc == NULL) || (VirtualMemConfig->VirtFree == NULL))
    {
        return -1;
    }
    VirtMem.VirtMem = (uint8_t *)VirtualMemConfig->VirtMalloc(VirtualMemConfig->BufRegSize);
    VirtMem.VirtFree = VirtualMemConfig->VirtFree;
    VirtMem.VirtMalloc = VirtualMemConfig->VirtMalloc;
#endif
    VirtMem.VirtMemSize = VirtualMemConfig->BufRegSize;
    memset((uint8_t *)VirtMem.VirtMem, 0, VirtMem.VirtMemSize);
    return 0;
}

void DeInitVirtualMemoryBuf(void)
{
#ifdef DEBUG_PRINT_EN
    PrintMem();
#endif
#if VIRTUAL_MEMORY_TYPE == 0
    if (VirtMem.VirtMem != NULL)
    {
        VirtMem.VirtMem = NULL;
    }

#elif VIRTUAL_MEMORY_TYPE == 1
    if (VirtMem.VirtMem != NULL)
    {
        VirtMem.VirtFree((uint8_t *)VirtMem.VirtMem);
    }
    memset(&VirtMem, 0, sizeof(VirtualMemory_t));
#endif
}

void PrintMem(void)
{
    uint32_t *ptr = (uint32_t *)VirtMem.VirtMem;
    uint32_t count = 0;
    while (ptr < (uint32_t *)(VirtMem.VirtMem + VirtMem.VirtMemSize))
    {
        printf("mem[%3lu][%p]::0x%8lx\r\n", count++, ptr, *ptr);
        ptr++;
    }
}