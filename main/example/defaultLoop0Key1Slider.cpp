/**
 * @file defaultLoop0Key1Slider.cpp
 * @author by mondraker (https://oshwhub.com/mondraker)(https://github.com/HwzLoveDz)
 * @brief The value of the slider is read using the normal loop method
 * The nine buttons are configured as a set of sliders with zero individual buttons
 * @version 0.1
 * @date 2024-07-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
//! Warnings: In hardware design, the slider must be independent and form a closed loop at both ends to obtain the correct values.

#include "vk3809ip.hpp"

extern "C"
{
    #include "i2c_port.h"
}

static const char *TAG = "main";

uint8_t scaleTo255(uint8_t value) {
    if (value > 227) {
        value = 227;
    }
    return (uint8_t)(((float)value / 227.0) * 255.0);
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init()); //初始化I2C

    if (slider.begin(twi_read, twi_write, VK3809IP_ADDR)) // 初始化芯片
    {
        ESP_LOGE(TAG, "Error init vk3809ip !!!");
        for(;;)
            ;
    }
    ESP_LOGI(TAG, "Success init vk3809ip !!!");

    if ((slider.getSystemCorrectionFlagState() == 1 && slider.getSystemWriteFlagState() != 1) == 0) // 系统校正标志与系统写入标志
    {
        for(;;)
        {
            ESP_LOGE(TAG, "Waitting for config vk3809ip !!!");
            vTaskDelay(pdMS_TO_TICKS(50));
            if ((slider.getSystemCorrectionFlagState() == 1 && slider.getSystemWriteFlagState() != 1) == 1){break;}
        }
    }
    ESP_LOGI(TAG, "Success write setting vk3809ip !!!");

    for(;;)
    {
        if (slider.getSliderPressedState(SLIDE_1_TOUCH_STATE))
        {
            static uint8_t afterValue = 0;
            uint8_t beforeValue = slider.getSliderData(SLIDE_1_POSITION);
            if (afterValue != beforeValue)
            {
                afterValue = beforeValue;
                printf("Slider position(0-227): %.3d\n", afterValue);
            // printf("Slider position(0-255):: %.3d\n", scaleTo255(afterValue));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
