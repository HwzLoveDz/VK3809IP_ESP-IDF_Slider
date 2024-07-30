/**
 * @file powerSaveInt0Key1Slider.cpp
 * @author by mondraker (https://oshwhub.com/mondraker)(https://github.com/HwzLoveDz)
 * @brief Reading the value of the slider uses the interrupt trigger method
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

static void slider_hander_task(void *);
static QueueHandle_t  gpio_evt_queue = NULL;

static void custum_slider_setting();

static void IRAM_ATTR slider_irq_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void irq_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_set_intr_type(VK_ISR_GPIO, GPIO_INTR_NEGEDGE);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(VK_ISR_GPIO, slider_irq_handler, (void *) VK_ISR_GPIO);
}

uint8_t scaleTo255(uint8_t value) {
    if (value > 227) {
        value = 227;
    }
    return (uint8_t)(((float)value / 227.0) * 255.0);
}

static void custum_slider_setting(){
    // Setting commands
    uint8_t settingDataByte1 = slider.settingCommandsDataByte1(
        SLIDE_APP_MODE,
        SETING_COMMANDS,
        SINGLE,
        AUTO_ADJUST_ENABALE,
        POWER_SAVE_ENABALE,
        DYNAMIC_THRESHOLD_DISABLE,
        AOTO_RESTET_TIME_15S
        );
    uint8_t settingDataByte2 = slider.settingCommandsDataByte2(
        KEY_NUM_0_DISABLE,
        KEY_ACK_TIME_4
        );
    uint8_t settingDataByte3 = slider.settingCommandsDataByte3(
        SLIDE_X_NUM_DISABLE,
        SLIDE_X_NUM_9
        );
    uint8_t settingDataByte4 = slider.settingCommandsDataByte4(
        KEY_OFF_NUM_1_DISABLE,
        SLIDE_X_NUM_DISABLE
        );
    slider.settingCommandsData(settingDataByte1, settingDataByte2, settingDataByte3, settingDataByte4);
}

extern "C" void app_main(void)
{
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(VK_ISR_GPIO, sizeof(uint32_t));

    // Register slider interrupt pins
    irq_init();

    ESP_ERROR_CHECK(i2c_master_init()); //初始化I2C

    if (slider.begin(twi_read, twi_write, VK3809IP_ADDR)) // 初始化芯片
    {
        ESP_LOGE(TAG, "Error init vk3809ip !!!");
        for(;;)
            ;
    }
    ESP_LOGI(TAG, "Success init vk3809ip !!!");

    custum_slider_setting();    // 用户自定义设置,必须放在这里

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

    xTaskCreate(slider_hander_task, "App/pwr", 4 * 1024, NULL, 10, NULL);
}

static void slider_hander_task(void *args)
{
    uint32_t io_num;
    for(;;) 
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) 
        {
            if (slider.getSliderPressedState(SLIDE_1_TOUCH_STATE))
            {
                static uint8_t afterValue = 0;
                uint8_t beforeValue = slider.getSliderData(SLIDE_1_POSITION);
                if (afterValue != beforeValue)
                {
                    afterValue = beforeValue;
                    // printf("Slider position(0-227): %.3d\n", afterValue);
                    printf("Slider position(0-255):: %.3d\n", scaleTo255(afterValue));
                }
            }
        }
    }
}
