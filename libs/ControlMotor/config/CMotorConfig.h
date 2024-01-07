#ifndef __CMotorConfig_h__
#define __CMotorConfig_h__

#define MOTOR_PIN_AIN1 20
#define MOTOR_PIN_AIN2 19
#define MOTOR_PIN_BIN1 18
#define MOTOR_PIN_BIN2 17

#define MOTOR_PIN_PWMA 35
#define MOTOR_PIN_PWMB 36

#define DUTY_START 0
#define MAX_MOTOR_VALUE 8192 /* LEDC_TIMER_13_BIT // Set duty resolution to 13 bits */
#define MIN_MOTOR_VALUE -8192 /* Set duty to 50%. (2 ** 13) * 50% = 4096 */
#define MOTOR_MASK_13BIT 0x7FFF

#define MOTOR_LEFT 1
#define MOTOR_RIGHT 0
#define MOTOR_REVERS_OFF 0
#define MOTOR_REVERS_ON 1


#endif //__CMotorConfig_h__