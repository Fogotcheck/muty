#ifndef __VirtualMemory_h__
#define __VirtualMemory_h__
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include "VRTypes.h"

extern VirtualMemory_t VirtMem;
extern VirtualRegsHandlers_t VirtHandlers[];

#if USE_MUTEX ==1
extern MutexWordSize_t MutexVirtMem[];
#endif

int InitVirtualMemoryBuf(VirtualMemoryConfig_t *VirtualMemConfig);
void DeInitVirtualMemoryBuf(void);
#endif //__VirtualMemory_h__