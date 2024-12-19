//*************************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: adc_demo.c
// Description:
//    1.��ʾ��չʾ�������CH0/PE3��CH1/PE2�˿�ʵ����·ADC����,����Ƶ��25Hz��
//    2.����ĸ��������ʾCH0/PE3��ADC����ֵ[0-4095]��
//    3.�Ҳ��ĸ��������ʾCH1/PE2��ADC����ֵ[0-4095]��
//    4.ע�⣺�����ѹֵ��Χ����Ϊ[0-3.3V]��������ջ��˿ڡ�
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20201125
// Date��2020-11-25
// History��
//
//*************************************************************************************

//*************************************************************************************
//
// ͷ�ļ�
//
//*************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"       // ��ַ�궨��
#include "inc/hw_types.h"        // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"     // ������
#include "driverlib/gpio.h"      // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"   // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"    // ϵͳ���ƶ���
#include "driverlib/systick.h"   // SysTick Driver ԭ��
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver ԭ��
#include "driverlib/adc.h"       // ��ADC�йصĶ���
#include "tm1638.h"              // �����TM1638оƬ�йصĺ���

#include "DAC6571.h" // ???DAC6571???????

//*************************************************************************************
//
// �궨��
//
//*************************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T40ms 1  // 40ms�����ʱ�����ֵ��2��20ms
#define V_T2s 5  // 2s�����ʱ�����ֵ��100��20ms
#define V_T80ms 4 // 80s�����ʱ�����ֵ��4��20ms

//*************************************************************************************
//
// ����ԭ������
//
//*************************************************************************************
void GPIOInit(void);    // GPIO��ʼ��
void ADCInit(void);     // ADC��ʼ��
void SysTickInit(void); // ����SysTick�ж�
void DevicesInit(void); // MCU������ʼ����ע���������������
void ADC_Sample(void);  // ��ȡADC����ֵ
//*************************************************************************************
//
// ��������
//
//*************************************************************************************

// �����ʱ������
uint8_t clock40ms = 0;
uint8_t clock2s = 0;
uint8_t clock80ms = 0;

// �����ʱ�������־
uint8_t clock40ms_flag = 0;
uint8_t clock2s_flag = 0;
uint8_t clock80ms_flag = 0;

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x11;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0};

// ��ǰ����ֵ
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// ϵͳʱ��Ƶ��
uint32_t ui32SysClock;

// ��Ŵ�ADC FIFO��ȡ�Ĳ������� [0-4095]
uint32_t pui32ADC0Value[4];

uint32_t ADCsteadyV_showV = 0;
uint32_t ADCsteadyV_showI = 0;
uint32_t ADCsteadyI_showI = 0;

uint32_t ADCsteadyI_showI_temp = 0;

uint32_t digitLeft = 0;
uint32_t digitRight = 0;

uint8_t mode=0;

// DAC6571
uint32_t DAC6571_code = 300;
uint32_t target_current = 1000;
uint32_t DAC6571_voltage = 250;
uint8_t DAC6571_flag = 0;

uint32_t max_current = 1001;

bool output_flag1 = 1, output_flag2 = 1;

//*************************************************************************************
//
// ������
//
//*************************************************************************************
int main(void)
{
    DevicesInit(); //  MCU������ʼ��

    while (clock2s < 3)
        ;          // ��ʱ>60ms,�ȴ�TM1638�ϵ����
    TM1638_Init(); // ��ʼ��TM1638

    DAC6571_flag = 1;

    while (1)
    {
        if (DAC6571_flag == 1)
        {
            // if (ADCsteadyI_showI < 30)
            //     target_current = max_current;
            // if (target_current > max_current)
            //     target_current = max_current;
            // DAC6571_code = 0.31888594164456235 * target_current + -21.46153846153847;
            
            // //�����⣿������----------------------------------------------
            // DAC6571_Fastmode_Operation(DAC6571_code);
        }
        

        if (clock2s_flag == 1)
        {
            clock2s_flag = 0;

            ADCsteadyV_showV = ADCsteadyV_showV * 5040 / 3100;
            ADCsteadyV_showI = 0.5269 * ADCsteadyV_showI + 74.2041;
            // �����������ʾ�����ѹ����8�л�����ѹԴ����
            if (output_flag1)
                digitLeft = ADCsteadyV_showV;
                //digitLeft = 4321;
            else
                digitLeft = ADCsteadyV_showI; 
            //digitLeft = ADCsteadyV_showV;
            digit[4] = digitLeft / 1000;     // ��ʾCH0/PE3��ADC����ֵǧλ��
            digit[5] = digitLeft / 100 % 10; // ��ʾCH0/PE3��ADC����ֵ��λ��
            digit[6] = digitLeft / 10 % 10;  // ��ʾCH0/PE3��ADC����ֵʮλ��
            digit[7] = digitLeft % 10;       // ��ʾCH0/PE3��ADC����ֵ��λ��


            target_current = ADCsteadyI_showI;

            if (ADCsteadyI_showI < 30)
                target_current = max_current;
            if (target_current > max_current)
                target_current = max_current;
            DAC6571_code = 0.31888594164456235 * target_current + -21.46153846153847;
            
            //�����⣿������----------------------------------------------
            //DAC6571_Fastmode_Operation(DAC6571_code);

            // �����ұ�����ʾĿ�����target_current����9�л�������Դ����
            if (output_flag2)
                digitRight = target_current;
            else
                digitRight = ADCsteadyI_showI; 
                //digitRight = 1234;

            //digitRight = ADCsteadyI_showI;
            digit[0] = digitRight / 1000;     // ��ʾCH1/PE2��ADC����ֵǧλ��
            digit[1] = digitRight / 100 % 10; // ��ʾCH1/PE2��ADC����ֵ��λ��
            digit[2] = digitRight / 10 % 10;  // ��ʾCH1/PE2��ADC����ֵʮλ��
            digit[3] = digitRight % 10;       // ��ʾCH1/PE2��ADC����ֵ��λ��

            ADCsteadyV_showV = 0;
            ADCsteadyV_showI = 0;
            ADCsteadyI_showI = 0;

            

        }

        if (clock40ms_flag == 1) // ���40ms�붨ʱ�Ƿ�
        {
            clock40ms_flag = 0;

            ADC_Sample();

            
        }

        if (clock80ms_flag == 1)
        {
            clock80ms_flag = 0;
            
        }
        

    }
}

//*************************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//          ��PK4����TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
// ������������
// ��������ֵ����
//
//*************************************************************************************
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

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // ʹ�ܶ˿� M
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
    {
    }; // �ȴ��˿� M׼�����

    // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);

    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);
}

//*************************************************************************************
//
// ����ԭ�ͣ�void ADCInit(void)
// �������ܣ�ADC0��ʼ��
//          ѡ��CHO/PE3��CH1/PE2������ΪADC0��������˿ڣ�ѡ�ò������в�����1
// ������������
// ��������ֵ����
//
//*************************************************************************************
void ADCInit(void)
{
    // ʹ��ADC0ģ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // ʹ�ܶ˿�E
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // ʹ��CH0/PE3��CH1/PE2������ΪADC����
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);

    // ѡ�ò������в�����1��������ʼ�ź���ADCProcessorTrigger�������������ȼ�Ϊ0
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    // ���ò������в�����1��0����:ѡ��CH0Ϊ����ͨ��
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0); // CH0/PE3

    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0); // CH1/PE2

    // ���ò������в�����1��1����:
    // ADC_CTL_CH1--ѡ��CH1Ϊ����ͨ���� ADC_CTL_END--�������е����һ��
    // ADC_CTL_IE--��������������ж�
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH1 | ADC_CTL_END | ADC_CTL_IE); // CH2/PE1

    // ʹ�ܲ������в�����1
    ADCSequenceEnable(ADC0_BASE, 1);

    // �ڲ���ǰ����������ж�״̬��־
    ADCIntClear(ADC0_BASE, 1);
}

//*************************************************************************************
//
// ����ԭ�ͣ�void ADC_Sample(void)
// �������ܣ���ȡADC����ֵ
// ������������
// ��������ֵ����
//
//*************************************************************************************
void ADC_Sample(void)
{
    // pui32ADC0Value�������ڴ�ADC FIFO��ȡ������
    extern uint32_t pui32ADC0Value[4];

    // ����ADC0�������з�����1��ʼ����
    ADCProcessorTrigger(ADC0_BASE, 1);

    // �ȴ�ADC0�������з�����1����ת�����
    while (!ADCIntStatus(ADC0_BASE, 1, false))
    {
    }

    // ���ADC0�жϱ�־
    ADCIntClear(ADC0_BASE, 1);

    // ��ȡADC0����ֵ���洢������pui32ADC0Value��
    ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);

    // ui32ADC0Value[0]ΪCH0/PE3��ѹԴ��ѹ�Ĳ���ֵ
    ADCsteadyV_showV = ADCsteadyV_showV + pui32ADC0Value[0] / 5;
    // ui32ADC0Value[1]ΪCH1/PE2��ѹԴ�����Ĳ���ֵ
    ADCsteadyV_showI = ADCsteadyV_showI + pui32ADC0Value[1] / 5;
    // ui32ADC0Value[1]ΪCH1/PE2����Դ�����Ĳ���ֵ
    ADCsteadyI_showI = ADCsteadyI_showI + pui32ADC0Value[2] / 5;
}

//*************************************************************************************
//
// ����ԭ�ͣ�SysTickInit(void)
// �������ܣ�����SysTick�ж�
// ������������
// ��������ֵ����
//
//*************************************************************************************
void SysTickInit(void)
{
    SysTickPeriodSet(ui32SysClock / SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
    SysTickEnable();                                    // SysTickʹ��
    SysTickIntEnable();                                 // SysTick�ж�����
}

//*************************************************************************************
//
// ����ԭ�ͣ�DevicesInit(void)
// �������ܣ�MCU������ʼ��������ϵͳʱ�����á�GPIO��ʼ����SysTick�ж�����
// ������������
// ��������ֵ����
//
//*************************************************************************************
void DevicesInit(void)
{
    // ʹ���ⲿ25MHz��ʱ��Դ������PLL��Ȼ���ƵΪ20MHz
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                      20000000);

    GPIOInit();        // GPIO��ʼ��
    ADCInit();         // ADC��ʼ��
    SysTickInit();     // ����SysTick�ж�
    IntMasterEnable(); // ���ж�����
}

//*************************************************************************************
//
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*************************************************************************************
void SysTick_Handler(void) // ��ʱ����Ϊ20ms
{

    // 40ms������ʱ������
    if (++clock40ms >= V_T40ms)
    {
        clock40ms_flag = 1; // ��40ms��ʱ�������־��1
        clock40ms = 0;
    }

    if (++clock80ms >= V_T80ms)
    {
        clock80ms_flag = 1; // ��40ms��ʱ�������־��1
        clock80ms = 0;

        if (ADCsteadyI_showI_temp != ADCsteadyI_showI)
        {
            // DAC6571_flag = 1;
            // ADCsteadyI_showI_temp = ADCsteadyI_showI;

            // target_current =  ADCsteadyI_showI;

            // if (ADCsteadyI_showI < 30)
            //     target_current = max_current;
            // if (target_current > max_current)
            //     target_current = max_current;
            // DAC6571_code = 0.31888594164456235 * target_current + -21.46153846153847;
            
            // //�����⣿������----------------------------------------------
            // //DAC6571_Fastmode_Operation(DAC6571_code);

            
        }
        
    }

    // 0.1������ʱ������
    if (++clock2s >= V_T2s)
    {
        clock2s_flag = 1; // ��0.1�뵽ʱ�������־��1
        clock2s = 0;

        //TM1638_RefreshDIGIandLED(digit, pnt, led);
    }

    // ˢ��ȫ������ܺ�LED
    TM1638_RefreshDIGIandLED(digit, pnt, led);

    // ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
    key_code = TM1638_Readkeyboard();

        if (key_code != 0)
    {
        if (key_cnt < 4)
            key_cnt++; // ����������4*20ms
        else if (key_cnt == 4)
        {

            switch (key_code)
            {
            // case 1: // ��100
            //     if (target_current < max_current - 100)
            //     {
            //         target_current += 100;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 4: // ��100
            //     if (target_current > 100)
            //     {
            //         target_current -= 100;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 2: // ��10
            //     if (target_current < max_current - 10)
            //     {
            //         target_current += 10;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 5: // ��10
            //     if (target_current >= 10)
            //     {
            //         target_current -= 10;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 3: // ��1
            //     if (target_current < max_current - 1)
            //     {
            //         target_current += 1;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            // case 6: // ��1
            //     if (target_current >= 1)
            //     {
            //         target_current -= 1;
            //         DAC6571_flag = 1;
            //     }
            //     break;
            case 8: // ת�����ģʽ����ѹ/����
                output_flag1 = !output_flag1;
                break;
            case 9: // ת�����ģʽ����ѹ/����
                output_flag2 = !output_flag2;
                break;
            default:
                break;
            }

            key_cnt = 5; // ����һֱ���ţ�ֻ�ı�һ��
        }
    }
    else
        key_cnt = 0;

}
