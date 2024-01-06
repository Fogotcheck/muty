#ifndef __VirtualRegConfig_h__
#define __VirtualRegConfig_h__

#define VIRTUAL_MEMORY_TYPE 0   /* 0=static mem, 1=usermalloc */
#define VIRTUAL_MEMORY_SIZE 128 /* size in byte for all regs */
#define VIRT_REG_USER_WORD_SIZE 4        /* reg size */
#define USE_MUTEX 1             /* 0=mutex off, 1= mutex on */
#if USE_MUTEX == 1
#define MUTEX_WORD_SIZE 4 /* pthread_mutex_t=long long,SemaphoreHandle_t=4 */
#endif
#if USE_MUTEX == 0              /* if mutex of synchro delay */
#define MAX_WAIT_COUNT_DELAY 10 /* number of iterations */
#define USER_DELAY_FOR_READ 100 /* time of each iteration */
#endif
// #define DEBUG_PRINT_EN /* print memory contents before cleaning */
#endif                 // VirtualMemoryConfig_t