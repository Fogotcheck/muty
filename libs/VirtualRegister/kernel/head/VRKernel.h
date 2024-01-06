#ifndef __VirtualRegsMapHandler_h__
#define __VirtualRegsMapHandler_h__
#include <stdio.h>
#include <string.h>
#include "VRMem.h"
#include "VRMap.h"
#include "VRPort.h"
#include "VRTypes.h"

#define VREG_GOOD_ALIGN(x) (!((x) & (3)))
#define VREG_GOOD_RANGE(x) ((x) < (VirtMem.VirtMemSize) ? 1 : 0)

#define VREG_GOOD_ADDR(x) (VREG_GOOD_RANGE(x) && VREG_GOOD_ALIGN(x))

// todo VAL(addr) (*(uint32_t *) - change wor user type
#define VAL(addr) (*(uint32_t *)(VirtMem.VirtMem + (uint32_t)(addr)))

int InitKernalRegsMap(VirtualMemoryConfig_t *VirtualMemConfig, VirtualPortConfig_t *VirtualPortConfigure);
void DeInitKernalRegsMap(void);
int SetHandlerForReg(uint32_t RegAddress,VirtualRegsHandlers_t *Handlers);
int GetHandlerForReg(uint32_t RegAddress, VirtualRegsHandlers_t **Handlers);
#endif //__VirtualRegsMapHandler_h__
