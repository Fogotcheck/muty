#ifndef __VirtualRegsPort_h__
#define __VirtualRegsPort_h__
#include <stdint.h>

#include "VRConfig.h"
#include "VRMap.h"

#include "VRMem.h"
#include "VRKernel.h"
#include "VRTypes.h"
#define VIRTUAL_START_PORT_DELAY 20

int InitPortVirtualRegs(VirtualPortConfig_t *VirtualPortConfig);
int ReadVirtualReg(uint32_t VirtRegAddress,uint32_t *VirtualRegValue);
int WriteVirtualReg(uint32_t VirtRegAddress, uint32_t *VirtualRegValue);
void LockAccessVirtMem(void);
void UnLockAccessVirtMem(void);
void VirtualSynchonErrCallback(uint32_t VirtRegAddress);
void DeInitPortVirtualRegs(void);
#endif //__VirtualRegsPort_h__
