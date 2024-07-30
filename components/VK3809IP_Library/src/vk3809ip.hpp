/**
 * @file vk3809ip.hpp
 * @author by mondraker (https://oshwhub.com/mondraker)(https://github.com/HwzLoveDz)
 * @brief vk3809ip capacitive touch button slider chip driver library
 * @version 0.1
 * @date 2024-07-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#if ARDUINO >= 100
#include "Arduino.h"
#else
// #include "WProgram.h"
#include <stdint.h>
#include <string.h>
#include <cstdio>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* 
    ! 仅支持Multi Read and Write模式，不支持Single Read and Write模式，给reg_addr赋值0xFF以禁用Single Read and Write.
    ! 在滑条应用模式，写入数据以3~4 Data Bytes为一组资料串流。当一笔数据串流写入完成后，系统会将数据覆写并进行系统重设。
    ! 若写入被中断并重新写入，则前一笔数据会被放弃。在每次写入完一组3~4 Data Bytes后，若要再次写入下一组设定，
    ! 需要送出Stop讯号结束当前数据传输，再重新写入下一组设定。
*/
#define REG_ADDR_NONE 0xFF // datasheet‘P10 Packet Stream’

#define VK3809IP_ADDR 0x53                            // 设备地址
#define VK3809IP_ADDR_READ ((VK3809IP_ADDR << 1) + 1) // 读取寄存器地址 0xA7
#define VK3809IP_ADDR_WRITE (VK3809IP_ADDR << 1)      // 写入寄存器地址 0xA6

#define VK_PASS 1
#define VK_FAIL 0

/**
 * @brief IIC 数据模式选择：
 * 默认使用 `SLIDE_APP_MODE`
 */
typedef enum
{
    PC_LINK_MODE,
    SLIDE_APP_MODE, // Define
} i2c_data_mode_t;
/**
 * @brief 设定阈值或应用：
 * 在 Slide application 模式，写入数据区分成应用设定以及阀值设定，当CT为 `SETING_COMMANDS` 时是写入应
 * 用设定，当CT为 `CUSTOM_THRESHOLD_COMMANDS` 时是写入阀值设定.
 */
typedef enum
{
    SETING_COMMANDS,
    CUSTOM_THRESHOLD_COMMANDS,
} custom_threshold_t;
/**
 * @brief 按键输出模式：
 * 有多个按键输出以及单一按键输出两种模式。此选项是对普通按键的输出设
 * 定，滑条按键则不受影响。单一按键输出模式时只会输出第一个被按下的按键，当按键放开
 * 后才会承认其它按键。
 * 默认使用 `SINGLE`
 */
typedef enum
{
    MULTIPLE,
    SINGLE, // Define
} key_output_mode_t;
/**
 * @brief 基准值自动调整：
 * 当无按键时，自动更新基准值。
 * 默认使用 `AUTO_ADJUST_ENABALE`
 */
typedef enum
{
    AUTO_ADJUST_DISABLE,
    AUTO_ADJUST_ENABALE, // Define
} aoto_adjust_en_t;
/**
 * @brief 省电模式：
 * 无按键4秒后进入睡眠模式。
 * 静态电流为:6.8uA@3.0V 工作电流:1.1mA@3.0V
 * 默认使用 `POWER_SAVE_ENABALE`
 */
typedef enum
{
    POWER_SAVE_DISABLE,
    POWER_SAVE_ENABALE, // Define
} power_save_mode_en_t;
/**
 * @brief 动态阀值:
 * 功能开启时，滑调按键的阀值会依照按压位置自动调整。
 * 默认使用 `DYNAMIC_THRESHOLD_DISABLE`
 */
typedef enum
{
    DYNAMIC_THRESHOLD_DISABLE, // Define
    DYNAMIC_THRESHOLD_ENABALE,
} dynamic_threshold_en_t;
/**
 * @brief 自动重置时间设定:
 * 在按键位置没有改变时开始计时，时间到自动重置。
 * 默认使用 `AOTO_RESTET_TIME_15S`
 */
typedef enum
{
    AOTO_RESTET_TIME_DISABLE = 0B00,
    AOTO_RESTET_TIME_15S, // Define
    AOTO_RESTET_TIME_30S,
    AOTO_RESTET_TIME_60S,
} aoto_reset_time_t;
/**
 * @brief 按键消抖时间:
 * 默认使用 `KEY_ACK_TIME_4`
 */
typedef enum
{
    KEY_ACK_TIME_1 = 0B000,
    KEY_ACK_TIME_2,
    KEY_ACK_TIME_3,
    KEY_ACK_TIME_4, // Define
    KEY_ACK_TIME_5,
    KEY_ACK_TIME_6,
    KEY_ACK_TIME_7,
    KEY_ACK_TIME_8,
} key_acknowledge_times_t;
/**
 * @brief 按键数设定:
 * 当滑条设定Disable时普通按键最大按键数为9Keys。
 * 当普通按键数设定9Keys时，若有滑条按键设定，
 * 则以最大按键数9减去滑条按键数，为普通按键数。
 * 默认使用 `KEY_NUM_9`
 */
typedef enum
{
    KEY_NUM_0_DISABLE = 0B00000,
    KEY_NUM_1,
    KEY_NUM_2,
    KEY_NUM_3,
    KEY_NUM_4,
    KEY_NUM_5,
    KEY_NUM_6,
    KEY_NUM_7,
    KEY_NUM_8,
    KEY_NUM_9, // Define
} key_number_t;
/**
 * @brief 滑条按键数设定:
 * 最多为9Keys。
 * 默认使用 `SLIDE_X_NUM_DISABLE`
 */
typedef enum
{
    SLIDE_X_NUM_DISABLE = 0B0000, // Define
    // SLIDE_X_NUM_2_DISABLE,
    SLIDE_X_NUM_3 = 0B0010,
    SLIDE_X_NUM_4,
    SLIDE_X_NUM_5,
    SLIDE_X_NUM_6,
    SLIDE_X_NUM_7,
    SLIDE_X_NUM_8,
    SLIDE_X_NUM_9,
} slide_x_number_t; // A minimum of three keys is required to form a slider
/**
 * @brief 多按键重置设定:
 * 最多为9Keys.以Slide1、Slide2、Slide3、Normal个别按压按键数做判断。
 * 默认使用 `KEY_OFF_NUM_1_DISABLE`
 */
typedef enum
{
    KEY_OFF_NUM_1_DISABLE = 0B0000, // Define
    KEY_OFF_NUM_2,
    KEY_OFF_NUM_3,
    KEY_OFF_NUM_4,
    KEY_OFF_NUM_5,
    KEY_OFF_NUM_6,
    KEY_OFF_NUM_7,
    KEY_OFF_NUM_8,
    KEY_OFF_NUM_9,
} key_off_number_t;
/**
 * @brief 
 * 按键的期待值与阀值设定是依照触摸按键的脚位编排，若滑条按键3 keys普通按键6keys，则 
 * TP0 – TP2 为滑条按键，TP3 – TP8 为普通按键。
 */
typedef enum
{
    TP_NUM_0 =0B11000000,
    TP_NUM_1,
    TP_NUM_2,
    TP_NUM_3,
    TP_NUM_4,
    TP_NUM_5,
    TP_NUM_6,
    TP_NUM_7,
    TP_NUM_8,
    TP_NUM_9,   //? 什么几把。。哪来的第十个按键??按手册写上吧免得有问题。。。
} tpx_setting_number_t;
/**
 * @brief 滑条位置标志的三个滑条枚举
 * 
 */
typedef enum{
    SLIDE_1_POSITION = 3,
    SLIDE_2_POSITION,
    SLIDE_3_POSITION,
}slider_x_position_t;
/**
 * @brief 滑条按键标志的三个滑条枚举
 * 
 */
typedef enum{
    SLIDE_1_TOUCH_STATE = 0,
    SLIDE_2_TOUCH_STATE,
    SLIDE_3_TOUCH_STATE,
}slider_x_touch_state_t;

/**
 * @brief I2C读写函数指针接口，对接相应芯片开发平台的I2C读写函数
 * 
 */
typedef uint32_t (*vk_com_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len); //! 类型错误：int

/**************************************************************************/
/*!
    @brief The VK3809IP driver class.
*/
/**************************************************************************/
class VK3809IP
{
public:
    VK3809IP(void);

    int begin(vk_com_fptr_t read_cb, vk_com_fptr_t write_cb, uint8_t addr = VK3809IP_ADDR);

    // bool begin(TwoWire *theWire = &Wire);

    bool init();
    
    /* 
        滑条按键可设置3~9Key，当设置为3Key时使用的是TP0~TP2，TP3~TP8可规划为一般按键;若
        滑条按键设置为4Key则使用TP0~TP3，TP4~TP8可规划为一般按键。如下表:
        Slide 1 Disable 3Key      3Key      3Key      Disable   Disable   9Key 
        Slide 2 Disable Disable   3Key      3Key      4Key      Disable   Disable 
        Slide 3 Disable Disable   Disable   3Key      4Key      4Key      Disable 
        TP0     Key 1   Slide 1_1 Slide 1_1 Slide 1_1 Slide 2_1 Slide 3_1 Slide 1_1 
        TP1     Key 2   Slide 1_2 Slide 1_2 Slide 1_2 Slide 2_2 Slide 3_2 Slide 1_2 
        TP2     Key 3   Slide 1_3 Slide 1_3 Slide 1_3 Slide 2_3 Slide 3_3 Slide 1_3 
        TP3     Key 4   Key 1     Slide 2_1 Slide 2_1 Slide 2_4 Slide 3_4 Slide 1_4 
        TP4     Key 5   Key 2     Slide 2_2 Slide 2_2 Slide 3_1 Key 1     Slide 1_5 
        TP5     Key 6   Key 3     Slide 2_3 Slide 2_3 Slide 3_2 Key 2     Slide 1_6 
        TP6     Key 7   Key 4     Key 1     Slide 3_1 Slide 3_3 Key 3     Slide 1_7 
        TP7     Key 8   Key 5     Key 2     Slide 3_2 Slide 3_4 Key 4     Slide 1_8 
        TP8     Key 9   Key 6     Key 3     Slide 3_3 Key 1     Key 5     Slide 1_9 
        ! 滑条按键需要依照编号顺序排列，才能正确计算位置，禁止任意变换排列顺序。
    */
    uint8_t settingCommandsDataByte1(
        i2c_data_mode_t i2c_data_mode_slide,
        custom_threshold_t custom_threshold_set,
        key_output_mode_t key_output_mode,
        aoto_adjust_en_t aoto_adjust,
        power_save_mode_en_t power_save_mode,
        dynamic_threshold_en_t dynamic_threshold,
        aoto_reset_time_t aoto_reset_time);
    uint8_t settingCommandsDataByte2(
        key_number_t key_number,
        key_acknowledge_times_t key_acknowledge_times);
    uint8_t settingCommandsDataByte3(
        slide_x_number_t slide_2_number,
        slide_x_number_t slide_1_number);
    uint8_t settingCommandsDataByte4(
        key_off_number_t key_off_number,
        slide_x_number_t slide_3_number);

    bool settingCommandsData(uint8_t DataByte1, uint8_t DataByte2, uint8_t DataByte3, uint8_t DataByte4);

    bool settingTpxThresholdData(uint16_t thresholdValue, tpx_setting_number_t tpNum);
    bool settingSleepThresholdData(uint16_t thresholdValue);

    bool getSystemCorrectionFlagState();
    bool getSystemWriteFlagState();
    bool getSliderPressedState(slider_x_touch_state_t sliderNum);
    bool getKeyPressedState(key_number_t keyNum);

    uint16_t getSliderData(slider_x_position_t position);
    
    void print_byte_as_binary(uint8_t byte);
    uint8_t* getAllData();

private:
    uint8_t _address;

    uint8_t extractBits(uint8_t byte, int startBit, int numBits);

    int _readByte(uint8_t nbytes, uint8_t *data);
    int _writeByte(uint8_t nbytes, uint8_t *data);
    
    bool writeThreeByteData(uint8_t DataByte1, uint8_t DataByte2, uint8_t DataByte3);
    bool writeFourByteData(uint8_t DataByte1, uint8_t DataByte2, uint8_t DataByte3, uint8_t DataByte4);

    vk_com_fptr_t _read_cb = nullptr;
    vk_com_fptr_t _write_cb = nullptr;
    // I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
};

extern VK3809IP slider;

#ifdef __cplusplus
}
#endif
