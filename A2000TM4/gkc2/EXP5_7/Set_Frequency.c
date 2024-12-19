//*****************************************************************************
//
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义和函数原型
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型
#include "driverlib/uart.h"       // 与UART有关的宏定义和函数原型

#include "tm1638.h"               // 与控制TM1638芯片有关的宏定义和函数原型

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define V_T200ms	100                  // 0.1s软件定时器溢出值，5个
//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断
void UARTInit(void);        // UART初始化
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
void UART6_Handler(void);   //UART6中断服务程序
void UARTStringPut(uint32_t ui32Base,uint8_t *cMessage);  //向UART模块发送字符串
//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
void Set_Freq(void)
{

   // DevicesInit();            //  MCU器件初始化

   // SysCtlDelay(60 * ( ui32SysClock / 3000)); // 延时>60ms,等待TM1638上电完成
    //TM1638_Init();	          // 初始化TM1638


	//UARTStringPut(UART6_BASE,freq);
	//SysCtlDelay(200 * ( ui32SysClock / 3000));
	//UARTStringPut(UART6_BASE,voice);

}

//*****************************************************************************
//
// 函数原型：void UARTInit(void)
// 函数功能：UART初始化。使能UART6，设置PP0,PP1为UART6 RX,TX引脚；
//          设置波特率及帧格式。
// 函数参数：无
//*****************************************************************************


//*****************************************************************************
//
// 函数原型：void UART6_Handler(void)
// 函数功能：UART6中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
/*
void UART6_Handler(void)
{
    //  uint8_t temp=0;
    
    int32_t uart6_int_status;
    uart6_int_status = UARTIntStatus(UART6_BASE, true); // 取中断状态
    UARTIntClear(UART6_BASE, uart6_int_status); // 清中断标志

}
*/
//*****************************************************************************
//
// 函数原型：void UARTStringPut(uint32_t ui32Base,const char *cMessage)
// 函数功能：向UART模块发送字符串
// 函数参数：ui32Base：UART模块
//          cMessage：待发送字符串  
// 函数返回值：无
//
//*****************************************************************************
void UARTStringPut(uint32_t ui32Base,uint8_t *cMessage)
{
	while(*cMessage != '\0')
		UARTCharPut(ui32Base, *(cMessage++));
}


