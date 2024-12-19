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
#include "JLX12864.h"

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY		50		// SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T100ms	5              // 0.1s�����ʱ�����ֵ��5��20ms
#define V_T500ms	25            // 0.5s�����ʱ�����ֵ��25��20ms
#define V_T2s 100                  // 2s�����ʱ�����ֵ��100��20ms
#define V_T10s 500                // 10s�����ʱ�����ֵ��500��20ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);        // GPIO��ʼ��
void SysTickInit(void);     // ����SysTick�ж� 
void DevicesInit(void);     // MCU������ʼ����ע���������������
void initial_act(void);
void ui_state_proc(uint8_t ui_state);
void ui_proc0(void);
void ui_proc001(void);
void ui_proc003(void);
void ui_proc005(void);
void in_de_num ( uint8_t w, /*unsigned*/ char* actstring );
void in_de_capletter ( uint8_t w, /*unsigned*/ char* actstring );
void ENTER_detect(void);
void LEFT_detect(void);
void RIGHT_detect(void);
void INCREASE_detect(void);
void DECREASE_detect(void);
void ANYKEY_detect(void);
void clear_flags(void);
//*****************************************************************************
//
// ��������
//
//*****************************************************************************

// �����ʱ������
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;
uint8_t clock2s = 0;
uint16_t clock10s = 0;

//LCD��Ļ״̬��
uint8_t ui_state = 0;

//��¼֮ǰ��LCD״̬
uint8_t ui_prestate = 0; 

// �����ʱ�������־
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock2s_flag = 0;
uint8_t clock10s_flag = 0;

//�������±�־
uint8_t key_RIGHT_flag = 0;
uint8_t key_LEFT_flag = 0;
uint8_t key_INCREASE_flag = 0;
uint8_t key_DECREASE_flag = 0;
uint8_t key_ENTER_flag = 0;
uint8_t key_ANY_flag = 0;

// ����10sû�а������±�־
uint8_t NOKEY_clock10s_flag = 0;

// �����ü�����
uint32_t test_counter = 0;

// 8λ�������ʾ�����ֻ���ĸ���ţ��ȶ�����ϴ����ҵ�1��(�±�4)�͵�3������λ(�±�6)����ʾ�»���
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8]={' ',' ',' ',' ','_',' ','_',' '};

// 8λС���� 1��  0����ʹ���ϴ��ҵ����2������λС��������ע���16�������ӵ�λ����λ�±�Ϊ0~7
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x04;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};

// ��ǰ����ֵ
uint8_t key_code = 0;

//�ϴΰ���״̬��־�� 1��δ����   0������
//uint8_t pre_key_code = 0;
uint8_t key_ENTER_prestate = 1;
uint8_t key_LEFT_prestate = 1;
uint8_t key_RIGHT_prestate = 1;
uint8_t key_INCREASE_prestate = 1;
uint8_t key_DECREASE_prestate = 1;
uint8_t key_prestate = 1;

// ����ʾ���ַ�Ϊ0~6��7��ģ�飬�洢��ģ���������
//��Ӧģ��     0    1   2   3    4    5    6
uint8_t x[] = {3,   3,  5,   5,   5,   5,   5};
uint8_t y[] = {1, 11,  1, 11, 12, 13, 14};

// ��ģ������
/*unsigned*/ char str[7][10] = {"����ģʽ��", "ģʽA", "����������",  "1",   ".",   "1", "Hz"};

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
	initial_lcd();//LCD��ʼ��
	initial_act();//�����ʼ��
	
	while (1)
	{				
		if (clock100ms_flag == 1)      // ���0.1�붨ʱ�Ƿ�
		{
			clock100ms_flag		= 0;
			// ÿ0.1���ۼӼ�ʱֵ�����������ʮ������ʾ���м�����(key_code!=0)ʱ��ͣ��ʱ(����ͣ����test_counter��digit[0~3])
			if (key_code == 0)
			{
				if (++test_counter >= 10000) test_counter = 0;//ע��test_counter�ӵ�10000ʱ��Ӧ���ܼ�ʱΪ1000.0s����ʱtest_counter�������¼�ʱ
				digit[0] = test_counter / 1000; 	    // �����λ��
				digit[1] = test_counter / 100 % 10; 	// ����ʮλ��
				digit[2] = test_counter / 10 % 10; 	  // �����λ��
				digit[3] = test_counter % 10;         // ����ʮ��λ��
			}
		}

		if (clock500ms_flag == 1)   // ���0.5�붨ʱ�Ƿ�
		{
			clock500ms_flag = 0;
			// 8��ָʾ��������Ʒ�ʽ��ÿ0.5�����ң�ѭ�����ƶ�һ��(��ӦΨһ�������Ǹ�������(ѭ��)�ƶ�һ��)
			temp = led[0];
			for (i = 0; i < 7; i++) led[i] = led[i + 1];
			led[7] = temp;
		}
		
		ui_state_proc( ui_state );
  }
}

void initial_act(void)
{
	clear_screen();
	//display_GB2312_string(1, 1, "                          ", 0);//��һ��
	display_GB2312_string(x[0], (y[0]-1)*8+1, str[0], 0);
	display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 0);
	display_GB2312_string(x[2], (y[2]-1)*8+1, str[2], 0);
	display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 0);
	display_GB2312_string(x[4], (y[4]-1)*8+1, str[4], 0);
	display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 0);
	display_GB2312_string(x[6], (y[6]-1)*8+1, str[6], 0);
	//display_GB2312_string(7, 1, "                          ", 0);//������
	//display_GB2312_string(3, (16-1)*8+1, "  ", 0);        //�ڶ���ĩ
	//display_GB2312_string(5, (16-1)*8+1, "  ", 0);        //������ĩ
}

void ui_state_proc(uint8_t ui_state)
{
	switch ( ui_state )
	{
		case 0:  ui_proc0();  break;
		case 001:  ui_proc001();  break;
		case 003:  ui_proc003();  break;
		case 005:  ui_proc005();  break;
		
		default:  ui_state=0; break;
	}
}

void ui_proc0(void)
{	
	if(key_ANY_flag)
  {
      key_ANY_flag = 0;
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 1);
		  //LCD_delay(86000);
      ui_state=001;
  }
	//LCD_delay(21500);
	ui_prestate = 0;
}

void ui_proc001(void)
{
	if(ui_prestate == 0){ui_prestate = 1; clear_flags();}//��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if( key_RIGHT_flag )
  {
      key_RIGHT_flag=0;
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 0);
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 1);
      ui_state=003;
  }
  else if(key_LEFT_flag)
	{
      key_LEFT_flag=0;
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 0);
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 1);
      ui_state=005;
  }
  else if (key_INCREASE_flag)
  {
      key_INCREASE_flag=0;
      in_de_capletter(1, str[1]);
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 1);
      ui_state=001;
	}
  else if (key_DECREASE_flag)
	{
      key_DECREASE_flag=0;
      in_de_capletter(2, str[1]);
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 1);
      ui_state=001;
	}
  else if (key_ENTER_flag)
	{
      key_ENTER_flag=0;
	}
  if(NOKEY_clock10s_flag==1)
	{
      NOKEY_clock10s_flag=0;
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 0);
		  key_ANY_flag = 0;//���һ�ΰ��µĲ���
      ui_state=0;
  }
}

void clear_flags(void)
{
	key_RIGHT_flag = 0;
  key_LEFT_flag = 0;
  key_INCREASE_flag = 0;
  key_DECREASE_flag = 0;
  key_ENTER_flag = 0;
}

void ui_proc003(void)
{
	if(key_RIGHT_flag)
  {
      key_RIGHT_flag=0;
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 0);
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 1);
      ui_state=005;
  }
  else if(key_LEFT_flag)
	{
      key_LEFT_flag=0;
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 0);
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 1);
      ui_state=001;
  }
  else if (key_INCREASE_flag)
  {
      key_INCREASE_flag=0;
      in_de_num(1, str[3]);
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 1);
      ui_state=003;
	}
  else if (key_DECREASE_flag)
	{
      key_DECREASE_flag=0;
      in_de_num(2, str[3]);
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 1);
      ui_state=003;
	}
  else if (key_ENTER_flag)
	{
      key_ENTER_flag=0;
	}
  if(NOKEY_clock10s_flag==1)
	{
      NOKEY_clock10s_flag=0;
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 0);
		  key_ANY_flag = 0;//���һ�ΰ��µĲ���
      ui_state=0;
  }
}

void ui_proc005(void)
{
	if(key_RIGHT_flag)
  {
      key_RIGHT_flag=0;
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 0);
      display_GB2312_string(x[1], (y[1]-1)*8+1, str[1], 1);
      ui_state=001;
  }
  else if(key_LEFT_flag)
	{
      key_LEFT_flag=0;
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 0);
      display_GB2312_string(x[3], (y[3]-1)*8+1, str[3], 1);
      ui_state=003;
  }
  else if (key_INCREASE_flag)
  {
      key_INCREASE_flag=0;
      in_de_num(1, str[5]);
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 1);
      ui_state=005;
	}
  else if (key_DECREASE_flag)
	{
      key_DECREASE_flag=0;
      in_de_num(2, str[5]);
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 1);
      ui_state=005;
	}
  else if (key_ENTER_flag)
	{
      key_ENTER_flag=0;
	}
  if(NOKEY_clock10s_flag==1)
	{
      NOKEY_clock10s_flag=0;
      display_GB2312_string(x[5], (y[5]-1)*8+1, str[5], 0);
		  key_ANY_flag = 0;//���һ�ΰ��µĲ���
      ui_state=0;
  }
}

void in_de_num ( uint8_t w, /*unsigned*/ char* actstring )    //w=1: increase; w=2: decrease; ��������
{
	if ( w==1 )
	{
		(* actstring)++;
		if (*actstring>'9') *actstring = '0';
	}
	else if( w == 2 )
	{
		(* actstring)--;
		if (*actstring<'0') *actstring='9';
	}
}

void in_de_capletter ( uint8_t w, /*unsigned*/ char* actstring )    //w=1: increase; w=2: decrease; ��ĸ����
{
	actstring += 4;
	if ( w==1 )
	{
		(* actstring)++;
		if (*actstring>'C') *actstring = 'A';
	}
	else if( w == 2 )
	{
		(* actstring)--;
		if (*actstring<'A') *actstring='C';
	}
}

void ENTER_detect(void)
{
	if (key_code == 5)
	{
		if ( key_ENTER_prestate == 1 ) key_ENTER_flag =1;
		key_ENTER_prestate = 0;
	}
	else key_ENTER_prestate = 1;
}

void LEFT_detect(void)
{
	if (key_code == 4)
	{
		if ( key_LEFT_prestate == 1 ) key_LEFT_flag =1;
		key_LEFT_prestate = 0;
	}
	else key_LEFT_prestate = 1;
}

void RIGHT_detect(void)
{
	if (key_code == 6)
	{
		if ( key_RIGHT_prestate == 1 ) key_RIGHT_flag =1;
		key_RIGHT_prestate = 0;
	}
	else key_RIGHT_prestate = 1;
}

void INCREASE_detect(void)
{
	if (key_code == 2)
	{
		if ( key_INCREASE_prestate == 1 ) key_INCREASE_flag =1;
		key_INCREASE_prestate = 0;
	}
	else key_INCREASE_prestate = 1;
}

void DECREASE_detect(void)
{
	if (key_code == 8)
	{
		if ( key_DECREASE_prestate == 1 ) key_DECREASE_flag =1;
		key_DECREASE_prestate = 0;
	}
	else key_DECREASE_prestate = 1;
}

void ANYKEY_detect()
{
	if (key_code !=0)
	{
		if ( key_prestate == 1 ) key_ANY_flag =1;
		key_prestate = 0;
	}
	else key_prestate = 1;
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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						// ʹ�ܶ˿� K	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)){};		// �ȴ��˿� K׼�����		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);						// ʹ�ܶ˿� M	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)){};		// �ȴ��˿� M׼�����		
	
  // ���ö˿� K�ĵ�4,5λ��PK4,PK5��Ϊ�������		PK4-STB  PK5-DIO
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4|GPIO_PIN_5);
	// ���ö˿� M�ĵ�0λ��PM0��Ϊ�������   PM0-CLK
	GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_0);	
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
	                                   20000000);

	GPIOInit();             // GPIO��ʼ��
    SysTickInit();          // ����SysTick�ж�
    IntMasterEnable();			// ���ж�����
}

//*****************************************************************************
// 
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������ע����ÿ��20ms�Զ�����һ��
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
	
	//2���������ʱ������
	/*if (++clock2s >= V_T2s)
	{
		clock2s_flag = 1;       // ��2�뵽ʱ�������־��1
		clock2s = 0;
	}*/
	
	//10���������ʱ������
	/*if (++clock10s >= V_T10s)
	{
		clock10s_flag = 1;     // ��10�뵽ʱ�������־��1
		clock10s = 0;
	}*/
	
	// ˢ����ʾ���ϵ�ȫ������ܡ�С�����LEDָʾ��
	TM1638_RefreshDIGIandLED(digit, pnt, led);
	
	// ��鵱ǰ�������룬0�����޼�������1-9��ʾ�ж�Ӧ����
	// ������ʾ�ڴ����ҵ�2λ�����(�±�5)��
	key_code = TM1638_Readkeyboard();
 
	digit[5] = key_code;
	
	ENTER_detect();
	LEFT_detect();
	RIGHT_detect();
	INCREASE_detect();
	DECREASE_detect();
	ANYKEY_detect();
	
	if (key_code == 0)
	{
		if (++ clock10s >= V_T10s)//10s��ʱ���������
		{
			NOKEY_clock10s_flag = 1; //��10s��ʱ�������־��1
			clock10s = 0;
		}
	}
	else clock10s = 0;
	
	/*if (pre_key_code != 6 && key_code==6){clock10s_flag = 0; key_RIGHT_flag =1;}                                      //���±�־key_RIGHT_flag
	else if ( pre_key_code != 4 && key_code==4 ){clock10s_flag = 0; key_LEFT_flag =1;}                                //���±�־key_LEFT_flag
	else if ( pre_key_code != 2 && key_code==2 ){clock10s_flag = 0; key_INCREASE_flag = 1;}                      //���±�־key_INCREASE_flag
	else if ( pre_key_code != 8 && key_code==8 ){clock10s_flag = 0; key_DECREASE_flag = 1;}                     //���±�־key_DECREASE_flag
	else if ( pre_key_code != 5 && key_code==5 ){clock10s_flag = 0; key_ENTER_flag =1;}                             //���±�־key_ENTER_flag
	else if ( pre_key_code != 1&&key_code==1 || pre_key_code != 3&&key_code==3 || pre_key_code != 7&&key_code==7 || pre_key_code != 9&&key_code==9 )clock10s_flag = 0;  
	else if ( ++clock10s >= V_T10s ){NOKEY_clock10s_flag = 1; clock10s_flag = 1; clock10s = 0;}                 //���±�־NOKEY_clock10s_flag
	
	// ����ǰ����ֵ����pre_key_code
	pre_key_code = key_code;*/
}
