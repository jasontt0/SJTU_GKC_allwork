//*****************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: exp2_0.c
// Description:
//    1.������λ�󣬵װ����ұ�4λ������Զ���ʾ��ʱ��ֵ�����λ��Ӧ��λ��0.1�룻
//    2.������λ�󣬵װ���8��LED�����������ʽ��������ѭ���任��Լ0.5��任1�Σ�
//    3.��û�а�������ʱ����������ڶ�λ�������ʾ��0����
//      ���˹�����ĳ�����������ʾ�ü��ı�ţ�
//      �˿���λ��ʱ�������ͣ�仯��ֹͣ��ʱ��ֱ���ſ��������Զ�������ʱ��
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20201228
// Date��2020-12-28
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
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	  // ϵͳ���ƶ���
#include "driverlib/systick.h"    // SysTick Driver ԭ��
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver ԭ��

#include "tm1638.h"               // �����TM1638оƬ�йصĺ���

#include "driverlib/uart.h"       // UART��غ궨��
#include "utils/uartstdio.h"      // UART0��Ϊ����̨��غ���ԭ������

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T100ms	5                   // 0.1s�����ʱ�����ֵ��5��20ms
#define V_T500ms	25                  // 0.5s�����ʱ�����ֵ��25��20ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж�
void DevicesInit(void);     // MCU������ʼ����ע���������������

void counter_for_100ms(void);	//����1��Ƶ��Ϊ100ms�ļ�ʱ����ʾ
void set_counter_to_0(void);
//void flash_light_run_right(void);	//����2������ƣ���������
//void flash_light_run_left(void);	//����2������ƣ���������
void show_key_number(void);	//����3����ʾ��ֵ

void switch_mode(void);	//ת�䲻ͬ����ģʽ
void run_mode(void);	//���в�ͬ�Ĺ���ģʽ
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
uint8_t digit[8]= {' ',' ',' ',' ','_',' ','_',' '};

// 8λС���� 1��  0��
// ע����������λС����������������Ϊ4��5��6��7��0��1��2(��)��3
uint8_t pnt = 0x04;//4=>������2��λΪ1������Ϊ0

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 1};

// ��ǰ����ֵ
uint8_t key_code = 0;

// ϵͳʱ��Ƶ��
uint32_t ui32SysClock;

//ָʾ����ģʽ
bool mode=true;

//*****************************************************************************
//
// ������
//
//*****************************************************************************
int main(void)
{

  DevicesInit();            //  MCU������ʼ��

  while (clock100ms < 3);   // ��ʱ>60ms,�ȴ�TM1638�ϵ����
  TM1638_Init();	          // ��ʼ��TM1638
  
	while (1)
	{

		
		if(key_code==1) switch_mode();

		run_mode();	

		if(key_code==2) set_counter_to_0();
		
	}
	
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
  //����TM1638оƬ�ܽ�
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);		  // ʹ�ܶ˿� K
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) {}; // �ȴ��˿� K׼�����

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);		  // ʹ�ܶ˿� M
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) {}; // �ȴ��˿� M׼�����
		
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);		   // ʹ�ܶ˿� J
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}; // �ȴ��˿� J׼�����

  // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
  // ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
  GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);
		
	// ���ö˿� J�ĵ�0λ��PJ0��Ϊ��������
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
	// �˿� J�ĵ�0λ��Ϊ�������룬�������óɡ�����������
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		
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
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); // ������������,��ʱ����20ms=1/50s
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
                                    20000000);

  GPIOInit();             // GPIO��ʼ��
  SysTickInit();          // ����SysTick�ж�
  IntMasterEnable();		// ���ж�����
}

//*****************************************************************************
//
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void)       // ��ʱ����Ϊ20ms,��ÿ20ms�Զ�����һ�η�����
{
  // 0.1������ʱ������
  if (++clock100ms >= V_T100ms)
    {
      clock100ms_flag = 1;    // ��0.1�뵽ʱ�������־��1
      clock100ms = 0;
    }

  // 0.5������ʱ������
  if (++clock500ms >= V_T500ms)
    {
      clock500ms_flag = 1;    // ��0.5�뵽ʱ�������־��1
      clock500ms = 0;
    }

  // ˢ��ȫ������ܺ�LEDָʾ��
  TM1638_RefreshDIGIandLED(digit, pnt, led);

	show_key_number();//����3����
		
	
}



//****************************************************************************
//����1
void counter_for_100ms(void)
{
	if (clock100ms_flag == 1)      // ���0.1�붨ʱ�Ƿ�
		{
			clock100ms_flag		= 0;// ÿ0.1���ۼӼ�ʱֵ�����������ʮ������
          
			if (++test_counter >= 10000) test_counter = 0;
			digit[0] = test_counter / 1000; 	    // �����λ��
      digit[1] = test_counter / 100 % 10; 	// ����ʮλ��
      digit[2] = test_counter / 10 % 10; 	    // �����λ��
      digit[3] = test_counter % 10;           // ����ٷ�λ��
     }
}



//********************************************
void set_counter_to_0(void)
{
	test_counter = 0; 
  digit[0] = test_counter / 1000; 	    // �����λ��
  digit[1] = test_counter / 100 % 10; 	// ����ʮλ��
  digit[2] = test_counter / 10 % 10; 	    // �����λ��
	digit[3] = test_counter % 10;           // ����ٷ�λ��        
}




		
//***********************************************************
//����3
void show_key_number(void)
{
	
	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
  // ������ʾ��һλ�������
  key_code = TM1638_Readkeyboard();

  digit[5] = key_code;//5��λ����ʾ������
	
}



//**********************************
//ת�乤��ģʽ
void switch_mode(void)
{
	if(mode==0)
	{
		mode=1;
	}
	else
		if(mode==1)
		{
			mode=0;
		}
}



//*************************************************
//���е�ǰ����ģʽ
void run_mode(void)
{
	
	if(mode)
	{
		counter_for_100ms();	//����1��Ƶ��Ϊ100ms�ļ�ʱ����ʾ
		led[7]=1;
	}
	
	else led[7]=0;
}

