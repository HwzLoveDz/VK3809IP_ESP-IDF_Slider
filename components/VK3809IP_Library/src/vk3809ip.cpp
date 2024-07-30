/**
 * @file vk3809ip.cpp
 * @author by mondraker (https://oshwhub.com/mondraker)(https://github.com/HwzLoveDz)
 * @brief vk3809ip capacitive touch button slider chip driver library
 * @version 0.1
 * @date 2024-07-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#if ARDUINO >= 100
#include "Arduino.h"
#else
// #include "WProgram.h"
#include <math.h>
#endif

#include "vk3809ip.hpp"

VK3809IP::VK3809IP() {}

int VK3809IP::begin(vk_com_fptr_t read_cb, vk_com_fptr_t write_cb, uint8_t addr)
{
  if (read_cb == nullptr || write_cb == nullptr)
    return -1;
  _read_cb = read_cb;
  _write_cb = write_cb;
  _address = addr;
  return init();
}

bool VK3809IP::init()
{
  // Default setting commands
  uint8_t settingDataByte1 = settingCommandsDataByte1(
      SLIDE_APP_MODE,
      SETING_COMMANDS,
      SINGLE,
      AUTO_ADJUST_ENABALE,
      POWER_SAVE_DISABLE,
      DYNAMIC_THRESHOLD_DISABLE,
      AOTO_RESTET_TIME_15S
      );
  uint8_t settingDataByte2 = settingCommandsDataByte2(
      KEY_NUM_0_DISABLE,
      KEY_ACK_TIME_4
      );
  uint8_t settingDataByte3 = settingCommandsDataByte3(
      SLIDE_X_NUM_DISABLE,
      SLIDE_X_NUM_9
      );
  uint8_t settingDataByte4 = settingCommandsDataByte4(
      KEY_OFF_NUM_1_DISABLE,
      SLIDE_X_NUM_DISABLE
      );
  settingCommandsData(settingDataByte1, settingDataByte2, settingDataByte3, settingDataByte4);

  // Default custom threshold commands
  for (int i = TP_NUM_0; i <= TP_NUM_9; i++)
  {
    settingTpxThresholdData(16, (tpx_setting_number_t)i);
  }
  
  // Default sleep threshold Setting
  settingSleepThresholdData(2);

  return 0;
}

/**************************************************************************/
/*!
    @brief The VK3809IP write function.
*/
/**************************************************************************/

uint8_t VK3809IP::settingCommandsDataByte1(
    i2c_data_mode_t i2c_data_mode_slide,
    custom_threshold_t custom_threshold_set,
    key_output_mode_t key_output_mode,
    aoto_adjust_en_t aoto_adjust,
    power_save_mode_en_t power_save_mode,
    dynamic_threshold_en_t dynamic_threshold,
    aoto_reset_time_t aoto_reset_time)
{
  uint8_t setbyte1 = 0B00000000;

  // 设置 setbyte1 命令
  setbyte1 |= (i2c_data_mode_slide << 7);     // bit 7
  setbyte1 |= (custom_threshold_set << 6);  // bit 6
  setbyte1 |= (key_output_mode << 5);   // bit 5
  setbyte1 |= (aoto_adjust << 4);       // bit 4
  setbyte1 |= (power_save_mode << 3);   // bit 3
  setbyte1 |= (dynamic_threshold << 2); // bit 2
  setbyte1 |= aoto_reset_time;          // bit 1-0

  return setbyte1;
}
uint8_t VK3809IP::settingCommandsDataByte2(
    key_number_t key_number,
    key_acknowledge_times_t key_acknowledge_times)
{
  uint8_t setbyte2 = 0B00000000;

  // 设置 setbyte2 命令
  setbyte2 |= (key_number << 3);            // bit 7-3
  setbyte2 |= key_acknowledge_times;        // bit 2-0

  return setbyte2;
}
uint8_t VK3809IP::settingCommandsDataByte3(
        slide_x_number_t  slide_2_number,
        slide_x_number_t slide_1_number)
{
  uint8_t setbyte3 = 0B00000000;

  // 设置 setbyte3 命令
  setbyte3 |= (slide_2_number << 4);            // bit 7-4
  setbyte3 |= slide_1_number;             // bit 3-0

  return setbyte3;
}
uint8_t VK3809IP::settingCommandsDataByte4(
    key_off_number_t  key_off_number,
    slide_x_number_t slide_3_number)
{
  uint8_t setbyte4 = 0B00000000;

  // 设置 setbyte4 命令
  setbyte4 |= (key_off_number << 4);            // bit 7-4
  setbyte4 |= slide_3_number; // bit 3-0

  return setbyte4;
}
/**
 * @brief 应用模式命令设置
 * 
 * @param DataByte1 
 * @param DataByte2 
 * @param DataByte3 
 * @param DataByte4 
 * @return true 
 * @return false 
 */
bool VK3809IP::settingCommandsData(uint8_t DataByte1, uint8_t DataByte2, uint8_t DataByte3, uint8_t DataByte4)
{
  return writeFourByteData(DataByte1, DataByte2, DataByte3, DataByte4);
}

/**
 * @brief 按键阈值设定:
 * 按键承认阀值越小灵敏度越高，越大灵敏度越低。预设的阀值为010H，建议的最小值为008H，若
 * 调整到008H按键灵敏度仍然不够，则建议加大CS电容，CS电容的值则建议小于39nF
 * 
 * @param thresholdValue 按键承认阀值(Define : 010H) 
 * @param tpNum 按键键值
 * @return true 
 * @return false 
 */
bool VK3809IP::settingTpxThresholdData(uint16_t thresholdValue, tpx_setting_number_t tpNum)
{
    // 确保 thresholdValue 是三位十进制数的范围
    if (thresholdValue <= 8) {
        thresholdValue = 8;
    }else if(thresholdValue >= 999) {
        thresholdValue = 999;
    }

    // 将 thresholdValue 转换为12位二进制字符串
    char binary_12bit[13]; // 12位二进制 + 结束符 '\0'
    int num = thresholdValue;
    for (int i = 11; i >= 0; --i) {
        binary_12bit[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    binary_12bit[12] = '\0'; // 添加字符串结束符

    // 拆分为 h, m, l 三部分
    char l[5], m[5], h[5];
    strncpy(l, binary_12bit, 4); l[4] = '\0';
    strncpy(m, binary_12bit + 4, 4); m[4] = '\0';
    strncpy(h, binary_12bit + 8, 4); h[4] = '\0';

    // 构造 byte2 和 byte3
    uint8_t data[3];
    data[0] = tpNum;
    data[1] = (uint8_t)(strtol(m, NULL, 2) << 4 | strtol(l, NULL, 2));
    data[2] = (uint8_t)(strtol(h, NULL, 2) << 4);
    writeThreeByteData(data[0], data[1], data[2]);
    return VK_PASS;
}

/**
 * @brief 睡眠唤醒阀值设定
 * 
 * @param thresholdValue 省电模式唤醒阀值(Define : 002H) 
 * @return true 
 * @return false 
 */
bool VK3809IP::settingSleepThresholdData(uint16_t thresholdValue)
{
    if (thresholdValue <= 0) {
        thresholdValue = 0;
    }else if(thresholdValue >= 999) {
        thresholdValue = 999;
    }

    char binary_12bit[13];
    int num = thresholdValue;
    for (int i = 11; i >= 0; --i) {
        binary_12bit[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    binary_12bit[12] = '\0';

    char l[5], m[5], h[5];
    strncpy(l, binary_12bit, 4); l[4] = '\0';
    strncpy(m, binary_12bit + 4, 4); m[4] = '\0';
    strncpy(h, binary_12bit + 8, 4); h[4] = '\0';

    uint8_t data[3];
    data[0] = 0xD0;
    data[1] = (uint8_t)(strtol(m, NULL, 2) << 4 | strtol(l, NULL, 2));
    data[2] = (uint8_t)(strtol(h, NULL, 2) << 4);
    writeThreeByteData(data[0], data[1], data[2]);
    return VK_PASS;
}

/**************************************************************************/
/*!
    @brief The VK3809IP read function.
*/
/**************************************************************************/

/**
 * @brief 系统校正标志:
 * 当值为0时，表示系统校正中，键值读取无效。当值为1时，键值有效。
 * @return true 
 * @return false 
 */
bool VK3809IP::getSystemCorrectionFlagState()
{
  uint8_t data[6];
  _readByte(sizeof(data), data);
  return (bool)extractBits(data[0], 7, 1);
}
/**
 * @brief 系统写入标志:
 * 上电为1，写入设定后该标志设置为0。
 * @return true 
 * @return false 
 */
bool VK3809IP::getSystemWriteFlagState()
{
  uint8_t data[6];
  _readByte(sizeof(data), data);
  return (bool)extractBits(data[0], 6, 1);
}
/**
 * @brief 滑条触摸标志:
 * 无触摸时为0，有触摸时为1
 * @param sliderNum 
 * @return true 
 * @return false 
 */
bool VK3809IP::getSliderPressedState(slider_x_touch_state_t sliderNum)
{
  uint8_t data[6];
  _readByte(sizeof(data), data);
  return (bool)extractBits(data[0], sliderNum, 1);
}
/**
 * @brief 触摸按键标志:
 * 无按键为0，有按键为1
 * @param keyNum 
 * @return true 
 * @return false 
 */
bool VK3809IP::getKeyPressedState(key_number_t keyNum)
{
  uint8_t data[6];
  _readByte(sizeof(data), data);
  // return (bool)extractBits(data[1], keyNum - 1, 1);
  return (keyNum == KEY_NUM_9) ? ((bool)extractBits(data[2], 0, 1)) : ((bool)extractBits(data[1], keyNum - 1, 1));
}
/**
 * @brief 滑条位置标志:
 * 预设为0，触摸滑条后输出按键位置，放开后保留最后按压位置
 * @param position 
 * @return uint16_t 
 */
uint16_t VK3809IP::getSliderData(slider_x_position_t position)
{
  uint8_t data[6];
  _readByte(sizeof(data), data);
  return data[position];
}
/**
 * @brief byte转换成bit
 * 调试时候用的
 * @param byte 
 */
void VK3809IP::print_byte_as_binary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%c", (byte & (1 << i)) ? '1' : '0');
    }
}
/**
 * @brief 读寄存器的所有值:
 * delete[] data; // 记得在使用完毕后释放动态分配的内存
 * @return uint8_t* 
 */
uint8_t* VK3809IP::getAllData()
{
  // uint8_t data[6];
  uint8_t* data = new uint8_t[6]; // 动态分配 6 个字节的空间
  _readByte(sizeof(data), data);
  return data;
}

/**************************************************************************/
/*!
    @brief The VK3809IP port function.
*/
/**************************************************************************/

/**
 * @brief 从一个字节中截取指定的位段
 * 
 * @param byte 要截取的字节
 * @param start_bit 目标开始位
 * @param num_bits 截取位数
 * @return uint8_t 
 */
uint8_t VK3809IP::extractBits(uint8_t byte, int startBit, int numBits) {
    // 计算掩码
    uint8_t mask = (1 << numBits) - 1; // 生成指定长度的全1掩码
    mask <<= startBit;                 // 将掩码移到起始位
    // 使用掩码提取位段并右移到最低位
    return (uint8_t)((byte & mask) >> startBit);
}

bool VK3809IP::writeThreeByteData(uint8_t DataByte1, uint8_t DataByte2, uint8_t DataByte3)
{
  uint8_t settingData[] = {DataByte1, DataByte2, DataByte3};
  _writeByte(sizeof(settingData), settingData);
  return VK_PASS;
}
bool VK3809IP::writeFourByteData(uint8_t DataByte1, uint8_t DataByte2, uint8_t DataByte3, uint8_t DataByte4)
{
  uint8_t settingData[] = {DataByte1, DataByte2, DataByte3, DataByte4};
  _writeByte(sizeof(settingData), settingData);
  return VK_PASS;
}

int VK3809IP::_readByte(uint8_t nbytes, uint8_t *data)
{
  if (_read_cb != nullptr)
  {
    return _read_cb(_address, REG_ADDR_NONE, data, nbytes);
  }

  return 0;
}
int VK3809IP::_writeByte(uint8_t nbytes, uint8_t *data)
{
  if (_write_cb != nullptr)
  {
    return _write_cb(_address, REG_ADDR_NONE, data, nbytes);
  }

  return 0;
}

VK3809IP slider;
