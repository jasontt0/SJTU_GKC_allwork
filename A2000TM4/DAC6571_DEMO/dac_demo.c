//*****************************************************************************
//
// Copyright: 2019-2021, �Ϻ���ͨ��ѧ����ʵ����Ƽ�����III-A��ѧ��
// File name: dac_demo.c
// Description: 
//    1.����������ڳ������DAC6571оƬ�����Ƿ�������PL0������DAC6571֮SDA��PL1����SCL��
//    2.������λ��DAC������Ϊ��ʮ���ƣ�1023���װ����ұ�4λ�������ʾ����ֵ��
//    3.��1�ź�4�ż��ֱ���ƣ�ʮ���ƣ�DAC�������100�ͼ�100��
//    4.��2�ź�5�ż��ֱ���ƣ�ʮ���ƣ�DAC�������10�ͼ�10��
//    5.��3�ź�6�ż��ֱ���ƣ�ʮ���ƣ�DAC�������1�ͼ�1��
//    6.������̥�ڿγ̳�ʼDEMO�������Բ��ֱ��������Ĵ��빦�ܻ�ۼ���
// Author:	�Ϻ���ͨ��ѧ����ʵ����Ƽ�����III-A��ѧ�飨�ϡ�Ԭ��
// Version: 1.1.0.20210930 
// Date��2021-9-30
// History��2021-9-31�޸�����ע�ͣ�Ԭ��
//
//*****************************************************************************

//*****************************************************************************
//
// ͷ�ļ�
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"     // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��

#include "tm1638.h"               // �����TM1638оƬ�йصĺ���
#include "DAC6571.h"              // �����DAC6571оƬ�йصĺ���

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T100ms	5              // 0.1s�����ʱ�����ֵ��5��20ms
#define V_T500ms	25             // 0.5s�����ʱ�����ֵ��25��20ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж� 
void DevicesInit(void);     // MCU������ʼ����ע���������������
//*****************************************************************************
//
// ��������
//
//*****************************************************************************

// �����ʱ������
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;


// �����ʱ�������־
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;

// �����ü�����
uint32_t test_counter = 0;

// 8λ�������ʾ�����ֻ���ĸ����
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8]={' ',' ',' ',' ','_',' ','_',' '};

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x0;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// ��ǰ����ֵ
uint8_t key_code = 0;
uint8_t key_cnt = 0;

// DAC6571
uint32_t DAC6571_code = 512;
uint32_t DAC6571_voltage = 250;
uint8_t  DAC6571_flag = 0;


// ϵͳʱ��Ƶ�� 
uint32_t ui32SysClock;

//*****************************************************************************
//
// ������
//
//*****************************************************************************
 int main(void)
{
	uint8_t temp,i;

	DevicesInit();            //  MCU������ʼ��
	
	while (clock100ms < 3);   // ��ʱ>60ms,�ȴ�TM1638�ϵ����
	TM1638_Init();	          // ��ʼ��TM1638
	
    DAC6571_flag = 1;
    
	while (1)
	{				
        if (DAC6571_flag == 1)   // ���DAC��ѹ�Ƿ�Ҫ��
		{
			DAC6571_flag = 0;

			digit[0] = DAC6571_code / 1000 ; 	  // ����ǧλ��
			digit[1] = DAC6571_code / 100 % 10;   // �����λ��
			digit[2] = DAC6571_code / 10 % 10;    // ����ʮλ��
            digit[3] = DAC6571_code % 10;         // �����λ��
            
            DAC6571_Fastmode_Operation(DAC6571_code);
		}

        //	�������ʾ����
		if (clock500ms_flag == 1)   // ���0.5�붨ʱ�Ƿ�
		{
			clock500ms_flag = 0;
			// 8��ָʾ��������Ʒ�ʽ��ÿ0.5�����ң�ѭ�����ƶ�һ��
			temp = led[0];
			for (i = 0; i < 7; i++) led[i] = led[i + 1];
			led[7] = temp;
		}
	}
	
}

//*****************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortK������PK4,PK5Ϊ�����ʹ��PortM������PM0Ϊ�����
//          ���װ������ϣ�PK4������TM1638��STB��PK5����TM1638��DIO��PM0����TM1638��CLK��
//          ��ͨ���˹����ӣ�PL0������DAC6571֮SDA��PL1����SCL��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void GPIOInit(void)
{
	//�������ڿ���TM1638оƬ��DAC6571оƬ�Ĺܽ�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);				// ʹ�ܶ˿� K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// �ȴ��˿� K׼�����		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);				// ʹ�ܶ˿� M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// �ȴ��˿� M׼�����		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);				// ʹ�ܶ˿� L	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)){};		// �ȴ��˿� L׼�����		
    
    // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	


    // ���ö˿� L�ĵ�0,1λ��PL0,PL1��Ϊ�������		PL0-SDA  PL1-SCL (DAC6571)
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0|GPIO_PIN_1);
        
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
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms
	SysTickEnable();  			// SysTickʹ��
	SysTickIntEnable();			// SysTick�ж�����
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
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | 
	                                   SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 
	                                   1000000);

	GPIOInit();             // GPIO��ʼ��
    SysTickInit();          // ����SysTick�ж�
    IntMasterEnable();			// ���ж�����
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void)       // ��ʱ����Ϊ20ms
{
	// 0.1������ʱ������
	if (++clock100ms >= V_T100ms)
	{
		clock100ms_flag = 1; // ��0.1�뵽ʱ�������־��1
		clock100ms = 0;
	}
	
 	// 0.5������ʱ������
	if (++clock500ms >= V_T500ms)
	{
		clock500ms_flag = 1; // ��0.5�뵽ʱ�������־��1
		clock500ms = 0;
	}
	
	// ˢ��ȫ������ܺ�LEDָʾ��
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ��һλ�������
	key_code = TM1638_Readkeyboard();

//	if (key_code != 0)
//	{
//		if (key_cnt < 4) key_cnt++;   // ����������4*20ms
//		else if (key_cnt == 4)
//		{
//			if (key_code == 1)      // ��1
//			{
//				if (DAC6571_code < DAC6571_code_max) 
//				{
//					DAC6571_code++;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 2)  // ��1
//			{
//				if (DAC6571_code > 0) 
//				{
//					DAC6571_code--;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 3)  // ��10
//			{
//				if (DAC6571_code < DAC6571_code_max - 10) 
//				{
//					DAC6571_code += 10;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 4)   // ��10
//			{
//				if (DAC6571_code > 10) 
//				{
//					DAC6571_code -= 10;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 5)   // ��100
//			{
//				if (DAC6571_code < DAC6571_code_max - 100) 
//				{
//					DAC6571_code += 100;
//					DAC6571_flag = 1;
//				}
//			}
//			else if (key_code == 6)   // ��100
//			{
//				if (DAC6571_code > 100) 
//				{
//					DAC6571_code -= 100;
//					DAC6571_flag = 1;
//				}
//			}

//			key_cnt = 5;   // ����һֱ���ţ�ֻ�ı�һ��
//		}
//	}
//	else key_cnt = 0;

	if (key_code != 0)
	{
		if (key_cnt < 4) key_cnt++;   // ����������4*20ms
		else if (key_cnt == 4)
		{
			
            switch(key_code)
            {
                case 1:     // ��100
                    if (DAC6571_code < DAC6571_code_max - 100) 
				    {
					     DAC6571_code += 100;
					     DAC6571_flag = 1;
				    }
                    break;
                case 4:    // ��100
                    if (DAC6571_code > 100) 
				    {
					    DAC6571_code -= 100;
					    DAC6571_flag = 1;
				    }
                    break;
                case 2:    // ��10
                   if (DAC6571_code < DAC6571_code_max - 10) 
				    {
					     DAC6571_code += 10;
					     DAC6571_flag = 1;
				    }                    
                    break;
                case 5:    // ��10
                   if (DAC6571_code >= 10) 
				    {
					    DAC6571_code -= 10;
					    DAC6571_flag = 1;
				    }
                    break;
                case 3:    // ��1
                   if (DAC6571_code < DAC6571_code_max - 1) 
				    {
					     DAC6571_code += 1;
					     DAC6571_flag = 1;
				    }
                    break;
                case 6:    // ��1
                   if (DAC6571_code >= 1) 
				    {
					    DAC6571_code -= 1;
					    DAC6571_flag = 1;
				    }
                    break;
                default:
                    break;
            }
            
			key_cnt = 5;   // ����һֱ���ţ�ֻ�ı�һ��
		}
	}
	else key_cnt = 0;
       
	digit[5] = key_code;   // ����ֵ

}
