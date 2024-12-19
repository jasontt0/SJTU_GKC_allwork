//*****************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: exp5_7.c
// Description: ����ʵ��ο�����Ҫ����ɽ��ն˹���
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20201228
// Date��2021-5-30
// History��
//
//*****************************************************************************

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h" // ��ַ�궨��
#include "inc/hw_types.h"  // �������ͺ궨�壬�Ĵ������ʺ���

#include "driverlib/debug.h"     // ������
#include "driverlib/gpio.h"      // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"   // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"    // ϵͳ���ƶ���
#include "driverlib/systick.h"   // SysTick Driver ԭ��
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/uart.h"      // ��UART�йصĺ궨��ͺ���ԭ��

#include "JLX12864.c"                //Һ����ʾ������
#include "ADC_Demo_Voltage.c"        //����AIN2/PE1�˿�ʵ�ֵ������뵥��ADC����
#include "Audio_Frequency_Measure.c" //����PL4/PL5�˿�ʵ��Ƶ�ʽ���
#include "Set_Frequency.c"

#include "tm1638.h" // �����TM1638оƬ�йصĺ���

#include "driverlib/adc.h" // ��ADC�йصĶ���

#include "ADC.h"
//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T20ms 1
#define V_T40ms 2  // 100ms�����ʱ�����ֵ��2��20ms
#define V_T100ms 5 // 100ms�����ʱ�����ֵ��1��100ms
#define V_T300ms 15
#define V_T1500ms 10 // 1s�����ʱ�����ֵ��20��100ms
#define V_T2s 100    // 2s�����ʱ�����ֵ��100��20ms
// #define V_T300ms 15  // 300ms�����ʱ�����ֵ��3��100ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);                                  // GPIO��ʼ��
void SysTickInit(void);                               // ����SysTick�ж�
void UARTInit(void);                                  // UART��ʼ��
void DevicesInit(void);                               // MCU������ʼ����ע���������������
void ui_state_proc(unsigned int ui_state);            // ���ݵ�ǰģʽ����ģʽ������
void ACT0(void);                                      // ģʽ0
void ACT1(void);                                      // ģʽ1
void ACT2(void);                                      // ģʽ2
void in_de(unsigned int w, unsigned char *actstring); // �Ӽ���������
void ENTER_detect(void);                              // �����������¼�⺯��
void LEFT_detect(void);
void RIGHT_detect(void);
void INCREASE_detect(void);
void DECREASE_detect(void);
void CODE_detect(void);                                                                                                        // ���ⰴ�����¼�⺯��
void initial_device(void);                                                                                                     // ģʽ0��ʼ��
void char_compare(unsigned char *actstring1, unsigned char *actstring2, unsigned char *actstring3, unsigned char *actstring4); // Ƶ�ʺϷ����жϺ���
void Get_Volt(void);                                                                                                           // ��¼��ѹֵ
void Get_Temperature(void);                                                                                                    // ����FM���յ�Ƶ�ʵõ�������������¶���Ϣ

//*****************************************************************************
//
// ��������
//
//*****************************************************************************

// �����ʱ������
uint8_t clock20ms = 0;
uint8_t clock40ms = 0;
uint8_t clock100ms = 0;
uint8_t clock300ms = 0;
uint32_t clock1500ms = 0;
uint32_t clock2s = 0;

// �����ʱ�������־
uint8_t clock20ms_flag = 0;
uint8_t clock40ms_flag = 0;
uint8_t clock100ms_flag = 0;
uint8_t clock300ms_flag = 0;
uint32_t clock1500ms_flag = 0;
uint32_t clock2s_flag = 0;

// ��¼״̬����ǰ״̬
uint8_t lcd_act = 0;

// �����ü�����
uint32_t test_counter = 0;

// ϵͳʱ��Ƶ��
uint32_t ui32SysClock;

// ϵͳʱ��Ƶ��
uint32_t g_ui32SysClock;

// ʶ�𰴼����µ��м����
uint8_t key_RIGHT_prestate = 1;
uint8_t key_LEFT_prestate = 1;
uint8_t key_INCREASE_prestate = 1;
uint8_t key_DECREASE_prestate = 1;
uint8_t key_ENTER_prestate = 1;
uint8_t key_CODE_prestate = 1;
uint8_t key_BACK_prestate = 1;

// �������±�־
uint8_t key_RIGHT_flag = 0;
uint8_t key_LEFT_flag = 0;
uint8_t key_INCREASE_flag = 0;
uint8_t key_DECREASE_flag = 0;
uint8_t key_ENTER_flag = 0;
uint8_t key_CODE_flag = 0;
uint8_t key_BACK_flag = 0;

// ģʽѡ��
unsigned int ui_state = 0;

// ��ѹ���¶ȣ�Ƶ�ʵ��ַ�����
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

// ���λ�ñ���
uint8_t position1 = 1;

// �ж��ַ���С����
uint8_t compare = 0;

// 5s��ʱ��������
uint8_t Cal_Enable = 0;

// ��ǰ����ֵ
uint8_t key_code = 0;

// ��ǰ�¶�ֵ
uint32_t temperature = 0;

// Ƶ���趨
uint8_t freq[12] = {"AT+FREQ=876\0"};

uint32_t receive_frequ[200] = {0};
uint8_t k_freq = 0;

uint8_t mode = 0;

//*****************************************************************************
//
// ������
//
//*****************************************************************************
int main(void)
{
    DevicesInit(); //  MCU������ʼ��

    while (clock100ms < 3)
        ;          // ��ʱ>60ms,�ȴ�TM1638�ϵ����
    TM1638_Init(); // ��ʼ��TM1638

    LCD_PORT_Init();
    initial_lcd();
    clear_screen(); // clear all dots

    while (1)
    {

        switch (mode)
        {
        case 1:
        {
            PWMGenDisable(PWM0_BASE, PWM_GEN_1); // M0PWM3(PF3)ֹͣ����PWM�ź�
            while (mode == 1)
            {
                if (clock40ms_flag == 1) // ���40ms�붨ʱ�Ƿ�
                {
                    clock40ms_flag = 0;
                }
                if (clock1500ms_flag == 1) // ���1.5s�붨ʱ�Ƿ�
                {
                    clock1500ms_flag = 0;
                    Get_Temperature(); // Ƶ�ʲ���������ʾ��������ϣ���¼�¶�ֵ
                }
                ui_state_proc(ui_state);
                break;
            }
        }
        case 3:
        {
            PWMGenDisable(PWM0_BASE, PWM_GEN_1); // M0PWM3(PF3)ֹͣ����PWM�ź�
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
    // ��������
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6); // ʹ��UART6ģ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); // ʹ�ܶ˿� P
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP))
        ; // �ȴ��˿� P׼�����

    GPIOPinConfigure(GPIO_PP0_U6RX); // ����PP0ΪUART6 RX����
    GPIOPinConfigure(GPIO_PP1_U6TX); // ����PP1ΪUART6 TX����

    // ���ö˿� P�ĵ�0,1λ��PP0,PP1��ΪUART����
    GPIOPinTypeUART(GPIO_PORTP_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // �����ʼ�֡��ʽ����
    UARTConfigSetExpClk(UART6_BASE,
                        ui32SysClock,
                        9600,                    // �����ʣ�9600
                        (UART_CONFIG_WLEN_8 |    // ����λ��8
                         UART_CONFIG_STOP_ONE |  // ֹͣλ��1
                         UART_CONFIG_PAR_NONE)); // У��λ����

}

//*****************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//          ��PK4����TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void GPIOInit(void)
{
    // ����TM1638оƬ�ܽ�
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK); // ʹ�ܶ˿� K
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
    {
    }; // �ȴ��˿� K׼�����

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM); // ʹ�ܶ˿� M
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
    {
    }; // �ȴ��˿� M׼�����

    // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);

    // F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // ʹ�ܶ˿� F
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }; // �ȴ��˿� F׼�����

    // ���ö˿� F�ĵ�3λ��PF3��Ϊ���������
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_3); // 2��3��In��out

    /*
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // ʹ�ܶ˿� L
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
    {
    }; // �ȴ��˿� L׼�����

    // ���ö˿� L�ĵ�3λ��PF3��Ϊ��������
    //GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_4);
    */
}

//*****************************************************************************
//
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTickInit(void)
{
    SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
    // SysTickPeriodSet(g_ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
    SysTickEnable();    // SysTickʹ��
    SysTickIntEnable(); // SysTick�ж�����
}

//*****************************************************************************
//
// ����ԭ�ͣ�void DevicesInit(void)
// �������ܣ�CU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//*****************************************************************************
void DevicesInit(void)
{
    // ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                      20000000);



    GPIOInit();        // GPIO��ʼ��
    ADCInit();         // ADC��ʼ��
    SysTickInit();     // ����SysTick�ж�
    UARTInit();        // ���ڳ�ʼ��
    IntMasterEnable(); // ���ж�����
    Timer0Init();      // Timer0��ʼ��

    PWMInit(1234);

    Timer1Init();

    FPULazyStackingEnable();
    FPUEnable(); // ʹ��FPU

    IntPrioritySet(INT_TIMER1A, 0x00);   // ����INT_TIMER1A������ȼ�
    IntPrioritySet(INT_TIMER0A, 0x01);   // ����INT_TIMER0A������ȼ�
    IntPrioritySet(FAULT_SYSTICK, 0xe0); // ����SYSTICK���ȼ�����INT_TIMER0A�����ȼ�
                                         // IntPrioritySet(INT_GPIOL,0xc0);
}

//*****************************************************************************
//
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void) // ��ʱ����Ϊ20ms
{
    if (++clock20ms >= V_T20ms)
    {
        clock20ms_flag = 1; // ��40ms��ʱ�������־��1
        clock20ms = 0;
    }

    // 40ms��ʱ������
    if (++clock40ms >= V_T40ms)
    {
        clock40ms_flag = 1; // ��40ms��ʱ�������־��1
        clock40ms = 0;
    }

    // 0.1������ʱ������
    if (++clock100ms >= V_T100ms)
    {
        clock100ms_flag = 1; // ��0.1�뵽ʱ�������־��1
        clock100ms = 0;
    }
    //
    if (++clock300ms >= V_T300ms)
    {
        clock300ms_flag = 1; // ��0.1�뵽ʱ�������־��1
        clock300ms = 0;
    }

    // 1.5������ʱ������
    if (++clock1500ms >= V_T1500ms)
    {
        clock1500ms_flag = 1; // ��1.5�뵽ʱ�������־��1
        clock1500ms = 0;
    }

    // 2������ʱ������
    if (Cal_Enable == 1) // 2���ʱ����
    {
        if (++clock2s >= V_T2s)
        {
            clock2s_flag = 1; // ��2�뵽ʱ�������־��1
            Cal_Enable = 0;   // ��2�뵽ʱ�����������0
            clock2s = 0;
        }
    }

    // ˢ��ȫ������ܺ�LEDָʾ��
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
    // ������ʾ��һλ�������
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
// ����ԭ�ͣ�void XXXXX_detect(void)
// �������ܣ��������¼�⺯��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ENTER_detect(void) // ȷ����
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
void LEFT_detect(void) // �����ƶ���
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
void RIGHT_detect(void) // �����ƶ���
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
void INCREASE_detect(void) //+�����
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
void DECREASE_detect(void) //-�����
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
void CODE_detect(void) // ��������
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
// ����ԭ�ͣ�void ui_state_proc(unsigned int ui_state)
// �������ܣ���������ת�ƺ���
// ������������ǰ�������ui_state
// ��������ֵ����
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
// ����ԭ�ͣ�void in_de(unsigned int w, unsigned char* actstring)
// �������ܣ��Ӽ��������㺯��
// ����������w����/���ж�  actstring�������ַ�
// ��������ֵ����
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
// ����ԭ�ͣ�void char_compare(unsigned char* actstring1, unsigned char* actstring2, unsigned char* actstring3, unsigned char* actstring4)
// �������ܣ�����Ƶ�ʺϷ����жϺ�����Ƶ����[88.0MHz,108.0MHz]֮��
// ��������������Ƶ���ַ�
// ��������ֵ����
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
// ����ԭ�ͣ�void ACT0(void)
// �������ܣ�ACT0���棬���յ�ѹ���¶�ֵ
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ACT0(void)
{
    display_GB2312_string(3, 1, "�¶ȣ�", 0);
    display_GB2312_string(3, 81, temperature1, 0);
    display_GB2312_string(3, 89, temperature2, 0);
    display_GB2312_string(3, 97, ".", 0);
    display_GB2312_string(3, 105, temperature3, 0);
    display_GB2312_string(3, 113, "��", 0);

    // ģʽACT0û����/��/+/-������

    if (key_ENTER_flag == 1 || key_LEFT_flag == 1 || key_RIGHT_flag == 1 || key_INCREASE_flag == 1 || key_DECREASE_flag == 1 || key_CODE_flag == 1)
    {
        key_ENTER_flag = 0; // ���������ת��ACT1
        key_LEFT_flag = 0;
        key_RIGHT_flag = 0;
        key_INCREASE_flag = 0;
        key_DECREASE_flag = 0;
        key_CODE_flag = 0;

        clear_screen();
        display_GB2312_string(1, 1, "��Ƶ��", 0); // Ĭ��ΪƵ�ʵİ�λ����
        display_GB2312_string(1, 41, freq1, 1);
        display_GB2312_string(1, 49, freq2, 0);
        display_GB2312_string(1, 57, freq3, 0);
        display_GB2312_string(1, 65, ".", 0);
        display_GB2312_string(1, 73, freq4, 0);
        display_GB2312_string(1, 81, "MHz", 0);
        display_GB2312_string(7, 97, "ȷ��", 0);
        position1 = 1;
        ui_state = 1;
    }
}

//*****************************************************************************
//
// ����ԭ�ͣ�void ACT1(void)
// �������ܣ�ACT1���棬�趨���ն�Ƶ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ACT1(void)
{
    if (key_RIGHT_flag) // �������
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
            display_GB2312_string(7, 97, "ȷ��", 1);
            position1 = 5;
            break;
        case 5:
            display_GB2312_string(7, 97, "ȷ��", 0);
            display_GB2312_string(1, 41, freq1, 1);
            position1 = 1;
            break;
        }
    }

    else if (key_LEFT_flag) // �������
    {
        key_LEFT_flag = 0;
        switch (position1)
        {
        case 1:
            display_GB2312_string(1, 41, freq1, 0);
            display_GB2312_string(7, 97, "ȷ��", 1);
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
            display_GB2312_string(7, 97, "ȷ��", 0);
            display_GB2312_string(1, 73, freq4, 1);
            position1 = 4;
            break;
        }
    }

    else if (key_INCREASE_flag) // ���Ƶ��
    {
        key_INCREASE_flag = 0;
        if (position1 == 1) // ��λ
        {
            in_de(1, freq1);
            display_GB2312_string(1, 41, freq1, 1);
        }
        else if (position1 == 2) // ʮλ
        {
            in_de(1, freq2);
            display_GB2312_string(1, 49, freq2, 1);
        }
        else if (position1 == 3) // ��λ
        {
            in_de(1, freq3);
            display_GB2312_string(1, 57, freq3, 1);
        }
        else if (position1 == 4) // ʮ��λ
        {
            in_de(1, freq4);
            display_GB2312_string(1, 73, freq4, 1);
        }
    }

    else if (key_DECREASE_flag) // ����Ƶ��
    {
        key_DECREASE_flag = 0;
        if (position1 == 1) // ��λ
        {
            in_de(2, freq1);
            display_GB2312_string(1, 41, freq1, 1);
        }
        else if (position1 == 2) // ʮλ
        {
            in_de(2, freq2);
            display_GB2312_string(1, 49, freq2, 1);
        }
        else if (position1 == 3) // ��λ
        {
            in_de(2, freq3);
            display_GB2312_string(1, 57, freq3, 1);
        }
        else if (position1 == 4) // ʮ��λ
        {
            in_de(2, freq4);
            display_GB2312_string(1, 73, freq4, 1);
        }
    }

    else if (key_ENTER_flag) // ȷ����
    {
        key_ENTER_flag = 0;
        if (position1 == 5)
        {
            char_compare(freq1, freq2, freq3, freq4); // �ж�Ƶ���Ƿ��ڹ涨��Χ��
            if (compare == 1)                         // Ƶ��ֵ�Ϸ���תACT0�����趨Ƶ��
            {
                if (*freq1 == '0') // Ƶ��С��100Hz
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
                else if (*freq1 == '1') // Ƶ�ʴ���100Hz
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
 
                display_GB2312_string(3, 1, "�¶ȣ�", 0);
                display_GB2312_string(3, 81, temperature1, 0);
                display_GB2312_string(3, 89, temperature2, 0);
                display_GB2312_string(3, 97, ".", 0);
                display_GB2312_string(3, 105, temperature3, 0);
                display_GB2312_string(3, 113, "��", 0);
                ui_state = 0;
            }
            else if (compare == 0) // Ƶ��ֵ���Ϸ���תACT2
            {
                Cal_Enable = 1; // ����2s��ʱ
                clear_screen();
                display_GB2312_string(1, 1, "Ƶ�ʳ���Χ��", 0);
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
// ����ԭ�ͣ�void ACT2(void)
// �������ܣ�ACT2���棬Ƶ�ʲ��Ϸ��ж���ʾ
// ������������
// ��������ֵ����
//
//*****************************************************************************
void ACT2(void)
{
    if (clock2s_flag == 1) // 2s����תACT1
    {
        clock2s_flag = 0;
        clear_screen();
        display_GB2312_string(1, 1, "��Ƶ��", 0); // Ĭ��ΪƵ�ʵİ�λ����
        display_GB2312_string(1, 41, freq1, 1);
        display_GB2312_string(1, 49, freq2, 0);
        display_GB2312_string(1, 57, freq3, 0);
        display_GB2312_string(1, 65, ".", 0);
        display_GB2312_string(1, 73, freq4, 0);
        display_GB2312_string(1, 81, "MHz", 0);
        display_GB2312_string(7, 97, "ȷ��", 0);
        position1 = 1;
        ui_state = 1;
    }
}

//*****************************************************************************
//
// ����ԭ�ͣ�void Get_volt(void)
// �������ܣ�����ADC_Demo.c�ļ�������õ�ѹֵд�����volt��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void Get_Volt(void) // ��¼��ѹֵ
{
    uint32_t temp;
    ADC_Voltage(); // ��ѹ����������ʾ���������
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
// ����ԭ�ͣ�void Get_Temperature(void)
// �������ܣ�����FM���յ�Ƶ�ʵõ�������������¶���Ϣ
// ������������
// ��������ֵ����
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
