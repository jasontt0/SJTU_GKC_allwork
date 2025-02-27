//*****************************************************************************
//
// Copyright: 2019-2021, 上海交通大学工程实践与科技创新III-A教学组
// File name: dac_demo.c
// Description: 
//    1.本代码可用于初步检查DAC6571芯片功能是否正常，PL0须连线DAC6571之SDA，PL1连线SCL；
//    2.开机或复位后，DAC编码置为（十进制）1023，底板上右边4位数码管显示该数值；
//    3.由1号和4号键分别控制（十进制）DAC编码其加100和减100；
//    4.由2号和5号键分别控制（十进制）DAC编码其加10和减10；
//    5.由3号和6号键分别控制（十进制）DAC编码其加1和减1；
//    6.代码脱胎于课程初始DEMO程序，所以部分保留了它的代码功能或痕迹；
// Author:	上海交通大学工程实践与科技创新III-A教学组（孟、袁）
// Version: 1.1.0.20210930 
// Date：2021-9-30
// History：2021-9-31修改完善注释（袁）
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
#include "driverlib/sysctl.h"     // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型

#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#include "DAC6571.h"              // 与控制DAC6571芯片有关的函数

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms

#define V_T100ms	5              // 0.1s软件定时器溢出值，5个20ms
#define V_T500ms	25             // 0.5s软件定时器溢出值，25个20ms

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);        // GPIO初始化
void SysTickInit(void);     // 设置SysTick中断 
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;


// 软件定时器溢出标志
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;

// 测试用计数器
uint32_t test_counter = 0;

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={' ',' ',' ',' ','_',' ','_',' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x0;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// 当前按键值
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// DAC6571
uint32_t DAC6571_code = 512;
uint32_t DAC6571_voltage = 250;
uint8_t  DAC6571_flag = 0;


// 系统时钟频率 
uint32_t ui32SysClock;

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
 int main(void)
{
	uint8_t temp,i;

	DevicesInit();            //  MCU器件初始化
	
	while (clock100ms < 3);   // 延时>60ms,等待TM1638上电完成
	TM1638_Init();	          // 初始化TM1638
	
    DAC6571_flag = 1;
    
	while (1)
	{				
        if (DAC6571_flag == 1)   // 检查DAC电压是否要变
		{
			DAC6571_flag = 0;

			digit[0] = DAC6571_code / 1000 ; 	  // 计算千位数
			digit[1] = DAC6571_code / 100 % 10;   // 计算百位数
			digit[2] = DAC6571_code / 10 % 10;    // 计算十位数
            digit[3] = DAC6571_code % 10;         // 计算个位数
            
            DAC6571_Fastmode_Operation(DAC6571_code);
		}

        //	走马灯显示部分
		if (clock500ms_flag == 1)   // 检查0.5秒定时是否到
		{
			clock500ms_flag = 0;
			// 8个指示灯以走马灯方式，每0.5秒向右（循环）移动一格
			temp = led[0];
			for (i = 0; i < 7; i++) led[i] = led[i + 1];
			led[7] = temp;
		}
	}
	
}

//*****************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （底板走线上，PK4连接着TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
//          （通过人工跳接，PL0须连线DAC6571之SDA，PL1连线SCL）
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void GPIOInit(void)
{
	//配置用于控制TM1638芯片、DAC6571芯片的管脚
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// 使能端口 K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// 等待端口 K准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// 等待端口 M准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);				// 使能端口 L	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)){};		// 等待端口 L准备完毕		
    
    // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	


    // 设置端口 L的第0,1位（PL0,PL1）为输出引脚		PL0-SDA  PL1-SCL (DAC6571)
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1);
        
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
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
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
	                                   1000000);

	GPIOInit();             // GPIO初始化
    SysTickInit();          // 设置SysTick中断
    IntMasterEnable();			// 总中断允许
}

//*****************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms
{
	// 0.1秒钟软定时器计数
	if (++clock100ms >= V_T100ms)
	{
		clock100ms_flag = 1; // 当0.1秒到时，溢出标志置1
		clock100ms = 0;
	}
	
 	// 0.5秒钟软定时器计数
	if (++clock500ms >= V_T500ms)
	{
		clock500ms_flag = 1; // 当0.5秒到时，溢出标志置1
		clock500ms = 0;
	}
	
	// 刷新全部数码管和LED指示灯
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在一位数码管上
	key_code = TM1638_Readkeyboard();

//	if (key_code != 0)
//	{
//		if (key_cnt < 4) key_cnt++;   // 按键消抖，4*20ms
//		else if (key_cnt == 4)
//		{
//			if (key_code == 1)      // 加1
//			{
//				if (DAC6571_code < DAC6571_code_max) 
//				{
//					DAC6571_code++;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 2)  // 减1
//			{
//				if (DAC6571_code > 0) 
//				{
//					DAC6571_code--;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 3)  // 加10
//			{
//				if (DAC6571_code < DAC6571_code_max - 10) 
//				{
//					DAC6571_code += 10;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 4)   // 减10
//			{
//				if (DAC6571_code > 10) 
//				{
//					DAC6571_code -= 10;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 5)   // 加100
//			{
//				if (DAC6571_code < DAC6571_code_max - 100) 
//				{
//					DAC6571_code += 100;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 6)   // 减100
//			{
//				if (DAC6571_code > 100) 
//				{
//					DAC6571_code -= 100;
//					DAC6571_flag = 1;
//				}
//			}

//			key_cnt = 5;   // 按键一直按着，只改变一次
//		}
//	}
//	else key_cnt = 0;

	if (key_code != 0)
	{
		if (key_cnt < 4) key_cnt++;   // 按键消抖，4*20ms
		else if (key_cnt == 4)
		{
			
            switch(key_code)
            {
                case 1:     // 加100
                    if (DAC6571_code < DAC6571_code_max - 100) 
				    {
					     DAC6571_code += 100;
					     DAC6571_flag = 1;
				    }
                    break;
                case 4:    // 减100
                    if (DAC6571_code > 100) 
				    {
					    DAC6571_code -= 100;
					    DAC6571_flag = 1;
				    }
                    break;
                case 2:    // 加10
                   if (DAC6571_code < DAC6571_code_max - 10) 
				    {
					     DAC6571_code += 10;
					     DAC6571_flag = 1;
				    }                    
                    break;
                case 5:    // 减10
                   if (DAC6571_code >= 10) 
				    {
					    DAC6571_code -= 10;
					    DAC6571_flag = 1;
				    }
                    break;
                case 3:    // 加1
                   if (DAC6571_code < DAC6571_code_max - 1) 
				    {
					     DAC6571_code += 1;
					     DAC6571_flag = 1;
				    }
                    break;
                case 6:    // 减1
                   if (DAC6571_code >= 1) 
				    {
					    DAC6571_code -= 1;
					    DAC6571_flag = 1;
				    }
                    break;
                default:
                    break;
            }
            
			key_cnt = 5;   // 按键一直按着，只改变一次
		}
	}
	else key_cnt = 0;
       
	digit[5] = key_code;   // 按键值

}
