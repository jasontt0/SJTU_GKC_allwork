//*******************************************************************************************************
//
// Copyright: 2021-2022, 上海交通大学电子工程系实验教学中心
// File name: Frequency_Measure.c
// Description: 本示例展示如何使用测频率法实现方波频率的测量（待测方波信号连接到T0CCP0/PL4引脚）
// 1.每次按下A2000TM4C底板按键1(SW1)后,程序开始测量方波信号的频率，测量完成后数码管显示测量结果,单位为Hz;
// 2.如果测试频率不在【10，9999】范围内容，则显示Err。
// 3.注意事项：待测方波信号高电平不宜高于3.3V，低电平不宜低于0V，并且一定要与底板共地
//             该程序不能用于检测<=1Hz频率的方波，频率检测误差率正负1Hz
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20210513
// Date：2021-05-13
// History：
//
//*******************************************************************************************************

//*******************************************************************************************************
//
// 头文件
//
//*******************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"       // 基址宏定义
#include "inc/hw_types.h"        // 数据类型宏定义，寄存器访问函数
#include "inc/hw_timer.h"        // 与定时器有关的宏定义
#include "inc/hw_ints.h"         // 与中断有关的宏定义
#include "driverlib/debug.h"     // 调试用
#include "driverlib/gpio.h"      // 通用IO口宏定义和函数原型
#include "driverlib/pin_map.h"   // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"    // 系统控制定义
#include "driverlib/systick.h"   // SysTick Driver 函数原型
#include "driverlib/interrupt.h" // NVIC中断控制驱动函数原型
#include "driverlib/timer.h"     // 与Timer有关的函数原型
#include "driverlib/pwm.h"       // 与Timer有关的函数原型
#include "driverlib/uart.h"
#include "driverlib/fpu.h"

#include "tm1638.h" // 与控制TM1638芯片有关的宏定义和函数原型

//*******************************************************************************************************
//
// 宏定义
//
//*******************************************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTick频率为50Hz，即循环定时周期20ms

#define FREQUENCY_MIN 10   // 有效频率最小值，根据实际需要设置
#define FREQUENCY_MAX 9999 // 有效频率最大值，根据实际需要设置
//*******************************************************************************************************
//
// 函数原型声明
//
//*******************************************************************************************************
void Timer0Init(void);              // Timer0初始化
void Timer2Init(void);              // Timer2初始化
void PWMInit(uint32_t ui32Freq_Hz); // 产生频率为ui32Freq_Hz的方波

extern uint32_t g_ui32SysClock;

// extern uint8_t mode;

//*******************************************************************************************************
//
// 变量定义
//
//*******************************************************************************************************

// 1s计数结束标志
volatile uint8_t g_ui8INTStatus = 0;

// 保存上一次TIMER0边沿计数值
volatile uint32_t g_ui32TPreCount = 0;

// 保存本次TIMER0边沿计数值
volatile uint32_t g_ui32TCurCount = 0;

// 记录测量的方波频率
uint32_t ui32Freq;

//*******************************************************************************************************
//
// 主程序
//
//*******************************************************************************************************
void Get_Frequency()
{

    // PWMInit(1234);   //  产生3998Hz方波，当用户没有信号发生器时用于产生测试信号,可以自行修改频率

    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer1A 超时中断使能
    IntEnable(INT_TIMER1A);                          // 开启 TIMER1A 中断源
    TimerEnable(TIMER1_BASE, TIMER_A);               // TIMER1 开始计时

    TimerEnable(TIMER0_BASE, TIMER_A); // TIMER0 开始计数

    if (g_ui8INTStatus == 1) // 1s定时结束，开始计算频率
    {
        g_ui8INTStatus = 0;

        ui32Freq = g_ui32TCurCount >= g_ui32TPreCount ? (g_ui32TCurCount - g_ui32TPreCount) : (g_ui32TCurCount - g_ui32TPreCount + 0xFFFF);

        ui32Freq = ui32Freq * 21;
    }

    // 数码管显示数值

    if ((ui32Freq >= FREQUENCY_MIN) && (ui32Freq <= FREQUENCY_MAX))
    {
        digit[4] = ui32Freq / 1000 % 10; // 计算千位数
        digit[5] = ui32Freq / 100 % 10;  // 计算百位数
        digit[6] = ui32Freq / 10 % 10;   // 计算十位数
        digit[7] = ui32Freq % 10;        // 计算个位数
    }
    else // 测量频率超过指定范围
    {
        digit[4] = 'E';
        digit[5] = 'R';
        digit[6] = 'R';
        digit[7] = ' ';
    }
}

//*******************************************************************************************************
//
// 函数原型：void Timer0Init(void)
// 函数功能：设置Timer0为输入边沿（上升沿）计数模式，T0CCP0(PL4)为捕获引脚
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void Timer0Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // 使能TIMER0

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // 使能GPIOL

    GPIOPinConfigure(GPIO_PL4_T0CCP0);                                                       // 配置引脚复用
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4);                                           // 引脚映射
    GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // 将引脚弱上拉

    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP); // 半长定时器，增计数
    // TimerPrescaleSet(TIMER0_BASE, TIMER_A, 255);   // 预分频256
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE); // 初始化配置为捕捉上升沿
}

//*******************************************************************************************************
//
// 函数原型：void Timer1Init(void)
// 函数功能：设置Timer1为一次性定时器，定时周期为1s
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void Timer1Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // TIMER1 使能

    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT); // 设置为 32 位 1次 定时器

    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32SysClock / 21); // TIMER1A装载计数值50ms
}

//*******************************************************************************************************
//
// 函数原型：void TIMER1A_Handler(void)
// 函数功能：Timer1A中断服务程序，记录捕获方波上升沿时定时器（TIMER0）的计数值
// 函数参数：无
// 函数返回值：无
//
//*******************************************************************************************************
void TIMER1A_Handler(void)
{

    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);        // 清除中断标志
    g_ui32TPreCount = g_ui32TCurCount;                     // 保存上一次TIMER0边沿计数值
    g_ui32TCurCount = TimerValueGet(TIMER0_BASE, TIMER_A); // 读取TIMER0边沿计数值
    TimerDisable(TIMER0_BASE, TIMER_A);                    // 停止TIMER0边沿计数
    g_ui8INTStatus = 1;                                    // 1s计数完成
}

//*******************************************************************************************************
//
// 函数原型：void PWMInit(uint32_t ui32Freq_Hz)
// 函数功能：产生频率为ui32Freq_Hz的方波(占空比为50%的PWM)，输出引脚为M0PWM4(PG0)
//          该函数是为了方便用户没有信号发生器时产生测试信号而编写的。
// 函数参数：ui32Freq_Hz 需要产生的方波的频率
// 函数返回值：无
//
//*******************************************************************************************************
void PWMInit(uint32_t ui32Freq_Hz)
{
    /*
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);             // 不分频
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // PWM0使能
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // 使能(允许)PWM0_3的输出
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);             //使能PWM0模块的1号发生器(因为3号PWM是1号发生器产生的)
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ui32SysClock / ui32Freq_Hz); // 根据Freq_Hz设置PWM周期


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // 使能GPIOF
    GPIOPinConfigure(GPIO_PF3_M0PWM3);              // 配置引脚复用
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);    // 引脚映射

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);   //配置PWM发生器
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/ 2)); //设置占空比为50%
*/

    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); // PWM0使能

    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // 使能(允许)PWM0_3的输出
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);             // 使能PWM0模块的1号发生器(因为3号PWM是1号发生器产生的)
    // PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, ui32SysClock / ui32Freq_Hz); // 根据Freq_Hz设置PWM周期

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // 使能GPIOF
    GPIOPinConfigure(GPIO_PF3_M0PWM3);           // 配置引脚复用
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3); // 引脚映射

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC); // 配置PWM发生器
    // PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //设置占空比为50%
}

//
void PWMStart(uint32_t ui32Freq_Hz)
{
    /*
        PWMGenDisable(PWM0_BASE, PWM_GEN_1);     //使能PWM0模块的1号发生器(因为3号PWM是2号发生器产生的)

        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, g_ui32SysClock / ui32Freq_Hz); // 根据Freq_Hz设置PWM周期
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/ 2)); //设置占空比为50%

        PWMGenEnable(PWM0_BASE, PWM_GEN_1);     //使能PWM0模块的1号发生器(因为3号PWM是1号发生器产生的)
    */
    //*******************************************************************************************************
    PWMGenDisable(PWM0_BASE, PWM_GEN_1); // 使能PWM0模块的1号发生器(因为3号PWM是2号发生器产生的)

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ui32SysClock / ui32Freq_Hz);                   // 根据Freq_Hz设置PWM周期
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, (PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 2)); // 设置占空比为50%

    PWMGenEnable(PWM0_BASE, PWM_GEN_1); // 使能PWM0模块的1号发生器(因为3号PWM是1号发生器产生的)
}
