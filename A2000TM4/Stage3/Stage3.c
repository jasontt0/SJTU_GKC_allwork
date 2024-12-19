// //*****************************************************************************
// // for 稳压源、稳流源均流
// // Description:
// // 稳压源J2连接PE0，稳压源J1连接PE1，稳流源J1连接PE2，稳流源J7连接PB2，稳流源J6连接PB3
// // 2.PE0-电压；PE1-稳流源电流；PE2-稳压源电流
// // 注意：输入电压值范围必须为[0-3.3V]，否则会烧坏端口。
// //*****************************************************************************

// //*****************************************************************************
// //
// // 头文件
// //
// //*****************************************************************************
// #include <stdint.h>
// #include <stdbool.h>
// #include "inc/hw_memmap.h"       // 基址宏定义
// #include "inc/hw_types.h"        // 数据类型宏定义，寄存器访问函数
// #include "driverlib/debug.h"     // 调试用
// #include "driverlib/gpio.h"      // 通用 IO 口宏定义
// #include "driverlib/pin_map.h"   // TM4C 系列 MCU 外围设备管脚宏定义
// #include "driverlib/sysctl.h"    // 系统控制定义
// #include "driverlib/systick.h"   // SysTick Driver 原型
// #include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver 原型
// #include "driverlib/adc.h"       // 与 ADC 有关的定义

// #include "tm1638.h" // 与控制 TM1638 芯片有关的函数

// //*****************************************************************************
// //
// // 宏定义
// //
// //*****************************************************************************
// #define SYSTICK_FREQUENCY 50 // SysTick 频率为 50Hz，即循环定时周期 20ms

// #define V_T100ms 5 // 0.1s 软件定时器溢出值，5 个 20ms
// #define V_T300ms 15
// #define V_T2s 100

// #define DAC6571_code_max 1023 // 10 位 ADC
// #define DAC6571_address 0x98  // 1001 10 A0 0 A0=0
// #define WINDOW_SIZE 7
// //*****************************************************************************
// //
// // 函数原型声明
// //
// //*****************************************************************************
// void GPIOInit(void);    // GPIO 初始化
// void DACInit(void);     // DAC
// void ADCInit(void);     // ADC 初始化
// void SysTickInit(void); // 设置 SysTick 中断
// void DevicesInit(void); // MCU 器件初始化，注：会调用上述函数
// void DAC6571_Byte_Transmission(uint8_t byte_data);
// void DAC6571_Fastmode_Operation(void);
// void ADC0_Sample(void); // 获取 ADC0 采样
// void SetCURRENT(void);
// void DisplayPower(uint32_t averagePower);
// void UpdateAveragePower(uint32_t newPower);
// void maxDisplayPower(uint32_t averagePower);
// void Displayomh(uint32_t currentomh);
// #define POWER_SAMPLES 10 // 这里的 10 可以根据需要调整
// uint8_t powerSampleIndex = 0;
// uint32_t powerSamples[POWER_SAMPLES];
// uint32_t averagePower = 0;
// uint32_t sum, i = 0;
// uint32_t currentPower = 0;
// uint32_t maxPower = 0;

// //*****************************************************************************
// //
// // 变量定义
// //
// //*****************************************************************************

// // 软件定时器计数
// uint8_t clock100ms = 0;
// uint8_t clock300ms = 0;
// uint8_t clock2s = 0;
// // 软件定时器溢出标志
// uint8_t clock20ms_flag = 0;
// uint8_t clock100ms_flag = 0;
// uint8_t clock300ms_flag = 0;
// uint8_t clock2s_flag = 0;
// // 8 位数码管显示的数字或字母符号
// // 注：板上数码位从左到右序号排列为 4、5、6、7、0、1、2、3
// uint8_t digit[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
// uint8_t digit_u[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_ic[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_iu[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_dac[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_id[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_aP[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_mP[4] = {' ', ' ', ' ', ' '};
// uint8_t digit_omh[4] = {' ', ' ', ' ', ' '};
// // 模式转换，阶段转换
// uint8_t mode = 1; // 1-ic,iu;2-dac,id;3-u//1-u,ic;2-dac,id;
// uint8_t state = 2;
// uint8_t mode_flag = 0;

// // 8 位小数点 1 亮 0 灭
// // 注：板上数码位小数点从左到右序号排列为 4、5、6、7、0、1、2、3
// uint8_t pnt = 0x00;

// // 8 个 LED 指示灯状态，0 灭，1 亮
// // 注：板上指示灯从左到右序号排列为 7、6、5、4、3、2、1、0
// // 对应元件 LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
// uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0};

// // 按键值
// uint8_t key_code = 0;
// uint8_t key_cnt = 0;

// // DAC6571
// uint32_t dac6571_code = 0;
// uint8_t dac6571_flag = 0;

// // 系统时钟频率
// uint32_t ui32SysClock;

// // ADC 采样值[0-4095]
// uint32_t ADCvalue0;
// uint32_t ADCvalue1;
// uint32_t ADCvalue2;
// uint32_t ui32ADC0Value[3];

// // 电压值(单位为 0.01V) [0.00-3.30]
// uint32_t AllVoltage;
// uint32_t averagePower;
// // 电流值(单位为 0.01A)
// uint32_t CCCurrent = 0;
// uint32_t CUCurrent = 0;
// uint32_t CUCurrent1 = 0;
// uint32_t currentomh = 0;
// uint8_t window_count_1 = 0;
// uint32_t values_1[WINDOW_SIZE] = {0};
// uint8_t window_count_2 = 0;
// uint32_t values_2[WINDOW_SIZE] = {0};
// uint8_t window_count_3 = 0;
// uint32_t values_3[WINDOW_SIZE] = {0};

// uint32_t sliding_window_filter_1(uint32_t value)
// {
//   uint8_t i;
//   uint32_t sum;

//   if (window_count_1 < WINDOW_SIZE)
//   {
//     values_1[window_count_1++] = value;
//   }

//   else
//   {
//     for (i = 1; i < WINDOW_SIZE; i++)
//     {
//       values_1[i - 1] = values_1[i];
//     }
//     values_1[WINDOW_SIZE - 1] = value;
//   }

//   sum = 0;
//   for (i = 0; i < window_count_1; i++)
//   {
//     sum += values_1[i];
//   }
//   return sum / window_count_1;
// }

// uint32_t sliding_window_filter_2(uint32_t value)
// {
//   uint8_t i;
//   uint32_t sum;

//   if (window_count_2 < WINDOW_SIZE)
//   {
//     values_2[window_count_2++] = value;
//   }

//   else
//   {
//     for (i = 1; i < WINDOW_SIZE; i++)
//     {
//       values_2[i - 1] = values_2[i];
//     }
//     values_2[WINDOW_SIZE - 1] = value;
//   }
//   sum = 0;
//   for (i = 0; i < window_count_2; i++)
//   {
//     sum += values_2[i];
//   }
//   return sum / window_count_2;
// }

// uint32_t sliding_window_filter_3(uint32_t value)
// {
//   uint8_t i;
//   uint32_t sum;

//   if (window_count_3 < WINDOW_SIZE)
//   {
//     values_3[window_count_3++] = value;
//   }

//   else
//   {
//     for (i = 1; i < WINDOW_SIZE; i++)
//     {
//       values_3[i - 1] = values_3[i];
//     }
//     values_3[WINDOW_SIZE - 1] = value;
//   }

//   sum = 0;
//   for (i = 0; i < window_count_3; i++)
//   {
//     sum += values_3[i];
//   }
//   return sum / window_count_3;
// }
// //***************************************************************************
// //
// // 主程序
// //
// //***************************************************************************
// int main(void)
// {

//   DevicesInit(); // MCU 器件初始化

//   while (clock100ms < 3)
//     ;            // 延时>60ms,等待 TM1638 上电完成
//   TM1638_Init(); // 初始化 TM1638

//   while (1)
//   {

//     if (mode_flag == 1)
//     {
//       mode_flag = 0;
//       if (mode < 4)
//         mode++;
//       else
//         mode = 1;
//     }

//     // 采样
//     if (clock20ms_flag == 1) // 检查 20ms 秒定时是否到
//     {
//       clock20ms_flag = 0;

//       ADC0_Sample();
//       ADCvalue0 = ui32ADC0Value[0];
//       ADCvalue1 = ui32ADC0Value[2];
//       ADCvalue2 = ui32ADC0Value[1];

//       ADCvalue0 = ADCvalue0 * 6600 / 4095;
//       ADCvalue1 = ADCvalue1 * 3300 / 4095;
//       ADCvalue2 = ADCvalue2 * 3300 / 4095;

//       ADCvalue1 = ADCvalue1 * 1.04 - 34;
//       ADCvalue2 = 1.043 * ADCvalue2 + 45;
//       AllVoltage = sliding_window_filter_1(ADCvalue0) + 10; // 电压
//       CCCurrent = sliding_window_filter_2(ADCvalue1);       // 稳流源电流
//       CUCurrent = sliding_window_filter_3(ADCvalue2);       // 稳压源电流
//       CUCurrent1 = CUCurrent - 10;
//     }

//     digit_u[0] = (AllVoltage / 1000) % 10; // 显示电压值千位
//     digit_u[1] = (AllVoltage / 100) % 10;  // 显示电压值百位
//     digit_u[2] = (AllVoltage / 10) % 10;   // 显示电压值十位
//     digit_u[3] = AllVoltage % 10;          // 显示电压值个位

//     digit_ic[0] = (CCCurrent / 1000) % 10; // 显示电流值千位
//     digit_ic[1] = (CCCurrent / 100) % 10;  // 显示电流值百位
//     digit_ic[2] = (CCCurrent / 10) % 10;   // 显示电流值十位
//     digit_ic[3] = CCCurrent % 10;          // 显示电流值个位数

//     digit_iu[0] = (CUCurrent1 / 1000) % 10; // 显示电流值千位
//     digit_iu[1] = (CUCurrent1 / 100) % 10;  // 显示电流值百位
//     digit_iu[2] = (CUCurrent1 / 10) % 10;   // 显示电流值十位
//     digit_iu[3] = CUCurrent1 % 10;          // 显示电流值个位数

//     if (clock100ms_flag == 1)
//     {
//       clock100ms_flag = 0;
//       dac6571_flag = 1;
//     }

//     if (dac6571_flag == 1) // 检查 DAC 电压是否要变
//     {
//       dac6571_flag = 0;
//       SetCURRENT();
//       DAC6571_Fastmode_Operation();

//       digit_dac[0] = (dac6571_code / 1000) % 10; // 计算千位 0-1023
//       digit_dac[1] = (dac6571_code / 100) % 10;  // 计算百位
//       digit_dac[2] = (dac6571_code / 10) % 10;   // 计算十位
//       digit_dac[3] = dac6571_code % 10;          // 计算个位
//     }
//     currentPower = (AllVoltage * (CUCurrent + CCCurrent)) * 0.001; // 计算当前总功率
//     UpdateAveragePower(currentPower);                              // 更新平均功率
//     DisplayPower(averagePower);
//     maxDisplayPower(averagePower);
//     currentomh = AllVoltage * 100 / (CUCurrent + CCCurrent); // 计算当前阻值
//     Displayomh(currentomh);
//     // 选择数码管、LED 显示内容

//     if (clock2s_flag == 1)
//     {
//       clock2s_flag = 0;
//       switch (mode)
//       {
//       case 1:
//         digit[0] = digit_iu[0];
//         digit[1] = digit_iu[1];
//         digit[2] = digit_iu[2];
//         digit[3] = digit_iu[3];
//         digit[4] = digit_ic[0];
//         digit[5] = digit_ic[1];
//         digit[6] = digit_ic[2];
//         digit[7] = digit_ic[3];
//         pnt = 0x11;
//         led[7] = 0;
//         led[6] = 1;
//         led[5] = 0;
//         led[4] = 0;
//         led[3] = 0;
//         led[2] = 0;
//         led[1] = 1;
//         led[0] = 0;
//         break;
//       case 2:
//         digit[0] = digit_dac[0];
//         digit[1] = digit_dac[1];
//         digit[2] = digit_dac[2];
//         digit[3] = digit_dac[3];
//         digit[4] = digit_u[0];
//         digit[5] = digit_u[1];
//         digit[6] = digit_u[2];
//         digit[7] = digit_u[3];
//         pnt = 0x10;
//         led[7] = 1;
//         led[6] = 0;
//         led[5] = 0;
//         led[4] = 0;
//         led[3] = 0;
//         led[2] = 0;
//         led[1] = 0;
//         led[0] = 0;
//         break;
//       case 3:
//         digit[0] = digit_mP[0];
//         digit[1] = digit_mP[1];
//         digit[2] = digit_mP[2];
//         digit[3] = digit_mP[3];
//         digit[4] = digit_aP[0];
//         digit[5] = digit_aP[1];
//         digit[6] = digit_aP[2];
//         digit[7] = digit_aP[3];
//         pnt = 0x11;
//         break;
//       case 4:
//         digit[0] = 0;
//         digit[1] = 0;
//         digit[2] = 5;
//         digit[3] = 2;
//         digit[4] = digit_omh[0];
//         digit[5] = digit_omh[1];
//         digit[6] = digit_omh[2];
//         digit[7] = digit_omh[3];
//         pnt = 0x20;
//       default:
//         break;
//       }
//     }

//     // switch (mode)
//     // {
//     // case 1:
//     //   digit[0] = digit_iu[0];
//     //   digit[1] = digit_iu[1];
//     //   digit[2] = digit_iu[2];
//     //   digit[3] = digit_iu[3];
//     //   digit[4] = digit_ic[0];
//     //   digit[5] = digit_ic[1];
//     //   digit[6] = digit_ic[2];
//     //   digit[7] = digit_ic[3];
//     //   pnt = 0x11;
//     //   led[7] = 0;
//     //   led[6] = 1;
//     //   led[5] = 0;
//     //   led[4] = 0;
//     //   led[3] = 0;
//     //   led[2] = 0;
//     //   led[1] = 1;
//     //   led[0] = 0;
//     //   break;
//     // case 2:
//     //   digit[0] = digit_dac[0];
//     //   digit[1] = digit_dac[1];
//     //   digit[2] = digit_dac[2];
//     //   digit[3] = digit_dac[3];
//     //   digit[4] = digit_u[0];
//     //   digit[5] = digit_u[1];
//     //   digit[6] = digit_u[2];
//     //   digit[7] = digit_u[3];
//     //   pnt = 0x10;
//     //   led[7] = 1;
//     //   led[6] = 0;
//     //   led[5] = 0;
//     //   led[4] = 0;
//     //   led[3] = 0;
//     //   led[2] = 0;
//     //   led[1] = 0;
//     //   led[0] = 0;
//     //   break;
//     // case 3:
//     //   digit[0] = digit_mP[0];
//     //   digit[1] = digit_mP[1];
//     //   digit[2] = digit_mP[2];
//     //   digit[3] = digit_mP[3];
//     //   digit[4] = digit_aP[0];
//     //   digit[5] = digit_aP[1];
//     //   digit[6] = digit_aP[2];
//     //   digit[7] = digit_aP[3];
//     //   pnt = 0x11;
//     //   break;
//     // case 4:
//     //   digit[0] = 0;
//     //   digit[1] = 0;
//     //   digit[2] = 5;
//     //   digit[3] = 2;
//     //   digit[4] = digit_omh[0];
//     //   digit[5] = digit_omh[1];
//     //   digit[6] = digit_omh[2];
//     //   digit[7] = digit_omh[3];
//     //   pnt = 0x20;
//     // default:
//     //   break;
//     // }
//   }
// }

// //***************************************************************************
// //
// // 函数原型：void GPIOInit(void)
// // 函数功能：GPIO 初始化。使能 PortK，设置 PK4,PK5 为输出；使能 PortM，设置 PM0 为输
// // （PK4 连接 TM1638 的 STB，PK5 连接 TM1638 的 DIO，PM0 连接 TM1638 的

// // 函数参数：无
// // 函数返回值：无
// //
// //***************************************************************************
// void GPIOInit(void)
// {
//   // 配置 TM1638 芯片管脚
//   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); // 使能端
//   while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
//   {
//   }; // 等待端口 K

//   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); // 使能端
//   while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
//   {
//   }; // 等待端口 M

//   // 设置端口 K 的第 4,5 位（PK4,PK5）为输出引脚 PK4-STB PK5-DIO
//   GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
//   // 设置端口 M 的第 0 位（PM0）为输出引脚 PM0-CLK
//   GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);
// }
// void DACInit(void)
// {
//   // 配置 TM1638 芯片管脚
//   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // 使能端
//   while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
//   {
//   }; // 等待端口 B

//   // 设置 PB2,PB3 为输出引脚
//   GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1);
// }
// //***************************************************************************
// //
// // 函数原型：void ADCInit(void)
// // 函数功能：ADC0、1 初始化。
// // 选择 AIN3/PE0、AIN2/PE1 作为 ADC 采样输入端口，采用单端口输入单次采样
// // 函数参数：无
// // 函数返回值：无
// //
// //***************************************************************************
// void ADCInit(void)
// {
//   // 使能 ADC0 模块
//   SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

//   // 使用 PE0、PE1、PE2 端口作为 ADC 输入
//   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

//   GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

//   // 单次采样(sample sequence 2 方式，4 个采样点)
//   ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);

//   //
//   // Configure step 0 on sequence 3. Sample channel 0 (ADC_CTL_CH0) in
//   // single-ended mode (default) and configure the interrupt flag
//   // (ADC_CTL_IE) to be set when the sample is done. Tell the ADC logic
//   // that this is the last conversion on sequence 3 (ADC_CTL_END). Sequenc

//   // 3 has only one programmable step. Sequence 1 and 2 have 4 steps, and
//   // sequence 0 has 8 programmable steps. Since we are only doing a single
//   // conversion using sequence 3 we will only configure step 0. For more
//   // information on the ADC sequences and steps, reference the datasheet.
//   //
//   ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH3);                            // PE0
//   ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH2);                            // PE1
//   ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END); // PE2

//   // 使能单次采样方式(sample sequence 2)
//   ADCSequenceEnable(ADC0_BASE, 2);

//   // 在采样前，必须清除中断状态标志
//   ADCIntClear(ADC0_BASE, 2);
// }

// //***************************************************************************
// //
// // 函数原型：uint32_t ADC_Sample(void)
// // 函数功能：获取 ADC 采样值，计入 ui32ADCValue。
// // 函数参数：无
// // 函数返回值：无
// //
// //***************************************************************************

// void ADC0_Sample(void)
// {

//   //
//   // This array is used for storing the data read from the ADC FIFO. It
//   // must be as large as the FIFO for the sequencer in use. This example
//   // uses sequence 3 which has a FIFO depth of 1. If another sequence
//   // was used with a deeper FIFO, then the array size must be changed.
//   //

//   // 触发 ADC 采样
//   ADCProcessorTrigger(ADC0_BASE, 2);

//   // 等待采样转换完成
//   while (!ADCIntStatus(ADC0_BASE, 2, false))
//   {
//   }

//   // 清除 ADC 中断标志
//   ADCIntClear(ADC0_BASE, 2);

//   // 读取 ADC 采样值
//   ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);
// }

// //***************************************************************************
// //
// // 函数原型：SysTickInit(void)
// // 函数功能：设置 SysTick 中断
// // 函数参数：无
// // 函数返回值：无
// //
// //***************************************************************************
// void SysTickInit(void)
// {
//   SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期

//   SysTickEnable();    // SysTick 使能
//   SysTickIntEnable(); // SysTick 中断允许
// }

// //***************************************************************************
// //
// // 函数原型：DevicesInit(void)
// // 函数功能：CU 器件初始化，包括系统时钟设置、GPIO 初始化和 SysTick 中断设置
// // 函数参数：无
// // 函数返回值：无
// //
// //***************************************************************************
// void DevicesInit(void)
// {
//   // 使用外部 25MHz 主时钟源，经过 PLL，然后分频为 20MHz
//   ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
//                                      SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
//                                     20000000);

//   GPIOInit(); // GPIO 初始化
//   DACInit();
//   ADCInit();         // ADC 初始化
//   SysTickInit();     // 设置 SysTick 中断
//   IntMasterEnable(); // 总中断允许
// }

// //***************************************************************************
// //
// // 函数原型：void SysTick_Handler(void)
// // 函数功能：SysTick 中断服务程序
// // 函数参数：无
// // 函数返回值：无
// //
// //***************************************************************************
// void SysTick_Handler(void) // 定时周期为 20ms
// {
//   static uint32_t refresh_counter = 0;
//   clock20ms_flag = 1;

//   // 0.1 秒钟软定时器计数
//   if (++clock100ms >= V_T100ms)
//   {
//     clock100ms_flag = 1; // 当 0.1 秒到时，溢出标志置 1
//     clock100ms = 0;
//   }

//   if (++clock300ms >= V_T300ms)
//   {
//     clock300ms_flag = 1; // 当 0.1 秒到时，溢出标志置 1
//     clock300ms = 0;
//   }

//   if (++clock2s >= V_T2s)
//   {
//     clock2s_flag = 1;
//     clock2s = 0;
//   }
//   // 刷新全部数码管和 LED
//   // TM1638_RefreshDIGIandLED(digit, pnt, led);

//   if (++refresh_counter >= 5)
//   {
//     TM1638_RefreshDIGIandLED(digit, pnt, led);
//     refresh_counter = 0; // 重置计数器
//   }
//   key_code = TM1638_Readkeyboard();
//   if (key_code != 0)
//   {
//     if (key_cnt < 3)
//       key_cnt++;
//     else if (key_cnt == 3)
//     {
//       switch (key_code)
//       {
//       case 1:
//         mode_flag = 1;
//         break;
//       default:
//         break;
//       }

//       key_cnt = 4;
//     }
//   }
//   else
//     key_cnt = 0;
// }

// void DAC6571_Byte_Transmission(uint8_t byte_data)
// {
//   uint8_t i, shelter;
//   shelter = 0x80;

//   for (i = 1; i <= 8; i++)
//   {
//     if ((byte_data & shelter) == 0)
//       GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, 0x0); // SDA_L;
//     else
//       GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0); // SDA_H;

//     GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1); // SCL_H;
//     GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, 0x0);        // SCL_L;
//     shelter >>= 1;
//   }
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0);
//   GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_0);     // SDA_IN;
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1); // SCL_H;
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, 0x0);        // SCL_L;
//   GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0);    // SDA_OUT;
// }

// void DAC6571_Fastmode_Operation(void)
// {
//   uint8_t msbyte, lsbyte;

//   // start SCL_H;SDA_H;SDA_L;SCL_L
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1);
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0);
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, 0x0);
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, 0x0);

//   DAC6571_Byte_Transmission(DAC6571_address);
//   msbyte = dac6571_code * 4 / 256;          // 9 8 7 6 .5 4 3 2 1 0
//   lsbyte = dac6571_code * 4 - msbyte * 256; // 5 4 3 2 1 0 00.
//   DAC6571_Byte_Transmission(msbyte);
//   DAC6571_Byte_Transmission(lsbyte);

//   // STOP SDA_L; SCL_H; SDA_H
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, 0x0);
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_2);
//   GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_3);
// }

// void SetCURRENT(void)
// {

//   switch (state)
//   {
//   case 1:

//     if (CUCurrent > 970)
//     {
//       dac6571_code = (uint32_t)((CUCurrent - 717.35) / 0.94);
//     }
//     if (CUCurrent < 970)
//     {
//       dac6571_code = (uint32_t)((CUCurrent - 10.87) / 3.281) + 30;
//     }
//     state = 2;
//     clock300ms_flag = 0;
//     clock300ms = 0;
//     break;
//   case 2:
//     if (clock300ms_flag == 1)
//     {
//       clock300ms_flag = 0;
//       state = 3;
//     }
//     break;
//   case 3:
//     if (AllVoltage > 1000 && AllVoltage > 5400 && CCCurrent < CUCurrent)
//     {
//       state = 4;
//     }

//     if (AllVoltage > 1000 && AllVoltage < 5400 && CCCurrent <= CUCurrent - 5)
//     {
//       dac6571_code += 0.035 * (CUCurrent - CCCurrent);
//       if (dac6571_code > 320)
//         dac6571_code = 320;
//       clock300ms_flag = 0;
//       clock300ms = 0;
//       state = 2;
//     }

//     if (AllVoltage > 1000 && CUCurrent > 9 * (CUCurrent + CCCurrent) / 10)
//     {
//       dac6571_code += 20;
//       if (dac6571_code > 320)
//         dac6571_code = 320;
//       clock300ms_flag = 0;
//       clock300ms = 0;
//       state = 2;
//     }

//     if (AllVoltage > 1000 && CCCurrent > 19 * (CUCurrent + CCCurrent) / 20)
//     {
//       dac6571_code -= 20;
//       if (dac6571_code < 21)
//         dac6571_code = 200;
//       if (dac6571_code > 320)
//         dac6571_code = 320;
//       clock300ms_flag = 0;
//       clock300ms = 0;
//       state = 2;
//     }

//     if (AllVoltage > 1000 && AllVoltage < 5400 && CCCurrent > CUCurrent + 5 && CCCurrent < 19 * CUCurrent)
//     {
//       dac6571_code -= 0.035 * (CCCurrent - CUCurrent);
//       if (dac6571_code < 21)
//         dac6571_code = 200;
//       if (dac6571_code > 320)
//         dac6571_code = 320;
//       clock300ms_flag = 0;
//       clock300ms = 0;
//       state = 2;
//     }

//     if (AllVoltage < 500)
//     {
//       dac6571_code = 320;
//     }

//     break;
//   case 4:
//     if (AllVoltage > 5400 && CCCurrent < CUCurrent)
//     {
//       state = 4;
//     }
//     else
//     {
//       state = 2;
//     }
//     break;

//   default:
//     break;
//   }
// }
// void UpdateAveragePower(uint32_t newPower)
// {
//   powerSamples[powerSampleIndex] = newPower;
//   powerSampleIndex = (powerSampleIndex + 1) % POWER_SAMPLES;
//   sum = 0;
//   for (i = 0; i < POWER_SAMPLES; i++)
//   {
//     sum += powerSamples[i];
//   }
//   averagePower = sum / POWER_SAMPLES;
// }
// void DisplayPower(uint32_t averagePower)
// {
//   digit_aP[0] = (averagePower / 1000) % 10; // 显示平均功率值千位
//   digit_aP[1] = (averagePower / 100) % 10;  // 显示平均功率值百位
//   digit_aP[2] = (averagePower / 10) % 10;   // 显示平均功率值十位
//   digit_aP[3] = averagePower % 10;          // 显示平均功率值个位
// }
// void maxDisplayPower(uint32_t averagePower)
// {
//   if (averagePower > maxPower && (averagePower - maxPower) < 200)
//     maxPower = averagePower;
//   if (averagePower < 150)
//     maxPower = 0;
//   if (maxPower > 1.2 * averagePower)
//     maxPower = 0;
//   digit_mP[0] = (maxPower / 1000) % 10; // 显示最大功率值千位
//   digit_mP[1] = (maxPower / 100) % 10;  // 显示大功率值百位
//   digit_mP[2] = (maxPower / 10) % 10;   // 显示大功率值十位
//   digit_mP[3] = maxPower % 10;          // 显示大功率值个位
// }
// void Displayomh(uint32_t currentomh)
// {
//   digit_omh[0] = (currentomh / 1000) % 10; // 显示最大功率值千位
//   digit_omh[1] = (currentomh / 100) % 10;  // 显示大功率值百位
//   digit_omh[2] = (currentomh / 10) % 10;   // 显示大功率值十位
//   digit_omh[3] = currentomh % 10;          // 显示大功率值个位
// }

// 稳压源J2连PE0,稳压源J1连PE1,稳流源J1连PE2,稳流源J7连PL1,稳流源J6连PL0
// 2.PE0-电压;PE1-稳流源电流;PE2-稳压源电流
// 注意:输入电压值范围必须为[0-3.3V],否则烧坏端口
//*****************************************************************************

//*****************************************************************************
//
// 头文件
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"       // 基址宏定义
#include "inc/hw_types.h"        // 数据类型宏定义
#include "driverlib/debug.h"     //
#include "driverlib/gpio.h"      //
#include "driverlib/pin_map.h"   // TM4C MCU
#include "driverlib/sysctl.h"    //
#include "driverlib/systick.h"   // SysTick Driver
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver
#include "driverlib/adc.h"       //  ADC

#include "tm1638.h" //  TM1638

//*****************************************************************************
//
//
//
//*****************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTick 频率为 50Hz,循环周期 20ms

#define V_T100ms 5 // 0.1s 软件定时器溢出值,5个20ms
#define V_T250ms 25
#define DAC6571_code_max 1023 // 10 位 ADC
#define DAC6571_address 0x98  // 1001 10 A0 0 A0=0
#define WINDOW_SIZE 7
//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);    // GPIO
void ADCInit(void);     // ADC
void SysTickInit(void); //  SysTick
void DevicesInit(void); // MCU
void dac6571_byte_transmission(uint8_t byte_data);
void dac6571_fastmode_operation(void);
void ADC0_Sample(void); //  ADC0采样
void SetCURRENT(void);
uint8_t powerSampleIndex = 0;

uint32_t sum, i = 0;

//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock100ms = 0;
uint8_t clock250ms = 0;
// 软件定时器溢出标志
uint8_t clock20ms_flag = 0;
uint8_t clock100ms_flag = 0;
uint8_t clock250ms_flag = 0;
// 8位数码管显示的数字或字母
// 数码管从左到右为45670123
uint8_t digit[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
uint8_t digit_u[4] = {' ', ' ', ' ', ' '};
uint8_t digit_ic[4] = {' ', ' ', ' ', ' '};
uint8_t digit_iu[4] = {' ', ' ', ' ', ' '};
uint8_t digit_dac[4] = {' ', ' ', ' ', ' '};
uint8_t digit_id[4] = {' ', ' ', ' ', ' '};
uint8_t digit_aP[4] = {' ', ' ', ' ', ' '};
uint8_t digit_mP[4] = {' ', ' ', ' ', ' '};
uint8_t digit_omh[4] = {' ', ' ', ' ', ' '};
// 模式、阶段转换
uint8_t mode = 1; // 1-ic,iu;2-dac,id
uint8_t state = 2;
uint8_t mode_flag = 0;

// 8位小数点1亮0灭
// 数码管从左到右为45670123
uint8_t pnt = 0x00;

// 8个LED状态,0灭1亮
// 从左到右为76543210
// 对应LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0};

// 按键值
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// DAC6571
uint32_t dac6571_code = 0;
uint8_t dac6571_flag = 0;

// 系统时钟频率
uint32_t ui32SysClock;

// ADC采样值[0-4095]
uint32_t ADCvalue0;
uint32_t ADCvalue1;
uint32_t ADCvalue2;
uint32_t ui32ADC0Value[3];

// 电压值(单位 0.01V) [0.00-3.30]
uint32_t AllVoltage;

// 电流值(单位 0.01A)
uint32_t CCCurrent = 0;
uint32_t CUCurrent = 0;
uint32_t CUCurrent1 = 0;

uint8_t window_count_1 = 0;
uint32_t values_1[WINDOW_SIZE] = {0};
uint8_t window_count_2 = 0;
uint32_t values_2[WINDOW_SIZE] = {0};
uint8_t window_count_3 = 0;
uint32_t values_3[WINDOW_SIZE] = {0};

uint32_t sliding_window_filter_1(uint32_t value)
{
  uint8_t i;
  uint32_t sum;

  if (window_count_1 < WINDOW_SIZE)
  {
    values_1[window_count_1++] = value;
  }

  else
  {
    for (i = 1; i < WINDOW_SIZE; i++)
    {
      values_1[i - 1] = values_1[i];
    }
    values_1[WINDOW_SIZE - 1] = value;
  }

  sum = 0;
  for (i = 0; i < window_count_1; i++)
  {
    sum += values_1[i];
  }
  return sum / window_count_1;
}

uint32_t sliding_window_filter_2(uint32_t value)
{
  uint8_t i;
  uint32_t sum;

  if (window_count_2 < WINDOW_SIZE)
  {
    values_2[window_count_2++] = value;
  }

  else
  {
    for (i = 1; i < WINDOW_SIZE; i++)
    {
      values_2[i - 1] = values_2[i];
    }
    values_2[WINDOW_SIZE - 1] = value;
  }
  sum = 0;
  for (i = 0; i < window_count_2; i++)
  {
    sum += values_2[i];
  }
  return sum / window_count_2;
}

uint32_t sliding_window_filter_3(uint32_t value)
{
  uint8_t i;
  uint32_t sum;

  if (window_count_3 < WINDOW_SIZE)
  {
    values_3[window_count_3++] = value;
  }

  else
  {
    for (i = 1; i < WINDOW_SIZE; i++)
    {
      values_3[i - 1] = values_3[i];
    }
    values_3[WINDOW_SIZE - 1] = value;
  }

  sum = 0;
  for (i = 0; i < window_count_3; i++)
  {
    sum += values_3[i];
  }
  return sum / window_count_3;
}
//***************************************************************************
//
// 主程序
//
//***************************************************************************
int main(void)
{

  DevicesInit(); // MCU

  while (clock100ms < 3)
    ;            // 延时>60ms,等待TM1638上电完成
  TM1638_Init(); // 初始化 TM1638

  while (1)
  {

    if (mode_flag == 1)
    {
      mode_flag = 0;
      if (mode < 2)
        mode++;
      else
        mode = 1;
    }

    // 采样
    if (clock20ms_flag == 1) // 检测 20ms 定时是否已到
    {
      clock20ms_flag = 0;

      ADC0_Sample();
      ADCvalue0 = ui32ADC0Value[0];
      ADCvalue1 = ui32ADC0Value[2];
      ADCvalue2 = ui32ADC0Value[1];

      ADCvalue0 = ADCvalue0 * 6600 / 4095 * 10007 / 10000 - 257;
      ADCvalue0 = 1.2549*ADCvalue0 + -967.2928;

      ADCvalue1 = ADCvalue1 * 3300 / 4095;
      ADCvalue1 = ADCvalue1 * 9874 / 10000 + 35;
      ADCvalue1 = 1.0244*ADCvalue1 + -57.1094;
      
      ADCvalue2 = ADCvalue2 * 3300 / 4095;
      ADCvalue2 = ADCvalue2 * 7436 / 10000 + 53;
      ADCvalue2 = 0.1018*ADCvalue2 + 688.9916;
      ADCvalue2 = 9.1127*ADCvalue2 + -6319.6317;


      AllVoltage = sliding_window_filter_1(ADCvalue0); // 电压
      CCCurrent = sliding_window_filter_2(ADCvalue1);       // 稳流源电流
      CUCurrent = sliding_window_filter_3(ADCvalue2);       // 稳压源电流
      CUCurrent1 = CUCurrent;
    }

    // 左侧显示稳流源电流，右侧显示稳压源电流

    digit_u[0] = (AllVoltage / 1000) % 10; // 显示电压值千位
    digit_u[1] = (AllVoltage / 100) % 10;  // 显示电压值百位
    digit_u[2] = (AllVoltage / 10) % 10;   // 显示电压值十位
    digit_u[3] = AllVoltage % 10;          // 显示电压值个位

    digit_ic[0] = (CCCurrent / 1000) % 10; // 显示电流值千位
    digit_ic[1] = (CCCurrent / 100) % 10;  // 显示电流值百位
    digit_ic[2] = (CCCurrent / 10) % 10;   // 显示电流值十位
    digit_ic[3] = CCCurrent % 10;          // 显示电流值个位

    digit_iu[0] = (CUCurrent1 / 1000) % 10; // 显示电流值千位
    digit_iu[1] = (CUCurrent1 / 100) % 10;  // 显示电流值百位
    digit_iu[2] = (CUCurrent1 / 10) % 10;   // 显示电流值十位
    digit_iu[3] = CUCurrent1 % 10;          // 显示电流值个位

    if (clock100ms_flag == 1)
    {
      clock100ms_flag = 0;
      dac6571_flag = 1;
    }

    if (dac6571_flag == 1) // 检测 DAC 电压是否要变
    {
      dac6571_flag = 0;
      dac6571_code = 0.9768*dac6571_code + 2.8655;
      SetCURRENT();
      dac6571_fastmode_operation();

      digit_dac[0] = (dac6571_code / 1000) % 10; // 计算千位0-1023
      digit_dac[1] = (dac6571_code / 100) % 10;  // 计算百位
      digit_dac[2] = (dac6571_code / 10) % 10;   // 计算十位
      digit_dac[3] = dac6571_code % 10;          // 计算个位
    }

    // LED显示内容
    switch (mode)
    {
    case 1:
      digit[0] = digit_iu[0];
      digit[1] = digit_iu[1];
      digit[2] = digit_iu[2];
      digit[3] = digit_iu[3];
      digit[4] = digit_ic[0];
      digit[5] = digit_ic[1];
      digit[6] = digit_ic[2];
      digit[7] = digit_ic[3];
      pnt = 0x11;
      led[7] = 1;
      led[6] = 0;
      led[5] = 0;
      led[4] = 0;
      led[3] = 0;
      led[2] = 0;
      led[1] = 0;
      led[0] = 0;
      break;
    case 2:
      digit[0] = digit_dac[0];
      digit[1] = digit_dac[1];
      digit[2] = digit_dac[2];
      digit[3] = digit_dac[3];
      digit[4] = digit_u[0];
      digit[5] = digit_u[1];
      digit[6] = digit_u[2];
      digit[7] = digit_u[3];
      pnt = 0x10;
      led[7] = 1;
      led[6] = 1;
      led[5] = 0;
      led[4] = 0;
      led[3] = 0;
      led[2] = 0;
      led[1] = 0;
      led[0] = 0;
      break;
    default:
      break;
    }
  }
}

//***************************************************************************
//
// 函数原型:void GPIOInit(void)
// 函数功能:GPIO初始化，使能 PortK,设置 PK4,PK5为输出;使能PortM,设置PM0为输出
// (PK4 连接 TM1638 的 STB,PK5 连接 TM1638 的 DIO,PM0 连接 TM1638的CLK

// 函数参数:无
// 返回值:无
//
//***************************************************************************
void GPIOInit(void)
{
  // 配置TM1638芯片管脚
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); // 使能端口K
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
  {
  }; // 等待端口K准备完毕

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); // 使能端口M
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
  {
  }; // 等待端口M准备完毕

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // 使能端口L
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
  {
  }; // 等待端口L准备完毕

  // 设置 PK4,PK5为输出 PK4-STB PK5-DIO
  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
  // 设置PM0为输出 PM0-CLK
  GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);
  // 设置PL0-SDA，PL1-SCL
  GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

//***************************************************************************
//
// 函数原型:void ADCInit(void)
// 函数功能:ADC0、1初始化
// 选择AIN3/PE0、AIN2/PE1 作为ADC采样输入端口
// 函数参数:无
// 函数返回值:无
//
//***************************************************************************
void ADCInit(void)
{
  // 使能ADC0模块
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

  // PE0、PE1、PE2作为ADC输入
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

  GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

  // 单次采样(sample sequence 2方式,4个采样点)
  ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);

  //
  // Configure step 0 on sequence 3. Sample channel 0 (ADC_CTL_CH0) in
  // single-ended mode (default) and configure the interrupt flag
  // (ADC_CTL_IE) to be set when the sample is done. Tell the ADC logic
  // that this is the last conversion on sequence 3 (ADC_CTL_END). Sequenc

  // 3 has only one programmable step. Sequence 1 and 2 have 4 steps, and
  // sequence 0 has 8 programmable steps. Since we are only doing a single
  // conversion using sequence 3 we will only configure step 0. For more
  // information on the ADC sequences and steps, reference the datasheet.
  //
  ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH3);                            // PE0
  ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH2);                            // PE1
  ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END); // PE2

  // 使能单次采样方式(sample sequence 2)
  ADCSequenceEnable(ADC0_BASE, 2);

  // 采样前清除中断标志
  ADCIntClear(ADC0_BASE, 2);
}

//***************************************************************************
//
// 函数原型:uint32_t ADC_Sample(void)
// 函数功能:获取ADC采样值,计入ui32ADCValue
// 函数参数:无
// 函数返回值:无
//
//***************************************************************************

void ADC0_Sample(void)
{

  //
  // This array is used for storing the data read from the ADC FIFO. It
  // must be as large as the FIFO for the sequencer in use. This example
  // uses sequence 3 which has a FIFO depth of 1. If another sequence
  // was used with a deeper FIFO, then the array size must be changed.
  //

  // 触发ADC采样
  ADCProcessorTrigger(ADC0_BASE, 2);

  // 等待采样转换完成
  while (!ADCIntStatus(ADC0_BASE, 2, false))
  {
  }

  // 清除ADC中断标志
  ADCIntClear(ADC0_BASE, 2);

  // 读取ADC采样值
  ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);
}

//***************************************************************************
//
// 函数原型:SysTickInit(void)
// 函数功能:设置SysTick中断
// 函数参数:无
// 函数返回值:无
//
//***************************************************************************
void SysTickInit(void)
{
  SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // 设置心跳节拍,定时周期

  SysTickEnable();    // SysTick 使能
  SysTickIntEnable(); // SysTick中断允许
}

//***************************************************************************
//
// 函数原型:DevicesInit(void)
// 函数功能:CU 初始化
// 函数参数:无
// 函数返回值:无
//
//***************************************************************************
void DevicesInit(void)
{
  // 使用外部25MHz主时钟源经过PLL,分频为20MHz
  ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                     SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                    20000000);

  GPIOInit();        // GPIO 初始化
  ADCInit();         // ADC 初始化
  SysTickInit();     // 设置SysTick中断
  IntMasterEnable(); // 总中断允许
}

//***************************************************************************
//
// 函数原型:void SysTick_Handler(void)
// 函数功能:SysTick 中断服务
// 函数参数:无
// 返回值:无
//
//***************************************************************************
void SysTick_Handler(void) // 周期20ms
{
  //static uint32_t refresh_counter = 0;
  clock20ms_flag = 1;

  // 0.1 软计时器计数
  if (++clock100ms >= V_T100ms)
  {
    clock100ms_flag = 1; // 当0.1s，溢出标志置1
    clock100ms = 0;
  }

  if (++clock250ms >= V_T250ms)
  {
    clock250ms_flag = 1; // 当0.1s，溢出标志置1
    clock250ms = 0;
    TM1638_RefreshDIGIandLED(digit, pnt, led);
  }
  // 刷新全部数码管和LED
  // TM1638_RefreshDIGIandLED(digit, pnt, led);

  // if (++refresh_counter >= 5)
  // {
  //   TM1638_RefreshDIGIandLED(digit, pnt, led);
  //   refresh_counter = 0; // 重置计数器
  // }
  key_code = TM1638_Readkeyboard();
  if (key_code != 0)
  {
    if (key_cnt < 3)
      key_cnt++;
    else if (key_cnt == 3)
    {
      switch (key_code)
      {
      case 1:
        mode_flag = 1;
        break;
      default:
        break;
      }

      key_cnt = 4;
    }
  }
  else
    key_cnt = 0;
}

void dac6571_byte_transmission(uint8_t byte_data)
{
  uint8_t i, shelter;
  shelter = 0x80;

  for (i = 1; i <= 8; i++)
  {
    if ((byte_data & shelter) == 0)
      GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, 0x0); // SDA_L;
    else
      GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0); // SDA_H;

    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1); // SCL_H;
    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, 0x0);        // SCL_L;
    shelter >>= 1;
  }
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0);
  GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_0);     // SDA_IN;
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1); // SCL_H;
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, 0x0);        // SCL_L;
  GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0);    // SDA_OUT;
}

void dac6571_fastmode_operation(void)
{
  uint8_t msbyte, lsbyte;

  // start SCL_H;SDA_H;SDA_L;SCL_L
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1);
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0);
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, 0x0);
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, 0x0);

  dac6571_byte_transmission(DAC6571_address);
  msbyte = dac6571_code * 4 / 256;          // 9 8 7 6 .5 4 3 2 1 0
  lsbyte = dac6571_code * 4 - msbyte * 256; // 5 4 3 2 1 0 00.
  dac6571_byte_transmission(msbyte);
  dac6571_byte_transmission(lsbyte);

  // STOP SDA_L; SCL_H; SDA_H
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, 0x0);
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_1, GPIO_PIN_1);
  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0, GPIO_PIN_0);
}

void SetCURRENT(void)
{

  switch (state)
  {
  case 1:

    if (CUCurrent > 970)
    {
      dac6571_code = (uint32_t)((CUCurrent - 717.35) / 0.94);
    }
    if (CUCurrent < 970)
    {
      dac6571_code = (uint32_t)((CUCurrent - 10.87) / 3.281) + 30;
    }
    state = 2;
    clock250ms_flag = 0;
    clock250ms = 0;
    break;
  case 2:
    if (clock250ms_flag == 1) // 延时250ms
    {
      clock250ms_flag = 0;
      state = 3;
    }
    break;
  case 3:
    if (AllVoltage > 1000 && AllVoltage > 5400 && CCCurrent < CUCurrent)
    {
      state = 4;
    }

    if (AllVoltage > 1000 && AllVoltage < 5400 && CCCurrent <= CUCurrent - 5)
    {
      dac6571_code += 0.035 * (CUCurrent - CCCurrent);
      if (dac6571_code > 320)
        dac6571_code = 320;
      clock250ms_flag = 0;
      clock250ms = 0;
      state = 2;
    }

    if (AllVoltage > 1000 && CUCurrent > 9 * (CUCurrent + CCCurrent) / 10)
    {
      dac6571_code += 20;
      if (dac6571_code > 320)
        dac6571_code = 320;
      clock250ms_flag = 0;
      clock250ms = 0;
      state = 2;
    }

    if (AllVoltage > 1000 && CCCurrent > 19 * (CUCurrent + CCCurrent) / 20)
    {
      dac6571_code -= 20;
      if (dac6571_code < 21)
        dac6571_code = 200;
      if (dac6571_code > 320)
        dac6571_code = 320;
      clock250ms_flag = 0;
      clock250ms = 0;
      state = 2;
    }

    if (AllVoltage > 1000 && AllVoltage < 5400 && CCCurrent > CUCurrent + 5 && CCCurrent < 19 * CUCurrent)
    {
      dac6571_code -= 0.035 * (CCCurrent - CUCurrent);
      if (dac6571_code < 21)
        dac6571_code = 200;
      if (dac6571_code > 320)
        dac6571_code = 320;
      clock250ms_flag = 0;
      clock250ms = 0;
      state = 2;
    }

    if (AllVoltage < 500)
    {
      dac6571_code = 320;
    }

    break;
  case 4:
    if (AllVoltage > 5400 && CCCurrent < CUCurrent)
    {
      state = 4;
    }
    else
    {
      state = 2;
    }
    break;

  default:
    break;
  }
}
