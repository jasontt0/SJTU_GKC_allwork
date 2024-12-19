//*****************************************************************************
//
// DAC6571.h - Prototypes for the DAC6571 driver.
//
// Copyright��2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// 
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20201203 
// Date��2020-12-03
// History��
//
//*****************************************************************************

#ifndef __DAC6571_H__
#define __DAC6571_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "inc/hw_i2c.h"
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	  // ϵͳ���ƶ���
#include "driverlib/i2c.h"

    
////DAC6571����

// ��������DAC6571��TM4C1294 GPIO���Ŷ���
#define SDA_PIN_BASE  GPIO_PORTL_BASE
#define SDA_PIN       GPIO_PIN_0
#define SCL_PIN_BASE  GPIO_PORTL_BASE
#define SCL_PIN       GPIO_PIN_1

#define DAC6571_code_max        1024  // DAC6571��10bits��DAC, 2^10=1024
#define DAC6571_address         0x98  // DAC6571�ĵ�ַ��1001 10 A0 0  A0=0

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
	
// ���һ��DACת��
extern void DAC6571_Fastmode_Operation(uint32_t);

// ��DAC6571��������1���ֽڣ�8λ������
extern void DAC6571_Byte_Transmission(uint8_t byte_data);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __DAC6571_H__
