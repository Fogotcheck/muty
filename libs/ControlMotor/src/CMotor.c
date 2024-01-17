#include "CMotor.h"
#include "CMotorConfig.h"

void SetReversMotor(int16_t val, uint8_t motor);
void ChangeReverse(uint8_t motor, uint8_t revers);
int CheckMotorVal(int16_t val);

esp_err_t InitCMotor(void)
{
    gpio_config_t MotorGpioConf = {};
    MotorGpioConf.intr_type = GPIO_INTR_DISABLE;
    MotorGpioConf.mode = GPIO_MODE_DEF_OUTPUT;
    MotorGpioConf.pin_bit_mask = (1ULL << MOTOR_PIN_AIN1) | (1ULL << MOTOR_PIN_AIN2) | (1ULL << MOTOR_PIN_BIN1) | (1ULL << MOTOR_PIN_BIN2);
    MotorGpioConf.pull_down_en = 0;
    MotorGpioConf.pull_up_en = 0;
    if (gpio_config(&MotorGpioConf) != ESP_OK)
    {
        return ESP_FAIL;
    }

    gpio_set_level(MOTOR_PIN_AIN1, 1);
    gpio_set_level(MOTOR_PIN_AIN2, 0);
    gpio_set_level(MOTOR_PIN_BIN1, 1);
    gpio_set_level(MOTOR_PIN_BIN2, 0);

    ledc_timer_config_t MotorRTimer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 8000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&MotorRTimer);

    ledc_channel_config_t MotorRChannel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = MOTOR_PIN_PWMA,
        .duty = DUTY_START, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&MotorRChannel));

    ledc_timer_config_t MotorLTimer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 8000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&MotorLTimer);

    ledc_channel_config_t MotorLChannel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_1,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = MOTOR_PIN_PWMB,
        .duty = DUTY_START, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&MotorLChannel));

    return ESP_OK;
}

int CheckMotorVal(int16_t val)
{
    if ((val > MAX_MOTOR_VALUE) || (val < MIN_MOTOR_VALUE))
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}

void ChangeReverse(uint8_t motor, uint8_t revers)
{
    switch (motor)
    {
    case MOTOR_RIGHT:
        if (revers == MOTOR_REVERS_OFF)
        {
            gpio_set_level(MOTOR_PIN_AIN1, 1);
            gpio_set_level(MOTOR_PIN_AIN2, 0);
        }
        else
        {
            gpio_set_level(MOTOR_PIN_AIN1, 0);
            gpio_set_level(MOTOR_PIN_AIN2, 1);
        }
        break;
    case MOTOR_LEFT:
        if (revers == MOTOR_REVERS_OFF)
        {
            gpio_set_level(MOTOR_PIN_BIN1, 1);
            gpio_set_level(MOTOR_PIN_BIN2, 0);
        }
        else
        {
            gpio_set_level(MOTOR_PIN_BIN1, 0);
            gpio_set_level(MOTOR_PIN_BIN2, 1);
        }
        break;
    default:
        break;
    }
}

void SetReversMotor(int16_t val, uint8_t motor)
{
    static uint8_t CurrntReversLMotor = MOTOR_REVERS_OFF;
    static uint8_t CurrntReversRMotor = MOTOR_REVERS_OFF;
    uint8_t revers = MOTOR_REVERS_OFF;
    switch (motor)
    {
    case MOTOR_RIGHT:
        if (val >= 0)
        {
            if (CurrntReversRMotor == MOTOR_REVERS_OFF)
            {
                return;
            }
            CurrntReversRMotor = MOTOR_REVERS_OFF;
        }
        else
        {
            if (CurrntReversRMotor == MOTOR_REVERS_ON)
            {
                return;
            }
            CurrntReversRMotor = MOTOR_REVERS_ON;
            revers = MOTOR_REVERS_ON;
        }
        break;
    case MOTOR_LEFT:
        if (val >= 0)
        {
            if (CurrntReversLMotor == MOTOR_REVERS_OFF)
            {
                return;
            }
            CurrntReversLMotor = MOTOR_REVERS_OFF;
        }
        else
        {
            if (CurrntReversLMotor == MOTOR_REVERS_ON)
            {
                return;
            }
            CurrntReversLMotor = MOTOR_REVERS_ON;
            revers = MOTOR_REVERS_ON;
        }
        break;
    default:
        return;
    }
    ChangeReverse(motor, revers);
}

int UpdateValLMotor(void *param)
{
    uint32_t *val = (uint32_t *)param;
    int16_t NewDuty = (int16_t)*val;
    if (CheckMotorVal(NewDuty) != ESP_OK)
    {
        return ESP_FAIL;
    }
    SetReversMotor(NewDuty, MOTOR_LEFT);
    if (NewDuty < 0)
    {
        NewDuty *= -1;
    }
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, (uint32_t)NewDuty));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    return ESP_OK;
}

int UpdateValRMotor(void *param)
{
    uint32_t *val = (uint32_t *)param;
    int16_t NewDuty = (int16_t)*val;
    if (CheckMotorVal(NewDuty) != ESP_OK)
    {
        return ESP_FAIL;
    }
    SetReversMotor(NewDuty, MOTOR_RIGHT);
    if (NewDuty < 0)
    {
        NewDuty *= -1;
    }

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint32_t)NewDuty));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    return ESP_OK;
}

void CMotorErrorsHandler(void *RegErr)
{
    uint32_t *val = (uint32_t *)RegErr;
    uint8_t CountErr = (uint8_t)(((uint32_t)*val >> 16));
    CountErr++;
    *val = 0;
    *val |= (0xee << 24) | (CountErr << 16);
}