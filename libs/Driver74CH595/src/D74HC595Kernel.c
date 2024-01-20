#include "D74HC595Kernel.h"

int InitD74HC595(void)
{
    gpio_config_t D74HC595_CS = {};
    D74HC595_CS.intr_type = GPIO_INTR_DISABLE;
    D74HC595_CS.mode = GPIO_MODE_DEF_OUTPUT;
    D74HC595_CS.pin_bit_mask = (1ULL << D74HC595_PIN_CS);
    D74HC595_CS.pull_down_en = 0;
    D74HC595_CS.pull_up_en = 1;
    if (gpio_config(&D74HC595_CS) != ESP_OK)
    {
        return ESP_FAIL;
    }
    gpio_set_level(D74HC595_PIN_CS, 1);

    spi_bus_config_t D74HC595_SPI_cfg={};


    return 0;
}