#ifndef __CMotor_h__
#define __CMotor_h__
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

esp_err_t InitCMotor(void);
int UpdateValLMotor(void *param);
int UpdateValRMotor(void *param);
#endif //__CMotor_h__