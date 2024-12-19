//**************************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: exp0_debug.c
// Description: LED4(D4-PF0)大约以6000毫秒为周期缓慢闪烁；
//              当按下PUSH1(USR_SW1-PJ0)键，LED4(D4-PF0)大约以100毫秒为周期快速闪烁；
//              松开PUSH1(USR_SW1-PJ0)键，LED4(D4-PF0)恢复以500毫秒为周期缓慢闪烁。
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20201228
// Date：2020-12-28
// History：
//
//**************************************************************************************

#define DEBUG_CONSOLE

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

#include "tm1638.h"               // 与控制TM1638芯片有关的函数

#include "driverlib/uart.h"       // UART相关宏定义
#include "utils/uartstdio.h"      // UART0作为控制台相关函数原型声明  
//**************************************************************************************
//
// 宏定义
//
//**************************************************************************************
//#define  MilliSecond      4000    // 形成1ms时延所需循环次数
//#define  FASTFLASHTIME    100	  // 短延时（100ms）
//#define  SLOWFLASHTIME    6000     // 长延时（1s）

#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

#define V_T100ms	5                   // 100ms软件定时器溢出值，5个20ms
#define V_T6s	300                  // 6s软件定时器溢出值，25个20ms

//**************************************************************************************
//
// 函数原型声明
//
//**************************************************************************************
//void  DelayMilliSec(uint32_t ui32DelaySecond);		// 延迟一定时长，单位为毫秒

void GPIOInit(void);         // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数

void  PF0Flash(uint8_t ui8KeyValue);      // 根据传入的按键值，决定PF0快闪或慢闪
void  InitConsole(void);      // UART0初始化

void switch_100ms(void);//100ms工作函数
void switch_6s(void);//6s工作函数

uint32_t ui32SysClock;
uint8_t ui8KeyValue;

uint32_t clock_6s;
uint32_t clock_100ms;

bool flag_100ms=0;
bool flag_6s=0;

uint8_t ifis_on=1;
//**************************************************************************************
//
// 主程序
//
//**************************************************************************************
int main(void)
{

//**************************************************************************************
//内部时钟源PIOSC(16MHz)经过系统时钟源OSC直接使用，分频为16/12/8 MHz
//	g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT|SYSCTL_USE_OSC,16000000);


//外部时钟源MOSC(25MHz)经过系统时钟源OSC直接使用，分频为25/12/1 MHz
//	g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_OSC , 1000000);


//外部时钟源MOSC(25MHz)经过系统时钟源PLL倍频到480MHz，分频为25/20/8 MHz
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 20000000);


//内部时钟源PIOSC(16MHz)经过系统时钟源PLL倍频到480MHz，分频为20/8/1 MHz
//	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_INT|SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),1000000);
//**************************************************************************************


  DevicesInit();     // MCU器件初始化

#ifdef DEBUG_CONSOLE
  InitConsole();          // UART0初始化
  UARTprintf("Hello Everyone\n");
  UARTprintf("System Clock = %d Hz\n", ui32SysClock);
#endif

GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // 点亮 LED4(D4-PF0)

  while(1)                // 无限循环
    {


#ifdef DEBUG_CONSOLE
      UARTprintf("PJ1= %d\n", ui8KeyValue);
#endif

      PF0Flash(ui8KeyValue);          // 根据传入的按键参数，决定PF0快闪或慢闪
    }
}


/*
//**************************************************************************************
//
// 函数原型：void DelayMilliSec(uint32_t ui32DelaySecond)
// 函数功能：延迟一定时长，单位为毫秒
// 函数参数：ui32DelaySecond：延迟毫秒数
//
//**************************************************************************************
void DelayMilliSec(uint32_t ui32DelaySecond)
{
    uint32_t ui32Loop;

    ui32DelaySecond = ui32DelaySecond * MilliSecond;
    for(ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++){ };
}


*/

//**************************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortF，设置PF0为输出；使能PortJ，设置PJ0为输入
// 函数参数：无
//
//**************************************************************************************
void GPIOInit(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);		   // 使能端口 F
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));	   // 等待端口 F准备完毕

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);		   // 使能端口 J
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}; // 等待端口 J准备完毕

  // 设置端口 F的第0位（PF0）为输出引脚
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

//    // 设置端口 F的第4位（PF4）为输出引脚
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);

//    // 设置端口 F的第0和4位（PF0与PF4）为输出引脚
//    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);

  // 设置端口 J的第0位（PJ0）为输入引脚
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);

  // 端口 J的第0位作为按键输入，类型设置成“推挽上拉”
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0, GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD_WPU);
}


//*****************************************************************************
//
// 函数原型：SysTickInit(void)
// 函数功能：设置SysTick中断
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTickInit(void)
{
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms=1/50s
  SysTickEnable();  			// SysTick使能
  SysTickIntEnable();			// SysTick中断允许
}

//*****************************************************************************
//
// 函数原型：void DevicesInit(void)
// 函数功能：CU器件初始化，包括系统时钟设置、GPIO初始化和SysTick中断设置
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void DevicesInit(void)
{
  // 使用外部25MHz主时钟源，经过PLL，然后分频为20MHz
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |
                                     SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480),
                                    20000000);

  GPIOInit();             // GPIO初始化
  SysTickInit();          // 设置SysTick中断
  IntMasterEnable();		// 总中断允许
}



//*****************************************************************************
//
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms,即每20ms自动进入一次服务函数
{
  ui8KeyValue = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0); // 读取 PJ0 键值  0-按下 1-松开

  ++clock_100ms;
  if(clock_100ms>=V_T100ms)
    {
      flag_100ms=1;
      clock_100ms=0;
    }

  ++clock_6s;
  if(clock_6s >=V_T6s )
    {
      flag_6s=1;
      clock_6s=0;
    }
}


//**************************************************************************************
//
// 函数原型：void PF0Flash(uint8_t ui8KeyValue)
// 函数功能：根据传入的按键值，决定PF0快闪或慢闪。0-快闪，1-慢闪
// 函数参数：ui8KeyValue：按键值
//
//**************************************************************************************
void PF0Flash(uint8_t ui8KeyValue)
{
  //uint32_t ui32DelayTime;

  if (ui8KeyValue	== 0)// PUSH1(USR_SW1-PJ0) 按下
    {
      switch_100ms();
    }
  else switch_6s();                                            // PUSH1(USR_SW1-PJ0) 松开


/*
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // 点亮 LED4(D4-PF0)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4); // 点亮 LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_0|GPIO_PIN_4); // 点亮 LED4(D4-PF0),LED3(D3-PF4)

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_0); //


  DelayMilliSec(ui32DelayTime);                          // 延时ui32DelayTime毫秒

  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // 关闭 LED4(D4-PF0)

//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00);        // 关闭 LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, 0x00); // 关闭 LED4(D4-PF0),LED3(D3-PF4)
//    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_4); //


  DelayMilliSec(ui32DelayTime);                          // 延时ui32DelayTime毫秒     !!!!不能用
*/
}


//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void InitConsole(void)
{
  //
  // Enable GPIO port A which is used for UART0 pins.
  // TODO: change this to whichever GPIO port you are using.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  //
  // Configure the pin muxing for UART0 functions on port A0 and A1.
  // This step is not necessary if your part does not support pin muxing.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);

  //
  // Enable UART0 so that we can configure the clock.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

  //
  // Select the alternate (UART) function for these pins.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //
  // Initialize the UART for console I/O.
  //
  UARTStdioConfig(0, 115200, ui32SysClock);

}


void switch_100ms(void)	//100ms工作函数
{
  if(flag_100ms && ifis_on==1)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // 关闭 LED4(D4-PF0)
      ifis_on=0;
      flag_100ms=0;
    }
  else if(flag_100ms && ifis_on==0)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // 点亮 LED4(D4-PF0)
      ifis_on=1;
      flag_100ms=0;
    }
}

void switch_6s(void)	//6s工作函数
{
  if(flag_6s && ifis_on==1)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // 关闭 LED4(D4-PF0)
      ifis_on=0;
      flag_6s=0;
    }
  else if(flag_6s && ifis_on==0)
    {
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); // 点亮 LED4(D4-PF0)
      ifis_on=1;
      flag_6s=0;
    }
}