//*****************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: ADC_Demo.c
// Description: 该示例展示如何利用AIN2/PE1端口实现单端输入单次ADC采样,采样频率25Hz
//    1.左侧四个数码管显示ADC采样值[0-4095]；
//    2.右侧三个数码管显示电压值[0.00-3.30V]；
//    3.注意：输入电压值范围必须为[0-3.3V]，否则会烧坏端口。
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20210513
// Date：2021-05-13
// History：
//
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/adc.h"        // 与ADC有关的定义 

#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "ADC.h"

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]= {' ',' ',' ',' ',' ',' ',' ',' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x2;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 0, 0, 0, 0, 0, 0, 0};

// 系统时钟频率
uint32_t ui32SysClock;

// AIN2(PE1)  ADC采样值[0-4095]
uint32_t ui32ADC0Value;

// AIN2电压值(单位为0.01V) [0.00-3.30]
uint32_t ui32ADC0Voltage;

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
void ADC_Voltage(void)          //将电压值采样，并显示到数码管上
{
    ui32ADC0Value = ADC_Sample();   // 采样

    //digit[4] = ui32ADC0Value / 1000; 	     // 显示ADC采样值千位数
    //digit[5] = ui32ADC0Value / 100 % 10; 	 // 显示ADC采样值百位数
    //digit[6] = ui32ADC0Value / 10 % 10; 	 // 显示ADC采样值十位数
    //digit[7] = ui32ADC0Value % 10;           // 显示ADC采样值个位数

    ui32ADC0Voltage = ui32ADC0Value * 330 / 4095;

    digit[1] = (ui32ADC0Voltage / 100) % 10; // 显示电压值个位数
    digit[2] = (ui32ADC0Voltage / 10) % 10;  // 显示电压值十分位数
    digit[3] = ui32ADC0Voltage % 10;         // 显示电压值百分位数

}


