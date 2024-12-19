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

#include "DAC6571.h"              // 与控制DAC6571芯片有关的函数

//*************************************************************************************
//
// 宏定义
//
//*************************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTick频率为50Hz，即循环定时周期20ms        

#define V_T500ms	25             // 0.5s软件定时器溢出值，25个20ms

#define V_T40ms	 2                      // 40ms软件定时器溢出值，2个20ms          
#define V_T100ms 5                      // 0.1s软件定时器溢出值，5个20ms          
#define WINDOW_SIZE 30
//*************************************************************************************
//
// 函数原型声明
//
//*************************************************************************************
void GPIOInit(void);        // GPIO初始化
void ADCInit(void);         // ADC初始化
void SysTickInit(void);     // 设置SysTick中断 
void DevicesInit(void);     // MCU器件初始化，注：会调用上述函数
void ADC_Sample(void);      // 获取ADC采样值 
//*************************************************************************************
//
// 变量定义
//
//*************************************************************************************

// 软件定时器计数
uint8_t clock500ms = 0;


// 软件定时器溢出标志
uint8_t clock500ms_flag = 0;

// 软件定时器计数
uint8_t clock40ms = 0;
uint8_t clock100ms = 0;

// 软件定时器溢出标志
uint8_t clock40ms_flag = 0;
uint8_t	clock100ms_flag = 0; 

// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8]={'.',' ',' ','.',' ',' ',' ',' '};

// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
//此时小数点4与0亮
uint8_t pnt = 0x11;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0};

// 系统时钟频率 
uint32_t ui32SysClock;

// 存放从ADC FIFO读取的采样数据 [0-4095]
uint32_t pui32ADC0Value[4];

// 当前按键值
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// DAC6571
uint32_t DAC6571_code = 300;
uint32_t target_current = 1000;
uint32_t DAC6571_voltage = 250;
uint8_t  DAC6571_flag = 0;

uint32_t max_current = 1001;

bool output_flag = 0;
//*************************************************************************************
//
// 主程序
//
//*************************************************************************************
 int main(void)
{
	DevicesInit();            //  MCU器件初始化
	
	while (clock100ms < 3);   // 延时>60ms,等待TM1638上电完成
	TM1638_Init();	          // 初始化TM1638
	
	int datas0[WINDOW_SIZE] = {0}, datas1[WINDOW_SIZE] = {0};  // 存放最近输入的WINDOW_SIZE个整数
  int sum0 = 0, sum1 = 0;                  // 存放最近输入的WINDOW_SIZE个整数的和
  int average0 = 0, average1 = 0;           // 存放最近输入的WINDOW_SIZE个整数的平均值（取整）
  unsigned int index = 0;

  DAC6571_flag = 1;

	while (1)
	{	
		
        if (DAC6571_flag == 1)   // 检查DAC电压是否要变
		{
			DAC6571_flag = 0;

			digit[0] = target_current / 1000 ; 	  // 计算千位数
			digit[1] = target_current / 100 % 10;   // 计算百位数
			digit[2] = target_current / 10 % 10;    // 计算十位数
            digit[3] = target_current % 10;         // 计算个位数

            DAC6571_code = 0.32082228116710876 * target_current - 21.230769230769226;
            
            DAC6571_Fastmode_Operation(DAC6571_code);
		}

        if (clock40ms_flag == 1)        // 检查40ms秒定时是否到
        {
            clock40ms_flag = 0;
            
					//硬件平均
					//ADCHardwareOversampleConfigure(ADC0_BASE, 64);
					
					//利用PC7高低电平测ADC时间
					//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_7);
					
					ADC_Sample();
					
					//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);
					
					pui32ADC0Value[0] = 1.6126958711603034 * pui32ADC0Value[0] - 8.361211292127942 ; //电压 * 5052 / 3274
					pui32ADC0Value[1] = pui32ADC0Value[1] * 8 / 10; //电流 
					
					
					           
/**/
        sum0 += pui32ADC0Value[0];                                                        ;
        sum0 -= datas0[index];
				sum1 += pui32ADC0Value[1];                                                        ;
        sum1 -= datas1[index];	
					;
        datas0[index] = pui32ADC0Value[0];
        datas1[index] = pui32ADC0Value[1];
					
  			index =  (index + 1) % WINDOW_SIZE;            
					;
        average0 = sum0 / WINDOW_SIZE;
				average1 = sum1 / WINDOW_SIZE;
				
				int value_output = 0;

        if(output_flag) value_output = average0;
        else value_output = average1;

/* 
            digit[4] = pui32ADC0Value[0] / 1000; 	  // 显示CH0/PE3的ADC采样值千位数
            digit[5] = pui32ADC0Value[0] / 100 % 10;  // 显示CH0/PE3的ADC采样值百位数
            digit[6] = pui32ADC0Value[0] / 10 % 10;   // 显示CH0/PE3的ADC采样值十位数
            digit[7] = pui32ADC0Value[0] % 10;        // 显示CH0/PE3的ADC采样值个位数

            digit[0] = pui32ADC0Value[1] / 1000; 	  // 显示CH1/PE2的ADC采样值千位数
            digit[1] = pui32ADC0Value[1] / 100 % 10;  // 显示CH1/PE2的ADC采样值百位数
            digit[2] = pui32ADC0Value[1] / 10 % 10;   // 显示CH1/PE2的ADC采样值十位数
            digit[3] = pui32ADC0Value[1] % 10;        // 显示CH1/PE2的ADC采样值个位数  
*/

						digit[4] = value_output / 1000; 	  // 显示CH0/PE3的ADC采样值千位数
            digit[5] = value_output / 100 % 10;  // 显示CH0/PE3的ADC采样值百位数
            digit[6] = value_output / 10 % 10;   // 显示CH0/PE3的ADC采样值十位数
            digit[7] = value_output % 10;        // 显示CH0/PE3的ADC采样值个位数
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
	//配置TM1638芯片管脚
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// 使能端口 K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// 等待端口 K准备完毕		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// 使能端口 M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// 等待端口 M准备完毕	

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);				// 使能端口 C	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)){};		// 等待端口 C准备完毕

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);				// 使能端口 L	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)){};		// 等待端口 L准备完毕
	
   // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
	// 设置端口 C的第7位（PC7）为输出引脚   PC7
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);	
		
	// 设置端口 L的第0,1位（PL0,PL1）为输出引脚		PL0-SDA  PL1-SCL (DAC6571)
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1);
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
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_2);

    // 选用采样序列产生器1，采样起始信号由ADCProcessorTrigger函数触发，优先级为0
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    // 设置采样序列产生器1第0步骤:选择CH0为采样通道
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0);
    
    // 设置采样序列产生器1第1步骤:
    // ADC_CTL_CH1--选择CH1为采样通道， ADC_CTL_END--采样序列的最后一步
    // ADC_CTL_IE--采样结束后产生中断
    ADCSequenceStepConfigure( ADC0_BASE, 1, 1, 
                              ADC_CTL_CH1 | ADC_CTL_END | ADC_CTL_IE);  

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
    while(!ADCIntStatus(ADC0_BASE, 1, false))
    {
    }

    // 清除ADC0中断标志
    ADCIntClear(ADC0_BASE, 1);

    // 读取ADC0采样值，存储到数组pui32ADC0Value中
    ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);

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
    SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
    SysTickEnable();  			// SysTick使能
    SysTickIntEnable();			// SysTick中断允许
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
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   20000000);

  GPIOInit();             // GPIO初始化
  ADCInit();              // ADC初始化
  SysTickInit();          // 设置SysTick中断
  IntMasterEnable();	  // 总中断允许
}

//*************************************************************************************
// 
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*************************************************************************************
void SysTick_Handler(void)       // 定时周期为20ms
{
 
	// 40ms秒钟软定时器计数
	if (++clock40ms >= V_T40ms)
	{
		clock40ms_flag = 1; // 当40ms到时，溢出标志置1
		clock40ms = 0;
	}

    
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
	
    if (key_code != 0)
	{
		if (key_cnt < 4) key_cnt++;   // 按键消抖，4*20ms
		else if (key_cnt == 4)
		{
			
            switch(key_code)
            {
                case 1:     // 加100
                    if (target_current < max_current - 100) 
				    {
					     target_current += 100;
					     DAC6571_flag = 1;
				    }
                    break;
                case 4:    // 减100
                    if (target_current > 100) 
				    {
					    target_current -= 100;
					    DAC6571_flag = 1;
				    }
                    break;
                case 2:    // 加10
                   if (target_current < max_current - 10) 
				    {
					     target_current += 10;
					     DAC6571_flag = 1;
				    }                    
                    break;
                case 5:    // 减10
                   if (target_current >= 10) 
				    {
					    target_current -= 10;
					    DAC6571_flag = 1;
				    }
                    break;
                case 3:    // 加1
                   if (target_current < max_current - 1) 
				    {
					     target_current += 1;
					     DAC6571_flag = 1;
				    }
                    break;
                case 6:    // 减1
                   if (target_current >= 1) 
				    {
					    target_current -= 1;
					    DAC6571_flag = 1;
				    }
                    break;
                case 9:    // 转换输出模式，电压/电流
                    output_flag = !output_flag;
                    break;
                default:
                    break;
            }
            
			key_cnt = 5;   // 按键一直按着，只改变一次
		}
	}
	else key_cnt = 0;
}
