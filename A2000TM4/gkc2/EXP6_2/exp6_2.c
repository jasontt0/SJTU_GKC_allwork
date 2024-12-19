//*****************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: exp5_7.c
// Description: 根据实验参考资料要求，完成接收端功能
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20201228
// Date：2021-5-30
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
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h" // 基址宏定义
#include "inc/hw_types.h"  // 数据类型宏定义，寄存器访问函数

#include "driverlib/debug.h"     // 调试用
#include "driverlib/gpio.h"      // 通用IO口宏定义
#include "driverlib/pin_map.h"   // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"    // 系统控制定义
#include "driverlib/systick.h"   // SysTick Driver 原型
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver 原型
#include "driverlib/uart.h"      // 与UART有关的宏定义和函数原型

#include "JLX12864.c"                //液晶显示屏定义
#include "ADC_Demo_Voltage.c"        //利用AIN2/PE1端口实现单端输入单次ADC采样
#include "Audio_Frequency_Measure.c" //利用PL4/PL5端口实现频率接收
#include "Set_Frequency.c"

#include "tm1638.h" // 与控制TM1638芯片有关的函数

#include "driverlib/adc.h" // 与ADC有关的定义

#include "ADC.h"
//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTick频率为50Hz，即循环定时周期20ms

#define V_T20ms 1
#define V_T40ms 2  // 100ms软件定时器溢出值，2个20ms
#define V_T100ms 5 // 100ms软件定时器溢出值，1个100ms
#define V_T300ms 15
#define V_T1500ms 10 // 1s软件定时器溢出值，20个100ms
#define V_T2s 100    // 2s软件定时器溢出值，100个20ms
// #define V_T300ms 15  // 300ms软件定时器溢出值，3个100ms

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);                                  // GPIO初始化
void SysTickInit(void);                               // 设置SysTick中断
void UARTInit(void);                                  // UART初始化
void DevicesInit(void);                               // MCU器件初始化，注：会调用上述函数
void ui_state_proc(unsigned int ui_state);            // 根据当前模式调用模式处理函数
void ACT0(void);                                      // 模式0
void ACT1(void);                                      // 模式1
void ACT2(void);                                      // 模式2
void in_de(unsigned int w, unsigned char *actstring); // 加减操作函数
void ENTER_detect(void);                              // 操作按键按下监测函数
void LEFT_detect(void);
void RIGHT_detect(void);
void INCREASE_detect(void);
void DECREASE_detect(void);
void CODE_detect(void);                                                                                                        // 任意按键按下监测函数
void initial_device(void);                                                                                                     // 模式0初始化
void char_compare(unsigned char *actstring1, unsigned char *actstring2, unsigned char *actstring3, unsigned char *actstring4); // 频率合法性判断函数
void Get_Volt(void);                                                                                                           // 记录电压值
void Get_Temperature(void);                                                                                                    // 根据FM接收的频率得到发射机传来的温度信息

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock20ms = 0;
uint8_t clock40ms = 0;
uint8_t clock100ms = 0;
uint8_t clock300ms = 0;
uint32_t clock1500ms = 0;
uint32_t clock2s = 0;

// 软件定时器溢出标志
uint8_t clock20ms_flag = 0;
uint8_t clock40ms_flag = 0;
uint8_t clock100ms_flag = 0;
uint8_t clock300ms_flag = 0;
uint32_t clock1500ms_flag = 0;
uint32_t clock2s_flag = 0;

// 记录状态机当前状态
uint8_t lcd_act = 0;

// 测试用计数器
uint32_t test_counter = 0;

// 系统时钟频率
uint32_t ui32SysClock;

// 系统时钟频率
uint32_t g_ui32SysClock;

// 识别按键按下的中间变量
uint8_t key_RIGHT_prestate = 1;
uint8_t key_LEFT_prestate = 1;
uint8_t key_INCREASE_prestate = 1;
uint8_t key_DECREASE_prestate = 1;
uint8_t key_ENTER_prestate = 1;
uint8_t key_CODE_prestate = 1;
uint8_t key_BACK_prestate = 1;

// 按键按下标志
uint8_t key_RIGHT_flag = 0;
uint8_t key_LEFT_flag = 0;
uint8_t key_INCREASE_flag = 0;
uint8_t key_DECREASE_flag = 0;
uint8_t key_ENTER_flag = 0;
uint8_t key_CODE_flag = 0;
uint8_t key_BACK_flag = 0;

// 模式选择
unsigned int ui_state = 0;

// 电压，温度，频率的字符变量
uint8_t volt1[2] = {"0"};
uint8_t volt2[2] = {"0"};
uint8_t volt3[2] = {"0"};
uint8_t volt4[2] = {"0"};
uint8_t temperature1[2] = {"3"};
uint8_t temperature2[2] = {"0"};
uint8_t temperature3[2] = {"5"};
uint8_t freq1[2] = {"1"};
uint8_t freq2[2] = {"0"};
uint8_t freq3[2] = {"1"};
uint8_t freq4[2] = {"7"};

// 光标位置变量
uint8_t position1 = 1;

// 判断字符大小变量
uint8_t compare = 0;

// 5s计时启动变量
uint8_t Cal_Enable = 0;

// 当前按键值
uint8_t key_code = 0;

// 当前温度值
uint32_t temperature = 0;

// 频率设定
uint8_t freq[12] = {"AT+FREQ=876\0"};

uint32_t receive_frequ[200] = {0};
uint8_t k_freq = 0;

uint8_t mode = 0;

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void)
{
    DevicesInit(); //  MCU器件初始化

    while (clock100ms < 3)
        ;          // 延时>60ms,等待TM1638上电完成
    TM1638_Init(); // 初始化TM1638

    LCD_PORT_Init();
    initial_lcd();
    clear_screen(); // clear all dots

    while (1)
    {

        switch (mode)
        {
        case 1:
        {
            PWMGenDisable(PWM0_BASE, PWM_GEN_1); // M0PWM3(PF3)停止产生PWM信号
            while (mode == 1)
            {
                if (clock40ms_flag == 1) // 检查40ms秒定时是否到
                {
                    clock40ms_flag = 0;
                }
                if (clock1500ms_flag == 1) // 检查1.5s秒定时是否到
                {
                    clock1500ms_flag = 0;
                    Get_Temperature(); // 频率采样，并显示在数码管上，记录温度值
                }
                ui_state_proc(ui_state);
                break;
            }
        }
        case 3:
        {
            PWMGenDisable(PWM0_BASE, PWM_GEN_1); // M0PWM3(PF3)停止产生PWM信号
            k_freq = 0;
            while (mode == 3)
            {
                if (clock100ms_flag == 1)
                {
                    clock100ms_flag = 0;
                    Get_Frequency();

                    receive_frequ[k_freq] = ui32Freq;
                    k_freq++;
                }
            }

            break;
        }
        case 7:
        {
            k_freq = 0;
            while (mode == 7)
            {
                if (clock100ms_flag == 1)
                {
                    clock100ms_flag = 0;

                    PWMStart(receive_frequ[k_freq]);
                    k_freq++;
                }
            }

            break;
        }

        case 9:
        {
            k_freq = 0;
            while (mode == 9)
            {
                if (clock100ms_flag == 1)
                {
                    clock100ms_flag = 0;
                    Get_Frequency();

                    PWMStart(ui32Freq);
                }
            }

            break;
        }

        }
    }
}

void UARTInit(void)
{
    // 引脚配置
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6); // 使能UART6模块
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); // 使能端口 P
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP))
        ; // 等待端口 P准备完毕

    GPIOPinConfigure(GPIO_PP0_U6RX); // 设置PP0为UART6 RX引脚
    GPIOPinConfigure(GPIO_PP1_U6TX); // 设置PP1为UART6 TX引脚

    // 设置端口 P的第0,1位（PP0,PP1）为UART引脚
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // 波特率及帧格式设置
    UARTConfigSetExpClk(UART6_BASE,
                        ui32SysClock,
                        9600,                    // 波特率：9600
                        (UART_CONFIG_WLEN_8 |    // 数据位：8
                         UART_CONFIG_STOP_ONE |  // 停止位：1
                         UART_CONFIG_PAR_NONE)); // 校验位：无

}

//*****************************************************************************
//
// 函数原型：void GPIOInit(void)
// 函数功能：GPIO初始化。使能PortK，设置PK4,PK5为输出；使能PortM，设置PM0为输出。
//          （PK4连接TM1638的STB，PK5连接TM1638的DIO，PM0连接TM1638的CLK）
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
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

    // 设置端口 K的第4,5位（PK4,PK5）为输出引脚		PK4-STB  PK5-DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // 设置端口 M的第0位（PM0）为输出引脚   PM0-CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);

    // F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // 使能端口 F
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }; // 等待端口 F准备完毕

    // 设置端口 F的第3位（PF3）为输入出引脚
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_3); // 2改3，In改out

    /*
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // 使能端口 L
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
    {
    }; // 等待端口 L准备完毕

    // 设置端口 L的第3位（PF3）为输入引脚
    //GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_4);
    */
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
    SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
    // SysTickPeriodSet(g_ui32SysClock/SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期20ms
    SysTickEnable();    // SysTick使能
    SysTickIntEnable(); // SysTick中断允许
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
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                      20000000);



    GPIOInit();        // GPIO初始化
    ADCInit();         // ADC初始化
    SysTickInit();     // 设置SysTick中断
    UARTInit();        // 串口初始化
    IntMasterEnable(); // 总中断允许
    Timer0Init();      // Timer0初始化

    PWMInit(1234);

    Timer1Init();

    FPULazyStackingEnable();
    FPUEnable(); // 使能FPU

    IntPrioritySet(INT_TIMER1A, 0x00);   // 设置INT_TIMER1A最高优先级
    IntPrioritySet(INT_TIMER0A, 0x01);   // 设置INT_TIMER0A最高优先级
    IntPrioritySet(FAULT_SYSTICK, 0xe0); // 设置SYSTICK优先级低于INT_TIMER0A的优先级
                                         // IntPrioritySet(INT_GPIOL,0xc0);
}

//*****************************************************************************
//
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void) // 定时周期为20ms
{
    if (++clock20ms >= V_T20ms)
    {
        clock20ms_flag = 1; // 当40ms到时，溢出标志置1
        clock20ms = 0;
    }

    // 40ms软定时器计数
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
    //
    if (++clock300ms >= V_T300ms)
    {
        clock300ms_flag = 1; // 当0.1秒到时，溢出标志置1
        clock300ms = 0;
    }

    // 1.5秒钟软定时器计数
    if (++clock1500ms >= V_T1500ms)
    {
        clock1500ms_flag = 1; // 当1.5秒到时，溢出标志置1
        clock1500ms = 0;
    }

    // 2秒钟软定时器计数
    if (Cal_Enable == 1) // 2秒计时允许
    {
        if (++clock2s >= V_T2s)
        {
            clock2s_flag = 1; // 当2秒到时，溢出标志置1
            Cal_Enable = 0;   // 当2秒到时，允许变量置0
            clock2s = 0;
        }
    }

    // 刷新全部数码管和LED指示灯
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
    // 键号显示在一位数码管上
    key_code = TM1638_Readkeyboard();

    if (key_code != 0)
    {
        switch (key_code)
        {
        case 1:
        {
            mode = 1;
            break;
        }
        case 3:
        {
            mode = 3;
            break;
        }
        case 7:
        {
            mode = 7;
            break;
        }
        case 9:
        {
            mode = 9;
            break;
        }
        default:
            break;
        }
    }

    if (mode == 1)
    {
        ENTER_detect();
        LEFT_detect();
        RIGHT_detect();
        INCREASE_detect();
        DECREASE_detect();
        CODE_detect();
    }
}

//*****************************************************************************
//
// 函数原型：void XXXXX_detect(void)
// 函数功能：按键按下监测函数
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ENTER_detect(void) // 确定键
{
    if (key_code == 5)
    {
        if (key_ENTER_prestate == 1)
            key_ENTER_flag = 1;
        key_ENTER_prestate = 0;
    }
    else
    {
        key_ENTER_prestate = 1;
    }
}

//*****************************************************************************
void LEFT_detect(void) // 向左移动键
{
    if (key_code == 4)
    {
        if (key_LEFT_prestate == 1)
            key_LEFT_flag = 1;
        key_LEFT_prestate = 0;
    }
    else
    {
        key_LEFT_prestate = 1;
    }
}

//*****************************************************************************
void RIGHT_detect(void) // 向右移动键
{
    if (key_code == 6)
    {
        if (key_RIGHT_prestate == 1)
            key_RIGHT_flag = 1;
        key_RIGHT_prestate = 0;
    }
    else
    {
        key_RIGHT_prestate = 1;
    }
}

//*****************************************************************************
void INCREASE_detect(void) //+运算键
{
    if (key_code == 2)
    {
        if (key_INCREASE_prestate == 1)
            key_INCREASE_flag = 1;
        key_INCREASE_prestate = 0;
    }
    else
    {
        key_INCREASE_prestate = 1;
    }
}
void DECREASE_detect(void) //-运算键
{
    if (key_code == 8)
    {
        if (key_DECREASE_prestate == 1)
            key_DECREASE_flag = 1;
        key_DECREASE_prestate = 0;
    }
    else
    {
        key_DECREASE_prestate = 1;
    }
}
void CODE_detect(void) // 其他按键
{
    if (key_code == 1 || key_code == 3 || key_code == 7 || key_code == 9)
    {
        if (key_CODE_prestate == 1)
            key_CODE_flag = 1;
        key_CODE_prestate = 0;
    }
    else
    {
        key_CODE_prestate = 1;
    }
}

//*****************************************************************************
//
// 函数原型：void ui_state_proc(unsigned int ui_state)
// 函数功能：工作画面转移函数
// 函数参数：当前画面序号ui_state
// 函数返回值：无
//
//*****************************************************************************
void ui_state_proc(unsigned int ui_state)
{
    switch (ui_state)
    {
    case 0:
        ACT0();
        break;
    case 1:
        ACT1();
        break;
    case 2:
        ACT2();
        break;

    default:
        ui_state = 0;
        break;
    }
}

//*****************************************************************************
//
// 函数原型：void in_de(unsigned int w, unsigned char* actstring)
// 函数功能：加减按键计算函数
// 函数参数：w：加/减判断  actstring：数字字符
// 函数返回值：无
//
//*****************************************************************************
void in_de(unsigned int w, unsigned char *actstring)
{
    if (w == 1)
    {
        (*actstring)++;
        if (*actstring > '9')
            *actstring = '0';
    }
    else if (w == 2)
    {
        (*actstring)--;
        if (*actstring < '0')
            *actstring = '9';
    }
}

//*****************************************************************************
//
// 函数原型：void char_compare(unsigned char* actstring1, unsigned char* actstring2, unsigned char* actstring3, unsigned char* actstring4)
// 函数功能：工作频率合法性判断函数，频率在[88.0MHz,108.0MHz]之间
// 函数参数：工作频率字符
// 函数返回值：无
//
//*****************************************************************************
void char_compare(unsigned char *actstring1, unsigned char *actstring2, unsigned char *actstring3, unsigned char *actstring4)
{
    if (*actstring1 == '0')
    {
        if (*actstring2 == '8')
        {
            if (*actstring3 == '8' || *actstring3 == '9')
                compare = 1;
            else
                compare = 0;
        }
        else if (*actstring2 == '9')
            compare = 1;
        else
            compare = 0;
    }
    else if (*actstring1 == '1')
    {
        if (*actstring2 == '0')
        {
            if (*actstring3 < '8')
                compare = 1;
            else if (*actstring3 == '8' && *actstring4 == '0')
                compare = 1;
            else
                compare = 0;
        }
        else
            compare = 0;
    }
    else
        compare = 0;
}

//*****************************************************************************
//
// 函数原型：void ACT0(void)
// 函数功能：ACT0界面，接收电压和温度值
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ACT0(void)
{
    display_GB2312_string(3, 1, "温度：", 0);
    display_GB2312_string(3, 81, temperature1, 0);
    display_GB2312_string(3, 89, temperature2, 0);
    display_GB2312_string(3, 97, ".", 0);
    display_GB2312_string(3, 105, temperature3, 0);
    display_GB2312_string(3, 113, "℃", 0);

    // 模式ACT0没有左/右/+/-光标操作

    if (key_ENTER_flag == 1 || key_LEFT_flag == 1 || key_RIGHT_flag == 1 || key_INCREASE_flag == 1 || key_DECREASE_flag == 1 || key_CODE_flag == 1)
    {
        key_ENTER_flag = 0; // 按任意键，转到ACT1
        key_LEFT_flag = 0;
        key_RIGHT_flag = 0;
        key_INCREASE_flag = 0;
        key_DECREASE_flag = 0;
        key_CODE_flag = 0;

        clear_screen();
        display_GB2312_string(1, 1, "载频：", 0); // 默认为频率的百位反白
        display_GB2312_string(1, 41, freq1, 1);
        display_GB2312_string(1, 49, freq2, 0);
        display_GB2312_string(1, 57, freq3, 0);
        display_GB2312_string(1, 65, ".", 0);
        display_GB2312_string(1, 73, freq4, 0);
        display_GB2312_string(1, 81, "MHz", 0);
        display_GB2312_string(7, 97, "确定", 0);
        position1 = 1;
        ui_state = 1;
    }
}

//*****************************************************************************
//
// 函数原型：void ACT1(void)
// 函数功能：ACT1界面，设定接收端频率
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ACT1(void)
{
    if (key_RIGHT_flag) // 光标右移
    {
        key_RIGHT_flag = 0;
        switch (position1)
        {
        case 1:
            display_GB2312_string(1, 41, freq1, 0);
            display_GB2312_string(1, 49, freq2, 1);
            position1 = 2;
            break;
        case 2:
            display_GB2312_string(1, 49, freq2, 0);
            display_GB2312_string(1, 57, freq3, 1);
            position1 = 3;
            break;
        case 3:
            display_GB2312_string(1, 57, freq3, 0);
            display_GB2312_string(1, 73, freq4, 1);
            position1 = 4;
            break;
        case 4:
            display_GB2312_string(1, 73, freq4, 0);
            display_GB2312_string(7, 97, "确定", 1);
            position1 = 5;
            break;
        case 5:
            display_GB2312_string(7, 97, "确定", 0);
            display_GB2312_string(1, 41, freq1, 1);
            position1 = 1;
            break;
        }
    }

    else if (key_LEFT_flag) // 光标左移
    {
        key_LEFT_flag = 0;
        switch (position1)
        {
        case 1:
            display_GB2312_string(1, 41, freq1, 0);
            display_GB2312_string(7, 97, "确定", 1);
            position1 = 5;
            break;
        case 2:
            display_GB2312_string(1, 49, freq2, 0);
            display_GB2312_string(1, 41, freq1, 1);
            position1 = 1;
            break;
        case 3:
            display_GB2312_string(1, 57, freq3, 0);
            display_GB2312_string(1, 49, freq2, 1);
            position1 = 2;
            break;
        case 4:
            display_GB2312_string(1, 73, freq4, 0);
            display_GB2312_string(1, 57, freq3, 1);
            position1 = 3;
            break;
        case 5:
            display_GB2312_string(7, 97, "确定", 0);
            display_GB2312_string(1, 73, freq4, 1);
            position1 = 4;
            break;
        }
    }

    else if (key_INCREASE_flag) // 提高频率
    {
        key_INCREASE_flag = 0;
        if (position1 == 1) // 百位
        {
            in_de(1, freq1);
            display_GB2312_string(1, 41, freq1, 1);
        }
        else if (position1 == 2) // 十位
        {
            in_de(1, freq2);
            display_GB2312_string(1, 49, freq2, 1);
        }
        else if (position1 == 3) // 个位
        {
            in_de(1, freq3);
            display_GB2312_string(1, 57, freq3, 1);
        }
        else if (position1 == 4) // 十分位
        {
            in_de(1, freq4);
            display_GB2312_string(1, 73, freq4, 1);
        }
    }

    else if (key_DECREASE_flag) // 降低频率
    {
        key_DECREASE_flag = 0;
        if (position1 == 1) // 百位
        {
            in_de(2, freq1);
            display_GB2312_string(1, 41, freq1, 1);
        }
        else if (position1 == 2) // 十位
        {
            in_de(2, freq2);
            display_GB2312_string(1, 49, freq2, 1);
        }
        else if (position1 == 3) // 个位
        {
            in_de(2, freq3);
            display_GB2312_string(1, 57, freq3, 1);
        }
        else if (position1 == 4) // 十分位
        {
            in_de(2, freq4);
            display_GB2312_string(1, 73, freq4, 1);
        }
    }

    else if (key_ENTER_flag) // 确定键
    {
        key_ENTER_flag = 0;
        if (position1 == 5)
        {
            char_compare(freq1, freq2, freq3, freq4); // 判断频率是否在规定范围内
            if (compare == 1)                         // 频率值合法，转ACT0，并设定频率
            {
                if (*freq1 == '0') // 频率小于100Hz
                {
                    UARTCharPut(UART6_BASE, 'A');
                    UARTCharPut(UART6_BASE, 'T');
                    UARTCharPut(UART6_BASE, '+');
                    UARTCharPut(UART6_BASE, 'F');
                    UARTCharPut(UART6_BASE, 'R');
                    UARTCharPut(UART6_BASE, 'E');
                    UARTCharPut(UART6_BASE, 'Q');
                    UARTCharPut(UART6_BASE, '=');
                    UARTCharPut(UART6_BASE, *freq2);
                    UARTCharPut(UART6_BASE, *freq3);
                    UARTCharPut(UART6_BASE, *freq4);
                }
                else if (*freq1 == '1') // 频率大于100Hz
                {
                    UARTCharPut(UART6_BASE, 'A');
                    UARTCharPut(UART6_BASE, 'T');
                    UARTCharPut(UART6_BASE, '+');
                    UARTCharPut(UART6_BASE, 'F');
                    UARTCharPut(UART6_BASE, 'R');
                    UARTCharPut(UART6_BASE, 'E');
                    UARTCharPut(UART6_BASE, 'Q');
                    UARTCharPut(UART6_BASE, '=');
                    UARTCharPut(UART6_BASE, *freq1);
                    UARTCharPut(UART6_BASE, *freq2);
                    UARTCharPut(UART6_BASE, *freq3);
                    UARTCharPut(UART6_BASE, *freq4);
                }
                clear_screen();
 
                display_GB2312_string(3, 1, "温度：", 0);
                display_GB2312_string(3, 81, temperature1, 0);
                display_GB2312_string(3, 89, temperature2, 0);
                display_GB2312_string(3, 97, ".", 0);
                display_GB2312_string(3, 105, temperature3, 0);
                display_GB2312_string(3, 113, "℃", 0);
                ui_state = 0;
            }
            else if (compare == 0) // 频率值不合法，转ACT2
            {
                Cal_Enable = 1; // 允许2s计时
                clear_screen();
                display_GB2312_string(1, 1, "频率超范围！", 0);
                ui_state = 2;
            }
        }
    }
    else if (key_CODE_flag)
    {
        key_CODE_flag = 0;
    }
}

//*****************************************************************************
//
// 函数原型：void ACT2(void)
// 函数功能：ACT2界面，频率不合法判断显示
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void ACT2(void)
{
    if (clock2s_flag == 1) // 2s到后，转ACT1
    {
        clock2s_flag = 0;
        clear_screen();
        display_GB2312_string(1, 1, "载频：", 0); // 默认为频率的百位反白
        display_GB2312_string(1, 41, freq1, 1);
        display_GB2312_string(1, 49, freq2, 0);
        display_GB2312_string(1, 57, freq3, 0);
        display_GB2312_string(1, 65, ".", 0);
        display_GB2312_string(1, 73, freq4, 0);
        display_GB2312_string(1, 81, "MHz", 0);
        display_GB2312_string(7, 97, "确定", 0);
        position1 = 1;
        ui_state = 1;
    }
}

//*****************************************************************************
//
// 函数原型：void Get_volt(void)
// 函数功能：根据ADC_Demo.c文件，将测得电压值写入变量volt中
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void Get_Volt(void) // 记录电压值
{
    uint32_t temp;
    ADC_Voltage(); // 电压采样，并显示在数码管上
    temp = ui32ADC0Value * 3300 / 4095;
    (*volt1)++;
    *volt1 = (temp / 1000) % 10 + '0';
    (*volt2)++;
    *volt2 = (temp / 100) % 10 + '0';
    (*volt3)++;
    *volt3 = (temp / 10) % 10 + '0';
    (*volt4)++;
    *volt4 = temp % 10 + '0';
}

//*****************************************************************************
//
// 函数原型：void Get_Temperature(void)
// 函数功能：根据FM接收的频率得到发射机传来的温度信息
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void Get_Temperature(void)
{
    Get_Frequency();
    temperature = ((ui32Freq - 300) * 40) / 300;
    (*temperature1)++;
    *temperature1 = (temperature / 100) % 10 + '0';
    (*temperature2)++;
    *temperature2 = (temperature / 10) % 10 + '0';
    (*temperature3)++;
    *temperature3 = temperature % 10 + '0';
}
