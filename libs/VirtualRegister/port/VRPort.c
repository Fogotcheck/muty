#include "VRPort.h"


#if USE_MUTEX == 0
Delay_t *VirtualPortDelay;
#elif USE_MUTEX == 1
CreateMutex_t *VirtualPortCreateMutex;
RemoveMutex_t *VirtualPortRemoveMutex;
LockMutex_t *VirtualPortLockMutex;
UnlockMutex_t *VirtualPortUnlockMutex;
void CreateMutexForAllHandels(void);
#endif

void ErrHadlerGO(uint32_t VirtRegAddress);
void HandlerReadGO(uint32_t VirtRegAddress, uint32_t *VirtualRegValue);
void HandlerWriteGO(uint32_t VirtRegAddress, uint32_t *VirtualRegValue);
int SynchronMemAccess(uint32_t VirtRegAddress);

void RemoveMutexForAllHandels(void);

int InitPortVirtualRegs(VirtualPortConfig_t *VirtualPortConfig)
{
#if USE_MUTEX == 0
    if (VirtualPortConfig->VirtDelay == NULL)
    {
        return -1;
    }
    VirtualPortDelay = VirtualPortConfig->VirtDelay;
    VirtualPortDelay(VIRTUAL_START_PORT_DELAY);
#elif USE_MUTEX == 1
    VirtualPortCreateMutex = VirtualPortConfig->VirtualCreateMutex;
    VirtualPortRemoveMutex = VirtualPortConfig->VirtualRemoveMutex;
    VirtualPortLockMutex = VirtualPortConfig->VirtualLockMutex;
    VirtualPortUnlockMutex = VirtualPortConfig->VirtualUnlockMutex;
    CreateMutexForAllHandels();
#endif

    return 0;
}
#if USE_MUTEX == 1
void CreateMutexForAllHandels(void)
{
    int ret = 0;
    if (VirtualPortCreateMutex == NULL)
    {
        LockAccessVirtMem();
        return;
    }

    for (size_t i = 0; i < VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE; i++)
    {
        VirtHandlers[i].MutexHandel = (void *)&MutexVirtMem[i];
        ret = VirtualPortCreateMutex(&VirtHandlers[i].MutexHandel);
        if (ret)
        {
            VirtualSynchonErrCallback((uint32_t)i);
            return;
        }
    }
}

void RemoveMutexForAllHandels(void)
{
    int ret = 0;
    if (VirtualPortCreateMutex == NULL)
    {
        LockAccessVirtMem();
        return;
    }

    for (size_t i = 0; i < VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE; i++)
    {
        ret = VirtualPortRemoveMutex(&VirtHandlers[i].MutexHandel);
        if (ret)
        {
            VirtualSynchonErrCallback((uint32_t)i);
            return;
        }
        VirtHandlers[i].MutexHandel = NULL;
    }
}
#endif
void DeInitPortVirtualRegs(void)
{
#if USE_MUTEX == 1
    RemoveMutexForAllHandels();
#endif
    memset((uint8_t *)&VirtHandlers[0], 0, sizeof(VirtualRegsHandlers_t) * (VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE));
}

int ReadVirtualReg(uint32_t VirtRegAddress, uint32_t *VirtualRegValue)
{
    if (!VREG_GOOD_ADDR(VirtRegAddress))
    {
        VAL(SYS_ERRORS_COUNT)
        ++;
        return -1;
    }
    HandlerReadGO(VirtRegAddress, VirtualRegValue);
    return 0;
}

int WriteVirtualReg(uint32_t VirtRegAddress, uint32_t *VirtualRegValue)
{
    if (!VREG_GOOD_ADDR(VirtRegAddress))
    {
        VAL(SYS_ERRORS_COUNT)
        ++;
        return -1;
    }
    HandlerWriteGO(VirtRegAddress, VirtualRegValue);
    return 0;
}

void HandlerWriteGO(uint32_t VirtRegAddress, uint32_t *VirtualRegValue)
{
    uint32_t ret = 0;
    VAL(VirtRegAddress) = *VirtualRegValue;
    if (VirtHandlers[VirtRegAddress].HandForWrite != NULL)
    {
        if (SynchronMemAccess(VirtRegAddress))
        {
            return;
        }
        VirtHandlers[VirtRegAddress].state.bit.write = 1;

        ret = VirtHandlers[VirtRegAddress].HandForWrite((void *)&VAL(VirtRegAddress));
        if (ret)
        {
            ErrHadlerGO(VirtRegAddress);
            return;
        }
        VirtHandlers[VirtRegAddress].state.bit.write = 0;
#if USE_MUTEX == 1
        ret = VirtualPortUnlockMutex(VirtHandlers[VirtRegAddress].MutexHandel);
        if (ret)
        {
            ErrHadlerGO(VirtRegAddress);
        }
#endif
    }
    
}

void LockAccessVirtMem(void)
{
    for (size_t i = 0; i < VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE; i++)
    {
        VirtHandlers[i].state.bit.lock = 1;
    }
}

void UnLockAccessVirtMem(void)
{
    for (size_t i = 0; i < VIRTUAL_MEMORY_SIZE / VIRT_REG_USER_WORD_SIZE; i++)
    {
        VirtHandlers[i].state.bit.lock = 0;
#if USE_MUTEX == 1
        VirtualPortUnlockMutex(VirtHandlers[i].MutexHandel);
#endif
    }
}

void HandlerReadGO(uint32_t VirtRegAddress, uint32_t *VirtualRegValue)
{
    uint32_t ret = 0;
    *VirtualRegValue = VAL(VirtRegAddress);
    if (VirtHandlers[VirtRegAddress].HandForRead != NULL)
    {
        if (SynchronMemAccess(VirtRegAddress))
        {
            return;
        }
        VirtHandlers[VirtRegAddress].state.bit.read = 1;
        ret = VirtHandlers[VirtRegAddress].HandForRead((void *)&VAL(VirtRegAddress));
        if (ret)
        {
            ErrHadlerGO(VirtRegAddress);
        }
        VirtHandlers[VirtRegAddress].state.bit.read = 0;
#if USE_MUTEX == 1
        ret = VirtualPortUnlockMutex(VirtHandlers[VirtRegAddress].MutexHandel);
        if (ret)
        {
            ErrHadlerGO(VirtRegAddress);
        }
#endif
    }
    
}

#if USE_MUTEX == 0
int SynchronMemAccess(uint32_t VirtRegAddress)
{
    uint32_t ErrCount = MAX_WAIT_COUNT_DELAY;
    while (VirtHandlers[VirtRegAddress].state.byte)
    {
        VirtualPortDelay(USER_DELAY_FOR_READ);
        ErrCount--;
    }
    if (ErrCount == 0)
    {
        VirtHandlers[VirtRegAddress].state.bit.Error = 1;
        VirtHandlers[VirtRegAddress].state.bit.lock = 1;
        VirtualSynchonErrCallback(VirtRegAddress);
        return -1;
    }
    return 0;
}
#elif USE_MUTEX == 1
int SynchronMemAccess(uint32_t VirtRegAddress)
{
    return VirtualPortLockMutex(VirtHandlers[VirtRegAddress].MutexHandel);
}
#endif

void ErrHadlerGO(uint32_t VirtRegAddress)
{
    VAL(SYS_ERRORS_COUNT)
    ++;
    VirtHandlers[VirtRegAddress].state.bit.Error = 1;
    if (VirtHandlers[VirtRegAddress].ErrHandler == NULL)
    {
        return;
    }
    VirtHandlers[VirtRegAddress].ErrHandler(&VAL(VirtRegAddress));
    VirtHandlers[VirtRegAddress].state.bit.Error = 0;
#if USE_MUTEX == 1
    VirtualPortUnlockMutex(VirtHandlers[VirtRegAddress].MutexHandel);
#endif
}

__attribute__((weak)) void VirtualSynchonErrCallback(__attribute__((unused)) uint32_t VirtRegAddress)
{
    return;
}
