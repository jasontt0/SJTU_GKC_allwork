//*************************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: adc_demo.c
// Description:
//    1.该示例展示如何利用CH0/PE3和CH1/PE2端口实现两路ADC采样,采样频率25Hz；
//    2.左侧四个数码管显示CH0/PE3的ADC采样值[0-4095]；
//    3.右侧四个数码管显示CH1/PE2的ADC采样值[0-4095]；
//    4.注意：输入电压值范围必须为[0-3.3V]，否则会烧坏端口。
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20201125
// Date：2020-11-25
// History：
//
//*************************************************************************************

//*************************************************************************************
//
// 头文件
//
//*************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"       // 基址宏定义
#include "inc/hw_types.h"        // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"     // 调试用
#include "driverlib/gpio.h"      // 通用IO口宏定义
#include "driverlib/pin_map.h"   // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"    // 系统控制定义
#include "driverlib/systick.h"   // SysTick Driver 原型
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver 原型
#include "driverlib/adc.h"       // 与ADC有关的定义
#include "tm1638.h"              // 与控制TM1638芯片有关的函数

#include "DAC6571.h" // ???DAC6571???????

//*************************************************************************************
//
// 宏定义
//
//*************************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTick频率为50Hz，即循环定时周期20ms

#define V_T40ms 1  // 40ms软件定时器溢出值，2个20ms
#define V_T2s 5  // 2s软件定时器溢出值，100个20ms
#define V_T80ms 4 // 80s软件定时器溢出值，4个20ms

//*************************************************************************************
//
// 函数原型声明
//
//*************************************************************************************
void GPIOInit(void);    // GPIO初始化
void ADCInit(void);     // ADC初始化
void SysTickInit(void); // 设置SysTick中断
void DevicesInit(void); // MCU器件初始化，注：会调用上述函数
void ADC_Sample(void);  // 获取ADC采样值
//*************************************************************************************
//
// 变量定义
//
//*************************************************************************************

// 软件定时器计数
uint8_t clock40ms = 0;
uint8_t clock2s = 0;
uint8_t clock80ms = 0;

// 软件定时器溢出标志
uint8_t clock40ms_flag = 0;
uint8_t clock2s_flag = 0;
uint8_t clock80ms_flag = 0;

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x11;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0};

// 当前按键值
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// 系统时钟频率
uint32_t ui32SysClock;

// 存放从ADC FIFO读取的采样数据 [0-4095]
uint32_t pui32ADC0Value[4];

uint32_t ADCsteadyV_showV = 0;
uint32_t ADCsteadyV_showI = 0;
uint32_t ADCsteadyI_showI = 0;

uint32_t ADCsteadyI_showI_temp = 0;

uint32_t digitLeft = 0;
uint32_t digitRight = 0;

uint8_t mode=0;

// DAC6571
uint32_t DAC6571_code = 300;
uint32_t target_current = 1000;
uint32_t DAC6571_voltage = 250;
uint8_t DAC6571_flag = 0;

uint32_t max_current = 1001;

bool output_flag1 = 1, output_flag2 = 1;

//*************************************************************************************
//
// 主程序
//
//*************************************************************************************
int main(void)
{
    DevicesInit(); //  MCU器件初始化

    while (clock2s < 3)
        ;          // 延时>60ms,等待TM1638上电完成
    TM1638_Init(); // 初始化TM1638

    DAC6571_flag = 1;

    while (1)
    {
        if (DAC6571_flag == 1)
        {
            // if (ADCsteadyI_showI < 30)
            //     target_current = max_current;
            // if (target_current > max_current)
            //     target_current = max_current;
            // DAC6571_code = 0.31888594164456235 * target_current + -21.46153846153847;
            
            // //有问题？？？？----------------------------------------------
            // DAC6571_Fastmode_Operation(DAC6571_code);
        }
        

        if (clock2s_flag == 1)
        {
            clock2s_flag = 0;

            ADCsteadyV_showV = ADCsteadyV_showV * 5040 / 3100;
            ADCsteadyV_showI = 0.5269 * ADCsteadyV_showI + 74.2041;
            // 开机左边先显示输出电压，按8切换成稳压源电流
            if (output_flag1)
                digitLeft = ADCsteadyV_showV;
                //digitLeft = 4321;
            else
                digitLeft = ADCsteadyV_showI; 
            //digitLeft = ADCsteadyV_showV;
            digit[4] = digitLeft / 1000;     // 显示CH0/PE3的ADC采样值千位数
            digit[5] = digitLeft / 100 % 10; // 显示CH0/PE3的ADC采样值百位数
            digit[6] = digitLeft / 10 % 10;  // 显示CH0/PE3的ADC采样值十位数
            digit[7] = digitLeft % 10;       // 显示CH0/PE3的ADC采样值个位数


            target_current = ADCsteadyI_showI;

            if (ADCsteadyI_showI < 30)
                target_current = max_current;
            if (target_current > max_current)
                target_current = max_current;
            DAC6571_code = 0.31888594164456235 * target_current + -21.46153846153847;
            
            //有问题？？？？----------------------------------------------
            //DAC6571_Fastmode_Operation(DAC6571_code);

            // 开机右边先显示目标电流target_current，按9切换成稳流源电流
            if (output_flag2)
                digitRight = target_current;
            else
                digitRight = ADCsteadyI_showI; 
                //digitRight = 1234;

            //digitRight = ADCsteadyI_showI;
            digit[0] = digitRight / 1000;     // 显示CH1/PE2的ADC采样值千位数
            digit[1] = digitRight / 100 % 10; // 显示CH1/PE2的ADC采样值百位数
            digit[2] = digitRight / 10 % 10;  // 显示CH1/PE2的ADC采样值十位数
            digit[3] = digitRight % 10;       // 显示CH1/PE2的ADC采样值个位数

            ADCsteadyV_showV = 0;
            ADCsteadyV_showI = 0;
            ADCsteadyI_showI = 0;

            

        }

        if (clock40ms_flag == 1) // 检查40ms秒定时是否到
        {
            clock40ms_flag = 0;

            ADC_Sample();

            
        }

        if (clock80ms_flag == 1)
        {
            clock80ms_flag = 0;
            
        }
        

    }
}

//*************************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void GPIOInit(void)
{
    // 配置TM1638芯片管脚
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); // 使能端口 K
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
    {
    }; // 等待端口 K准备完毕

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); // 使能端口 M
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
    {
    }; // 等待端口 M准备完毕

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // 使能端口 M
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
    {
    }; // 等待端口 M准备完毕

    // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);

    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);
}

//*************************************************************************************
//
// 函数原型：void ADCInit(void)
// 函数功能：ADC0初始化
//          选择CHO/PE3和CH1/PE2引脚作为ADC0采样输入端口，选用采样序列产生器1
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void ADCInit(void)
{
    // 使能ADC0模块
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // 使能端口E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // 使用CH0/PE3和CH1/PE2引脚作为ADC输入
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);

    // 选用采样序列产生器1，采样起始信号由ADCProcessorTrigger函数触发，优先级为0
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    // 设置采样序列产生器1第0步骤:选择CH0为采样通道
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0); // CH0/PE3

    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0); // CH1/PE2

    // 设置采样序列产生器1第1步骤:
    // ADC_CTL_CH1--选择CH1为采样通道， ADC_CTL_END--采样序列的最后一步
    // ADC_CTL_IE--采样结束后产生中断
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH1 | ADC_CTL_END | ADC_CTL_IE); // CH2/PE1

    // 使能采样序列产生器1
    ADCSequenceEnable(ADC0_BASE, 1);

    // 在采样前，必须清除中断状态标志
    ADCIntClear(ADC0_BASE, 1);
}

//*************************************************************************************
//
// 函数原型：void ADC_Sample(void)
// 函数功能：获取ADC采样值
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void ADC_Sample(void)
{
    // pui32ADC0Value数组用于从ADC FIFO读取的数据
    extern uint32_t pui32ADC0Value[4];

    // 触发ADC0采样序列发生器1开始工作
    ADCProcessorTrigger(ADC0_BASE, 1);

    // 等待ADC0采样序列发生器1采样转换完成
    while (!ADCIntStatus(ADC0_BASE, 1, false))
    {
    }

    // 清除ADC0中断标志
    ADCIntClear(ADC0_BASE, 1);

    // 读取ADC0采样值，存储到数组pui32ADC0Value中
    ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);

    // ui32ADC0Value[0]为CH0/PE3稳压源电压的采样值
    ADCsteadyV_showV = ADCsteadyV_showV + pui32ADC0Value[0] / 5;
    // ui32ADC0Value[1]为CH1/PE2稳压源电流的采样值
    ADCsteadyV_showI = ADCsteadyV_showI + pui32ADC0Value[1] / 5;
    // ui32ADC0Value[1]为CH1/PE2稳流源电流的采样值
    ADCsteadyI_showI = ADCsteadyI_showI + pui32ADC0Value[2] / 5;
}

//*************************************************************************************
//
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void SysTickInit(void)
{
    SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
    SysTickEnable();                                    // SysTick使能
    SysTickIntEnable();                                 // SysTick中断允许
}

//*************************************************************************************
//
// 函数原型：DevicesInit(void)
// 函数功能：MCU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void DevicesInit(void)
{
    // 使用外部25MHz主时钟源，经过PLL，然后分频为20MHz
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                      20000000);

    GPIOInit();        // GPIO初始化
    ADCInit();         // ADC初始化
    SysTickInit();     // 设置SysTick中断
    IntMasterEnable(); // 总中断允许
}

//*************************************************************************************
//
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void SysTick_Handler(void) // 定时周期为20ms
{

    // 40ms秒钟软定时器计数
    if (++clock40ms >= V_T40ms)
    {
        clock40ms_flag = 1; // 当40ms到时，溢出标志置1
        clock40ms = 0;
    }

    if (++clock80ms >= V_T80ms)
    {
        clock80ms_flag = 1; // 当40ms到时，溢出标志置1
        clock80ms = 0;

        if (ADCsteadyI_showI_temp != ADCsteadyI_showI)
        {
            // DAC6571_flag = 1;
            // ADCsteadyI_showI_temp = ADCsteadyI_showI;

            // target_current =  ADCsteadyI_showI;

            // if (ADCsteadyI_showI < 30)
            //     target_current = max_current;
            // if (target_current > max_current)
            //     target_current = max_current;
            // DAC6571_code = 0.31888594164456235 * target_current + -21.46153846153847;
            
            // //有问题？？？？----------------------------------------------
            // //DAC6571_Fastmode_Operation(DAC6571_code);

            
        }
        
    }

    // 0.1秒钟软定时器计数
    if (++clock2s >= V_T2s)
    {
        clock2s_flag = 1; // 当0.1秒到时，溢出标志置1
        clock2s = 0;

        //TM1638_RefreshDIGIandLED(digit, pnt, led);
    }

    // 刷新全部数码管和LED
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
    key_code = TM1638_Readkeyboard();

        if (key_code != 0)
    {
        if (key_cnt < 4)
            key_cnt++; // 按键消抖，4*20ms
        else if (key_cnt == 4)
        {

            switch (key_code)
            {
            // case 1: // 加100
            //     if (target_current < max_current - 100)
            //     {
            //         target_current += 100;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 4: // 减100
            //     if (target_current > 100)
            //     {
            //         target_current -= 100;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 2: // 加10
            //     if (target_current < max_current - 10)
            //     {
            //         target_current += 10;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 5: // 减10
            //     if (target_current >= 10)
            //     {
            //         target_current -= 10;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 3: // 加1
            //     if (target_current < max_current - 1)
            //     {
            //         target_current += 1;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 6: // 减1
            //     if (target_current >= 1)
            //     {
            //         target_current -= 1;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            case 8: // 转换输出模式，电压/电流
                output_flag1 = !output_flag1;
                break;
            case 9: // 转换输出模式，电压/电流
                output_flag2 = !output_flag2;
                break;
            default:
                break;
            }

            key_cnt = 5; // 按键一直按着，只改变一次
        }
    }
    else
        key_cnt = 0;

}
