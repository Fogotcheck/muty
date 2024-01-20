#ifndef __CMotorConfig_h__
#define __CMotorConfig_h__

#define MOTOR_PIN_AIN1 37
#define MOTOR_PIN_AIN2 38
#define MOTOR_PIN_BIN1 40
#define MOTOR_PIN_BIN2 41

#define MOTOR_PIN_PWMA 36
#define MOTOR_PIN_PWMB 42

#define DUTY_START 0
#define MAX_MOTOR_VALUE 8192  /* LEDC_TIMER_13_BIT // Set duty resolution to 13 bits */
#define MIN_MOTOR_VALUE -8192 /* Set duty to 50%. (2 ** 13) * 50% = 4096 */

#define MOTOR_LEFT 1
#define MOTOR_RIGHT 0
#define MOTOR_REVERS_OFF 0
#define MOTOR_REVERS_ON 1

#endif //__CMotorConfig_h__