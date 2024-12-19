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

//**************************************************************************************
//
// 头文件
//
//**************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"     // 系统控制宏定义


#include "driverlib/uart.h"       // UART相关宏定义
#include "utils/uartstdio.h"      // UART0作为控制台相关函数原型声明
// uartstdio.h和uartstdio.c拷贝到utils目录下
//**************************************************************************************
//
// 宏定义
//
//**************************************************************************************
#define  MilliSecond      4000    // 形成1ms时延所需循环次数 
#define  FASTFLASHTIME    150	  // 短延时（100ms）
#define  SLOWFLASHTIME    6000     // 长延时（6s）

//**************************************************************************************
//
// 函数原型声明
//
//**************************************************************************************
void  DelayMilliSec(uint32_t ui32DelaySecond);		// 延迟一定时长，单位为毫秒
void  GPIOInit(void);                               // GPIO初始化
void  PN_Switch(uint8_t ui8KeyValue_0, uint8_t ui8KeyValue_1);      // 根据传入的按键值，决定PF0快闪或慢闪
void  InitConsole(void);      // UART0初始化


uint32_t g_ui32SysClock;
//**************************************************************************************
//
// 主程序
//
//**************************************************************************************
int main(void)
{
  uint8_t ui8KeyValue_0, ui8KeyValue_1;

//**************************************************************************************
//内部时钟源PIOSC(16MHz)经过系统时钟源OSC直接使用，分频为16/12/8 MHz
//	g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT|SYSCTL_USE_OSC,12000000);


//外部时钟源MOSC(25MHz)经过系统时钟源OSC直接使用，分频为25/12/1 MHz
//	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_OSC | SYSCTL_CFG_VCO_480), 1000000);


//外部时钟源MOSC(25MHz)经过系统时钟源PLL倍频到480MHz，分频为25/20/8 MHz
  g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 25000000);


//内部时钟源PIOSC(16MHz)经过系统时钟源PLL倍频到480MHz，分频为20/8/1 MHz
//    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_INT|SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),1000000);
//**************************************************************************************


  GPIOInit();             // GPIO初始化

#ifdef DEBUG_CONSOLE
  InitConsole();          // UART0初始化
  UARTprintf("Hello Everyone\n");
  UARTprintf("System Clock = %d Hz\n", g_ui32SysClock);
#endif

  while(1)// 无限循环
    {

      // 读取 PJ0 键值  0-按下 1-松开
      ui8KeyValue_0 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0);
      ui8KeyValue_1 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1);

#ifdef DEBUG_CONSOLE
      UARTprintf("PJ0= %d\t%d\n", ui8KeyValue_0,ui8KeyValue_1);
#endif

      PN_Switch(ui8KeyValue_0, ui8KeyValue_1);          // 根据传入的按键参数，决定PF0快闪或慢闪
    }
}


//**************************************************************************************
//
// 函数原型：void DelayMilliSec(uint32_t ui32DelaySecond)
// 函数功能：延迟一定时长，单位为毫秒
// 函数参数：ui32DelaySecond：延迟毫秒数

void DelayMilliSec(uint32_t ui32DelaySecond)
{
  uint32_t ui32Loop;

  ui32DelaySecond = ui32DelaySecond * MilliSecond;
  for(ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++) { };
}
//**************************************************************************************




//**************************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortF，设置PF0为输出；使能PortJ，设置PJ0为输入
// 函数参数：无
//
//**************************************************************************************
void GPIOInit(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);		   // 使能端口 F
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));	   // 等待端口 F准备完毕

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);		   // 使能端口 J
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}; // 等待端口 J准备完毕

  // 设置端口 N 的第0位（PF0）为输出引脚
//    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

//    // 设置端口 N 的第4位（PF4）为输出引脚
//    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

//    // 设置端口 N的第0和4位（PF0与PF4）为输出引脚
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1);

  // 设置端口 J的第0位（PJ0）为输入引脚
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);

  // 端口 J的第0位作为按键输入，类型设置成“推挽上拉”
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  // 端口 J的第1位作为按键输入，类型设置成“推挽上拉”
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

}

//**************************************************************************************
//
// 函数原型：void PF0Flash(uint8_t ui8KeyValue)
// 函数功能：根据传入的按键值，决定PF0快闪或慢闪。0-快闪，1-慢闪
// 函数参数：ui8KeyValue：按键值
//
//**************************************************************************************
void PN_Switch(uint8_t ui8KeyValue_0, uint8_t ui8KeyValue_1)
{
  uint32_t ui32DelayTime;

  if (ui8KeyValue_0	== 0) GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); //  PUSH1(USR_SW1-PJ0) 按下,点亮 D4-PN0
  else GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00); // 关闭 D4-PN0

  if (ui8KeyValue_1	== 0) GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1); //  PUSH1(USR_SW1-PJ0) 按下,点亮 D4-PN1
  else GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00);        // 关闭 D4-PN1



  /*
  	//DelayMilliSec(ui32DelayTime);                          // 延时ui32DelayTime毫秒

      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // 关闭 LED4(D4-PF0)

  //    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00);        // 关闭 LED3(D3-PF4)
  //    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, 0x00); // 关闭 LED4(D4-PF0),LED3(D3-PF4)
  //    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_4); //


      DelayMilliSec(ui32DelayTime);                          // 延时ui32DelayTime毫秒
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
  UARTStdioConfig(0, 115200, g_ui32SysClock);

}
