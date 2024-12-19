//*******************************************************************************************************
//
// Copyright: 2021-2022, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: Frequency_Measure.c
// Description: ��ʾ��չʾ���ʹ�ò�Ƶ�ʷ�ʵ�ַ���Ƶ�ʵĲ��������ⷽ���ź����ӵ�T0CCP0/PL4���ţ�
// 1.ÿ�ΰ���A2000TM4C�װ尴��1(SW1)��,����ʼ���������źŵ�Ƶ�ʣ�������ɺ��������ʾ�������,��λΪHz;
// 2.�������Ƶ�ʲ��ڡ�10��9999����Χ���ݣ�����ʾErr��
// 3.ע��������ⷽ���źŸߵ�ƽ���˸���3.3V���͵�ƽ���˵���0V������һ��Ҫ��װ干��
//             �ó��������ڼ��<=1HzƵ�ʵķ�����Ƶ�ʼ�����������1Hz
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20210513
// Date��2021-05-13
// History��
//
//*******************************************************************************************************

//*******************************************************************************************************
//
// ͷ�ļ�
//
//*******************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"       // ��ַ�궨��
#include "inc/hw_types.h"        // �������ͺ궨�壬�Ĵ������ʺ���
#include "inc/hw_timer.h"        // �붨ʱ���йصĺ궨��
#include "inc/hw_ints.h"         // ���ж��йصĺ궨��
#include "driverlib/debug.h"     // ������
#include "driverlib/gpio.h"      // ͨ��IO�ں궨��ͺ���ԭ��
#include "driverlib/pin_map.h"   // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"    // ϵͳ���ƶ���
#include "driverlib/systick.h"   // SysTick Driver ����ԭ��
#include "driverlib/interrupt.h" // NVIC�жϿ�����������ԭ��
#include "driverlib/timer.h"     // ��Timer�йصĺ���ԭ��
#include "driverlib/pwm.h"       // ��Timer�йصĺ���ԭ��
#include "driverlib/uart.h"
#include "driverlib/fpu.h"

#include "tm1638.h" // �����TM1638оƬ�йصĺ궨��ͺ���ԭ��

//*******************************************************************************************************
//
// �궨��
//
//*******************************************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define FREQUENCY_MIN 10   // ��ЧƵ����Сֵ������ʵ����Ҫ����
#define FREQUENCY_MAX 9999 // ��ЧƵ�����ֵ������ʵ����Ҫ����
//*******************************************************************************************************
//
// ����ԭ������
//
//*******************************************************************************************************
void Timer0Init(void);              // Timer0��ʼ��
void Timer2Init(void);              // Timer2��ʼ��
void PWMInit(uint32_t ui32Freq_Hz); // ����Ƶ��Ϊui32Freq_Hz�ķ���

extern uint32_t g_ui32SysClock;

// extern uint8_t mode;

//*******************************************************************************************************
//
// ��������
//
//*******************************************************************************************************

// 1s����������־
volatile uint8_t g_ui8INTStatus = 0;

// ������һ��TIMER0���ؼ���ֵ
volatile uint32_t g_ui32TPreCount = 0;

// ���汾��TIMER0���ؼ���ֵ
volatile uint32_t g_ui32TCurCount = 0;

// ��¼�����ķ���Ƶ��
uint32_t ui32Freq;

//*******************************************************************************************************
//
// ������
//
//*******************************************************************************************************
void Get_Frequency()
{

    // PWMInit(1234);   //  ����3998Hz���������û�û���źŷ�����ʱ���ڲ��������ź�,���������޸�Ƶ��

    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer1A ��ʱ�ж�ʹ��
    IntEnable(INT_TIMER1A);                          // ���� TIMER1A �ж�Դ
    TimerEnable(TIMER1_BASE, TIMER_A);               // TIMER1 ��ʼ��ʱ

    TimerEnable(TIMER0_BASE, TIMER_A); // TIMER0 ��ʼ����

    if (g_ui8INTStatus == 1) // 1s��ʱ��������ʼ����Ƶ��
    {
        g_ui8INTStatus = 0;

        ui32Freq = g_ui32TCurCount >= g_ui32TPreCount ? (g_ui32TCurCount - g_ui32TPreCount) : (g_ui32TCurCount - g_ui32TPreCount + 0xFFFF);

        ui32Freq = ui32Freq * 21;
    }

    // �������ʾ��ֵ

    if ((ui32Freq >= FREQUENCY_MIN) && (ui32Freq <= FREQUENCY_MAX))
    {
        digit[4] = ui32Freq / 1000 % 10; // ����ǧλ��
        digit[5] = ui32Freq / 100 % 10;  // �����λ��
        digit[6] = ui32Freq / 10 % 10;   // ����ʮλ��
        digit[7] = ui32Freq % 10;        // �����λ��
    }
    else // ����Ƶ�ʳ���ָ����Χ
    {
        digit[4] = 'E';
        digit[5] = 'R';
        digit[6] = 'R';
        digit[7] = ' ';
    }
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void Timer0Init(void)
// �������ܣ�����Timer0Ϊ������أ������أ�����ģʽ��T0CCP0(PL4)Ϊ��������
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void Timer0Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // ʹ��TIMER0

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // ʹ��GPIOL

    GPIOPinConfigure(GPIO_PL4_T0CCP0);                                                       // �������Ÿ���
    GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4);                                           // ����ӳ��
    GPIOPadConfigSet(GPIO_PORTL_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // ������������

    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP); // �볤��ʱ����������
    // TimerPrescaleSet(TIMER0_BASE, TIMER_A, 255);   // Ԥ��Ƶ256
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE); // ��ʼ������Ϊ��׽������
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void Timer1Init(void)
// �������ܣ�����Timer1Ϊһ���Զ�ʱ������ʱ����Ϊ1s
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void Timer1Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // TIMER1 ʹ��

    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT); // ����Ϊ 32 λ 1�� ��ʱ��

    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32SysClock / 21); // TIMER1Aװ�ؼ���ֵ50ms
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void TIMER1A_Handler(void)
// �������ܣ�Timer1A�жϷ�����򣬼�¼���񷽲�������ʱ��ʱ����TIMER0���ļ���ֵ
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void TIMER1A_Handler(void)
{

    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);        // ����жϱ�־
    g_ui32TPreCount = g_ui32TCurCount;                     // ������һ��TIMER0���ؼ���ֵ
    g_ui32TCurCount = TimerValueGet(TIMER0_BASE, TIMER_A); // ��ȡTIMER0���ؼ���ֵ
    TimerDisable(TIMER0_BASE, TIMER_A);                    // ֹͣTIMER0���ؼ���
    g_ui8INTStatus = 1;                                    // 1s�������
}

//*******************************************************************************************************
//
// ����ԭ�ͣ�void PWMInit(uint32_t ui32Freq_Hz)
// �������ܣ�����Ƶ��Ϊui32Freq_Hz�ķ���(ռ�ձ�Ϊ50%��PWM)���������ΪM0PWM4(PG0)
//          �ú�����Ϊ�˷����û�û���źŷ�����ʱ���������źŶ���д�ġ�
// ����������ui32Freq_Hz ��Ҫ�����ķ�����Ƶ��
// ��������ֵ����
//
//*******************************************************************************************************
void PWMInit(uint32_t ui32Freq_Hz)
{
    /*
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);             // ����Ƶ
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // PWM0ʹ��
    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // ʹ��(����)PWM0_3�����
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);             //ʹ��PWM0ģ���1�ŷ�����(��Ϊ3��PWM��1�ŷ�����������)
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ui32SysClock / ui32Freq_Hz); // ����Freq_Hz����PWM����


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // ʹ��GPIOF
    GPIOPinConfigure(GPIO_PF3_M0PWM3);              // �������Ÿ���
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);    // ����ӳ��

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);   //����PWM������
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/ 2)); //����ռ�ձ�Ϊ50%
*/

    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); // PWM0ʹ��

    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); // ʹ��(����)PWM0_3�����
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);             // ʹ��PWM0ģ���1�ŷ�����(��Ϊ3��PWM��1�ŷ�����������)
    // PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, ui32SysClock / ui32Freq_Hz); // ����Freq_Hz����PWM����

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // ʹ��GPIOF
    GPIOPinConfigure(GPIO_PF3_M0PWM3);           // �������Ÿ���
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3); // ����ӳ��

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC); // ����PWM������
    // PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //����ռ�ձ�Ϊ50%
}

//
void PWMStart(uint32_t ui32Freq_Hz)
{
    /*
        PWMGenDisable(PWM0_BASE, PWM_GEN_1);     //ʹ��PWM0ģ���1�ŷ�����(��Ϊ3��PWM��2�ŷ�����������)

        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, g_ui32SysClock / ui32Freq_Hz); // ����Freq_Hz����PWM����
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)/ 2)); //����ռ�ձ�Ϊ50%

        PWMGenEnable(PWM0_BASE, PWM_GEN_1);     //ʹ��PWM0ģ���1�ŷ�����(��Ϊ3��PWM��1�ŷ�����������)
    */
    //*******************************************************************************************************
    PWMGenDisable(PWM0_BASE, PWM_GEN_1); // ʹ��PWM0ģ���1�ŷ�����(��Ϊ3��PWM��2�ŷ�����������)

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ui32SysClock / ui32Freq_Hz);                   // ����Freq_Hz����PWM����
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3, (PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 2)); // ����ռ�ձ�Ϊ50%

    PWMGenEnable(PWM0_BASE, PWM_GEN_1); // ʹ��PWM0ģ���1�ŷ�����(��Ϊ3��PWM��1�ŷ�����������)
}
