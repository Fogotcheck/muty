#ifndef __VirtualTypes_h__
#define __VirtualTypes_h__
#include <stdint.h>

#include "VRConfig.h"

typedef void *(VirtualMalloc_t)(size_t);
typedef void(VirtualFree_t)(void *);

typedef struct VirtualMemoryConfig
{
#if VIRTUAL_MEMORY_TYPE == 0
    void *pBufReg;
#elif VIRTUAL_MEMORY_TYPE == 1
    VirtualMalloc_t *VirtMalloc;
    VirtualFree_t *VirtFree;
#endif
    size_t BufRegSize;
} VirtualMemoryConfig_t;

typedef struct VirtualMemory
{
    volatile uint8_t *VirtMem;
    size_t VirtMemSize;
#if VIRTUAL_MEMORY_TYPE == 1
    VirtualMalloc_t *VirtMalloc;
    VirtualFree_t *VirtFree;
#endif
} VirtualMemory_t;

typedef int(Handlers_t)(void *);
typedef void(ErrHandlers_t)(void *);

#if MUTEX_WORD_SIZE == 4
typedef uint32_t MutexWordSize_t;
#elif MUTEX_WORD_SIZE == 8
typedef uint64_t MutexWordSize_t;
#endif

#pragma pack(push, 1)
typedef struct HandlersFlag
{
    uint8_t read : 1;
    uint8_t write : 1;
    uint8_t Error : 1;
    uint8_t lock : 1;
    uint8_t free_4 : 1;
    uint8_t free_5 : 1;
    uint8_t free_6 : 1;
    uint8_t free_7 : 1;
} HandlersFlag_t;

typedef union HandlersFlagU8
{
    HandlersFlag_t bit;
    uint8_t byte;
} HandlersFlagU8_t;

typedef struct VirtualRegsHandlers
{
    Handlers_t *HandForRead;
    Handlers_t *HandForWrite;
    ErrHandlers_t *ErrHandler;
    void *MutexHandel;
    HandlersFlagU8_t state;
} VirtualRegsHandlers_t;
#pragma pack(pop)

#if USE_MUTEX == 0
typedef void(Delay_t)(const uint32_t);
#elif USE_MUTEX == 1
typedef int(CreateMutex_t)(void **);
typedef int(RemoveMutex_t)(void **);
typedef int(LockMutex_t)(void *);
typedef int(UnlockMutex_t)(void *);
#endif

typedef struct VirtualPortConfig
{
#if USE_MUTEX == 0
    Delay_t *VirtDelay;
#elif USE_MUTEX == 1
    CreateMutex_t *VirtualCreateMutex;
    RemoveMutex_t *VirtualRemoveMutex;
    LockMutex_t *VirtualLockMutex;
    UnlockMutex_t *VirtualUnlockMutex;
#endif
} VirtualPortConfig_t;

typedef struct VirtualArgHandler
{
    uint32_t Address;
    void *Data;
    size_t DataSize;
} VirtualArgHandler_t;

#endif //__VirtualTypes_h__