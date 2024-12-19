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
#include "inc/hw_memmap.h"		 // ��ַ�궨��
#include "inc/hw_types.h"		 // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"	 // ������
#include "driverlib/gpio.h"		 // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"	 // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"	 // ϵͳ���ƶ���
#include "driverlib/systick.h"	 // SysTick Driver ԭ��
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver ԭ��

#include "tm1638.h" // �����TM1638оƬ�йصĺ���
#include "JLX12864.h"
#include <string.h>

//*****************************************************************************
//
// �궨��
//
//*****************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTickƵ��Ϊ50Hz����ѭ����ʱ����20ms

#define V_T100ms 5	// 0.1s�����ʱ�����ֵ��5��20ms
#define V_T500ms 25 // 0.5s�����ʱ�����ֵ��25��20ms
#define V_T2s 100	// 2s�����ʱ�����ֵ��100��20ms
#define V_T5s 250	// 5s�����ʱ�����ֵ��250��20ms
#define V_T10s 500	// 10s�����ʱ�����ֵ��500��20ms

//*****************************************************************************
//
// ����ԭ������
//
//*****************************************************************************
void GPIOInit(void);	// GPIO��ʼ��
void SysTickInit(void); // ����SysTick�ж�
void DevicesInit(void); // MCU������ʼ����ע���������������

void init_act0(void); // act0�����ʼ��
void init_act1(void); // act1�����ʼ��
void init_act2(void); // act2�����ʼ��
void init_act3(void); // act3�����ʼ��
void init_act4(void); // act4�����ʼ��

void act0_ui_state_proc(uint8_t ui_state); // act0�ܻ���
void act1_ui_state_proc(uint8_t ui_state); // act1�ܻ���
void act2_ui_state_proc(uint8_t ui_state); // act2�ܻ���
void act3_ui_state_proc(uint8_t ui_state); // act3�ܻ���
void act4_ui_state_proc(uint8_t ui_state); // act4�ܻ���

void act0_ui_proc0(void);	 // act0��ʼ��û�й�꣩
void act0_ui_proc_set(void); // act0�ġ����á�

void act1_ui_proc0(void);		   // act1��ʼû�й��
void act1_ui_proc_mode(void);	   // act1������ģʽ��
void act1_ui_proc_parameter(void); // act1������������
void act1_ui_proc_return(void);	   // act1�����ء�

void act2_ui_proc0(void);		 // act2��ʼû�й��
void act2_ui_proc_modeABC(void); // act2��ģʽx��
void act2_ui_proc_yes(void);	 // act2��ȷ����
void act2_ui_proc_no(void);		 // act2��ȡ����

void act3_ui_proc0(void);	  // act3��ʼû�й��
void act3_ui_proc_ones(void); // act3�ġ�(��λ��)��
void act3_ui_proc_tens(void); // act3�ġ�(ʮ��λ��)��
void act3_ui_proc_yes(void);  // act3�ġ�ȷ����
void act3_ui_proc_no(void);	  // act3�ġ�ȡ����

void change_number(uint8_t w, char *actstring);	// �Ӽ���/ʮ��λ����
void change_Letter(uint8_t w, char *actstring); // �Ӽ���ģʽX���еĴ�д��ĸ

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
uint16_t clock5s = 0;
uint16_t clock10s = 0;

// ����״̬��
uint8_t act_state = 0;

// ĳ������LCD��Ļ״̬��״̬
uint8_t ui_state = 0;

// ��¼֮ǰ��LCD״̬
uint8_t ui_prestate = 0;

// �����ʱ�������־
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock2s_flag = 0;
uint8_t act4_clock5s_flag = 0;
uint8_t clock10s_flag = 0;

// �������±�־
uint8_t right_flag = 0;//�Ҽ�
uint8_t left_flag = 0;//���
uint8_t increase_flag = 0;//�ϼ�
uint8_t decrease_flag = 0;//�¼�
uint8_t enter_flag = 0;//�м�
uint8_t ANY_flag = 0;//�Ƿ����κΰ�������

// ����10sû�а������±�־
uint8_t flag_nokey_10s = 0;

// �����ü�����
uint32_t test_counter = 0;

// 8λ�������ʾ�����ֻ���ĸ���ţ��ȶ�����ϴ����ҵ�1��(�±�4)�͵�3������λ(�±�6)����ʾ�»���
// ע����������λ�������������Ϊ4��5��6��7��0��1��2��3
uint8_t digit[8] = {' ', ' ', ' ', ' ', '_', ' ', '_', ' '};

// 8λС���� 1��  0����ʹ���ϴ��ҵ����2������λС��������ע���16�������ӵ�λ����λ�±�Ϊ0~7
// ע����������λС����������������Ϊ4��5��6��7��0��1��2��3
uint8_t pnt = 0x04;

// 8��LEDָʾ��״̬��0��1��
// ע������ָʾ�ƴ������������Ϊ7��6��5��4��3��2��1��0
//     ��ӦԪ��LED8��LED7��LED6��LED5��LED4��LED3��LED2��LED1
// uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0}; ///////////////////////////////////////

// ��ǰ����ֵ
uint8_t key_code = 0;

// �ϴΰ���״̬��־�� 1��δ����   0������
// uint8_t pre_key_code = 0;
uint8_t key_ENTER_prestate = 1;
uint8_t key_LEFT_prestate = 1;
uint8_t key_RIGHT_prestate = 1;
uint8_t key_INCREASE_prestate = 1;
uint8_t key_DECREASE_prestate = 1;
uint8_t key_prestate = 1;

// ����ʾ���ַ�Ϊ0~6��7��ģ�飬�洢��ģ���������
// ��Ӧģ��     0    1   2   3    4    5    6
// uint8_t x[] = {3,   3,  5,   5,   5,   5,   5};
// uint8_t y[] = {1, 11,  1, 11, 12, 13, 14};

// ֻ��ģʽA��"(��λ��)1"��"(ʮ��λ��)1"��Ϊ�ַ�����������,�±�0~2(ԭ��ģ�������У���13���ַ���)
// /*unsigned*/ char str[13][10] = {"����ģʽ" , "ģʽA" , "��������" ,  "1",   "."  ,  "1", "Hz" , "��" , "����" , "����" , "ȷ��" , "ȡ��" , "�����������Ϸ�"};
char str[3][10] = {"ģʽA", "1", "1"};
char initial_modeX[10];					 // ���ڱ���ĳ�������act2����ʱ��ʼ�ġ�ģʽX������
char initial_unit[10], initial_tens[10]; // ���ڱ���ĳ�������act3����ʱ��ʼ�ĸ�λ��ʮ��λ

// ϵͳʱ��Ƶ��
uint32_t ui32SysClock;

//*****************************************************************************
//
// ������
//
//*****************************************************************************
int main(void)
{
	uint8_t temp, i;

	DevicesInit(); //  MCU������ʼ��

	while (clock100ms < 3)
		; // ��ʱ>60ms,�ȴ�TM1638�ϵ����

	TM1638_Init(); // ��ʼ��TM1638
	initial_lcd(); // LCD��ʼ��

	init_act0(); // �����ʼ��Ϊact0�Ļ���

	while (1)
	{
		if (clock100ms_flag == 1) // ���0.1�붨ʱ�Ƿ�
		{
			clock100ms_flag = 0;
			// ÿ0.1���ۼӼ�ʱֵ�����������ʮ������ʾ���м�����(key_code!=0)ʱ��ͣ��ʱ(����ͣ����test_counter��digit[0~3])
			if (key_code == 0)
			{
				if (++test_counter >= 10000)
					test_counter = 0;
				digit[0] = test_counter / 1000;		// �����λ��
				digit[1] = test_counter / 100 % 10; // ����ʮλ��
				digit[2] = test_counter / 10 % 10;	// �����λ��
				digit[3] = test_counter % 10;		// ����ʮ��λ��
			}
		}

		/*if (clock500ms_flag == 1)   // ���0.5�붨ʱ�Ƿ�
		{
			clock500ms_flag = 0;
			// 8��ָʾ��������Ʒ�ʽ��ÿ0.5�����ң�ѭ�����ƶ�һ��(��ӦΨһ�������Ǹ�������(ѭ��)�ƶ�һ��)
			temp = led[0];
			for (i = 0; i < 7; i++) led[i] = led[i + 1];
			led[7] = temp;
		}*/
		//////////////////////////////////////////////////////////////////////////

		switch (act_state)
		{
		case 0:
			act0_ui_state_proc(ui_state);
			break;
		case 1:
			act1_ui_state_proc(ui_state);
			break;
		case 2:
			act2_ui_state_proc(ui_state);
			break;
		case 3:
			act3_ui_state_proc(ui_state);
			break;
		case 4:
			act4_ui_state_proc(ui_state);
			break;

		default:
			act_state = 0;
			break;
		}
	}
}

//************************************************************************
void init_act0(void)
{
	clear_screen();

	display_GB2312_string(3, 17, str[0], 0); //(3 - 1) * 8 + 1=17
	display_GB2312_string(3, 81, str[1], 0); //(11 - 1) * 8 + 1=81
	display_GB2312_string(3, 89, ".", 0);	 //(12 - 1) * 8 + 1=89
	display_GB2312_string(3, 97, str[2], 0); //(13 - 1) * 8 + 1=97
	display_GB2312_string(3, 105, "Hz", 0);	 //(14 - 1) * 8 + 1=105
	display_GB2312_string(7, 1, "����", 0);
}

//*************************************************************
void init_act1(void)
{
	clear_screen();

	display_GB2312_string(3, 17, "����ģʽ", 0); //(3 - 1) * 8 + 1=17
	display_GB2312_string(5, 17, "��������", 0); //(3 - 1) * 8 + 1=17
	display_GB2312_string(7, 105, "����", 0);	 //(13 - 1) * 8 + 1=97
}

//********************************************************************
void init_act2(void)
{
	clear_screen();

	strcpy(initial_modeX, str[0]); // �ȱ����ʼ�ġ�ģʽX������

	display_GB2312_string(3, 1, "����ģʽ��", 0);
	display_GB2312_string(3, 81, str[0], 0); //(11 - 1) * 8 + 1=81
	display_GB2312_string(7, 1, "ȷ��", 0);
	display_GB2312_string(7, 97, "ȡ��", 0); //(13 - 1) * 8 + 1=97
}

//*****************************************************************
void init_act3(void)
{
	clear_screen();

	strcpy(initial_unit, str[1]);
	strcpy(initial_tens, str[2]); // �ȱ����ʼ�ĸ�λ��ʮ��λ����

	display_GB2312_string(3, 1, "����������", 0);
	display_GB2312_string(3, 81, str[1], 0); //(11 - 1) * 8 + 1=81
	display_GB2312_string(3, 89, ".", 0);	 //(12 - 1) * 8 + 1=89
	display_GB2312_string(3, 97, str[2], 0); //(13 - 1) * 8 + 1=97
	display_GB2312_string(3, 105, "Hz", 0);	 //(14 - 1) * 8 + 1=105
	display_GB2312_string(7, 1, "ȷ��", 0);
	display_GB2312_string(7, 97, "ȡ��", 0); //(13 - 1) * 8 + 1=97
}

//***************************************************************************
void init_act4(void)
{
	clear_screen();

	display_GB2312_string(3, 1, "�����������Ϸ�", 0);
}


//******************************************
void act0_ui_state_proc(uint8_t ui_state)
{
	switch (ui_state)
	{
	case 0:
		act0_ui_proc0();
		break;
	case 1:
		act0_ui_proc_set();
		break;
\
	default:
		ui_state = 0;
		break;
	}
}


//*******************************************************
void act1_ui_state_proc(uint8_t ui_state)
{
	switch (ui_state)
	{
	case 0:
		act1_ui_proc0();
		break;
	case 1:
		act1_ui_proc_mode();
		break;
	case 2:
		act1_ui_proc_parameter();
		break;
	case 3:
		act1_ui_proc_return();
		break;

	default:
		ui_state = 0;
		break;
	}
}


//**************************************************
void act2_ui_state_proc(uint8_t ui_state)
{
	switch (ui_state)
	{
	case 0:
		act2_ui_proc0();
		break;
	case 1:
		act2_ui_proc_modeABC();
		break;
	case 2:
		act2_ui_proc_yes();
		break;
	case 3:
		act2_ui_proc_no();
		break;

	default:
		ui_state = 0;
		break;
	}
}


//*****************************************************
void act3_ui_state_proc(uint8_t ui_state)
{
	switch (ui_state)
	{
	case 0:
		act3_ui_proc0();
		break;
	case 1:
		act3_ui_proc_ones();
		break;
	case 2:
		act3_ui_proc_tens();
		break;
	case 3:
		act3_ui_proc_yes();
		break;
	case 4:
		act3_ui_proc_no();
		break;

	default:
		ui_state = 0;
		break;
	}
}


//*********************************************
void act4_ui_state_proc(uint8_t ui_state)
{
	// display_GB2312_string(3, 1, "�����������Ϸ�", 0);
	if (act4_clock5s_flag == 1) // ��ת��act3
	{
		act4_clock5s_flag = 0;
		ANY_flag = 0;
		init_act3();
		act_state = 3;
		ui_state = 0;
	}
}



//************************************************
void act0_ui_proc0(void)
{
	if (ANY_flag)
	{
		ANY_flag = 0;
		display_GB2312_string(7, 1, "����", 1);
		ui_state = 1;
	}
	ui_prestate = 0;
}


//************************************************
void act0_ui_proc_set(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} 

	if (right_flag)
	{
		right_flag = 0;
	}
	else if (left_flag)
	{
		left_flag = 0;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
	}
	else if (enter_flag) // ��ת��act1
	{
		enter_flag = 0;
		ANY_flag = 0;
	
		init_act1();
		act_state = 1;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(7, 1, "����", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//********************************************************
void act1_ui_proc0(void)
{
	if (ANY_flag)
	{
		ANY_flag = 0;
		display_GB2312_string(3, 17, "����ģʽ", 1);//(3 - 1) * 8 + 1=17
		ui_state = 1;
	}
	ui_prestate = 0;
}


//****************************************************8
void act1_ui_proc_mode(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (3 - 1) * 8 + 1, "����ģʽ", 0);
		display_GB2312_string(5, (3 - 1) * 8 + 1, "��������", 1);
		ui_state = 2;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(3, (3 - 1) * 8 + 1, "����ģʽ", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "����", 1);
		ui_state = 3;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 1;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 1;
	}
	else if (enter_flag) // ��ת��act2
	{
		enter_flag = 0;
		ANY_flag = 0;
		init_act2();
		act_state = 2;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(3, (3 - 1) * 8 + 1, "����ģʽ", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//**************************************************
void act1_ui_proc_parameter(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(5, (3 - 1) * 8 + 1, "��������", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "����", 1);
		ui_state = 3;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(5, (3 - 1) * 8 + 1, "��������", 0);
		display_GB2312_string(3, (3 - 1) * 8 + 1, "����ģʽ", 1);
		ui_state = 1;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 2;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 2;
	}
	else if (enter_flag) // ��ת��act3
	{
		enter_flag = 0;
		ANY_flag = 0;
		init_act3();
		act_state = 3;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(5, (3 - 1) * 8 + 1, "��������", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//****************************************************
void act1_ui_proc_return(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "����", 0);
		display_GB2312_string(3, (3 - 1) * 8 + 1, "����ģʽ", 1);
		ui_state = 1;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "����", 0);
		display_GB2312_string(5, (3 - 1) * 8 + 1, "��������", 1);
		ui_state = 2;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 3;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 3;
	}
	else if (enter_flag) // ��ת��act0
	{
		enter_flag = 0;
		ANY_flag = 0;
		init_act0();
		act_state = 0;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "����", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act2_ui_proc0(void)
{
	if (ANY_flag)
	{
		ANY_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 1);
		ui_state = 1;
	}
	ui_prestate = 0;
}


//******************************************************
void act2_ui_proc_modeABC(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 0);
		display_GB2312_string(7, 1, "ȷ��", 1);
		ui_state = 2;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 1);
		ui_state = 3;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		change_Letter(1, str[0]);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 1);
		ui_state = 1;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		change_Letter(2, str[0]);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 1);
		ui_state = 1;
	}
	else if (enter_flag)
	{
		enter_flag = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act2_ui_proc_yes(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, 1, "ȷ��", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 1);
		ui_state = 3;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, 1, "ȷ��", 0);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 1);
		ui_state = 1;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 2;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 2;
	}
	else if (enter_flag) // ��ת��act1����ģʽX���óɹ�������³�ʼֵ
	{
		enter_flag = 0;
		ANY_flag = 0;
		strcpy(initial_modeX, str[0]);
		init_act1();
		act_state = 1;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(7, 1, "ȷ��", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act2_ui_proc_no(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 0);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 1);
		ui_state = 1;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 0);
		display_GB2312_string(7, 1, "ȷ��", 1);
		ui_state = 2;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 3;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 3;
	}
	else if (enter_flag) // ��ת��act1����ģʽX����ʧ�ܣ��踴ԭΪ��ʼֵ
	{
		enter_flag = 0;
		ANY_flag = 0;
		strcpy(str[0], initial_modeX);
		init_act1();
		act_state = 1;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act3_ui_proc0(void)
{
	if (ANY_flag)
	{
		ANY_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 1);
		ui_state = 1;
	}
	ui_prestate = 0;
}


//	******************************************************
void act3_ui_proc_ones(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 0);
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 1);
		ui_state = 2;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 1);
		ui_state = 4;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		change_number(1, str[1]);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 1);
		ui_state = 1;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		change_number(2, str[1]);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 1);
		ui_state = 1;
	}
	else if (enter_flag)
	{
		enter_flag = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act3_ui_proc_tens(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 0);
		display_GB2312_string(7, 1, "ȷ��", 1);
		ui_state = 3;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 0);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 1);
		ui_state = 1;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		change_number(1, str[2]);
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 1);
		ui_state = 2;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		change_number(2, str[2]);
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 1);
		ui_state = 2;
	}
	else if (enter_flag)
	{
		enter_flag = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act3_ui_proc_yes(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, 1, "ȷ��", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 1);
		ui_state = 4;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, 1, "ȷ��", 0);
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 1);
		ui_state = 2;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 3;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 3;
	}
	else if (enter_flag) // ��ת��act1��act4 !
	{
		uint8_t value_10times = (str[1][0] - '0') * 10 + (str[2][0] - '0');
		if (value_10times >= 7 && value_10times <= 77) // �������Ϸ�������ת��act1���Ҹ�λ��ʮλ���óɹ�������³�ʼֵ
		{
			led[7] = 0;
			enter_flag = 0;
			ANY_flag = 0;
			strcpy(initial_unit, str[1]);
			strcpy(initial_tens, str[2]);
			init_act1();
			act_state = 1;
			ui_state = 0;
		}
		else // ���������Ϸ�������ת��act4���Ҹ�λ��ʮλ����ʧ�ܣ��踴ԭΪ��ʼֵ
		{
			led[7] = 1;
			enter_flag = 0;
			ANY_flag = 0;
			strcpy(str[1], initial_unit);
			strcpy(str[2], initial_tens);
			init_act4();
			act_state = 4;
			ui_state = 0;
		}
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(7, 1, "ȷ��", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void act3_ui_proc_no(void)
{
	if (ui_prestate == 0)
	{
		ui_prestate = 1;
		clear_flags();
	} // ��״̬0��״̬1�Ա�־λ��Ӱ�첻��
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 0);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 1);
		ui_state = 1;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 0);
		display_GB2312_string(7, 1, "ȷ��", 1);
		ui_state = 3;
	}
	else if (increase_flag)
	{
		increase_flag = 0;
		ui_state = 4;
	}
	else if (decrease_flag)
	{
		decrease_flag = 0;
		ui_state = 4;
	}
	else if (enter_flag) // ��ת��act1���Ҹ�λ��ʮ��λ����ʧ�ܣ����踴ԭΪ��ʼֵ
	{
		enter_flag = 0;
		ANY_flag = 0;
		strcpy(str[1], initial_unit);
		strcpy(str[2], initial_tens);
		init_act1();
		act_state = 1;
		ui_state = 0;
	}
	if (flag_nokey_10s == 1)
	{
		flag_nokey_10s = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "ȡ��", 0);
		ANY_flag = 0; // ���һ�ΰ��µĲ���
		ui_state = 0;
	}
}


//******************************************************
void clear_flags(void)
{
	right_flag = 0;
	left_flag = 0;
	increase_flag = 0;
	decrease_flag = 0;
	enter_flag = 0;
}


//******************************************************
void change_number(uint8_t w, /*unsigned*/ char *actstring) // w=1: increase; w=2: decrease; ��������
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


//******************************************************
void change_Letter(uint8_t w, /*unsigned*/ char *actstring) // w=1: increase; w=2: decrease; ��ĸ����
{
	actstring += 4; // �����ֵ��ַ�����һ������ռ��������� 
	if (w == 1)
	{
		(*actstring)++;
		if (*actstring > 'C')
			*actstring = 'A';
	}
	else if (w == 2)
	{
		(*actstring)--;
		if (*actstring < 'A')
			*actstring = 'C';
	}
}


//******************************************************
void ENTER_detect(void)
{
	if (key_code == 5)
	{
		if (key_ENTER_prestate == 1)
			enter_flag = 1;
		key_ENTER_prestate = 0;
	}
	else
		key_ENTER_prestate = 1;
}


//******************************************************
void LEFT_detect(void)
{
	if (key_code == 4)
	{
		if (key_LEFT_prestate == 1)
			left_flag = 1;
		key_LEFT_prestate = 0;
	}
	else
		key_LEFT_prestate = 1;
}


//******************************************************
void RIGHT_detect(void)
{
	if (key_code == 6)
	{
		if (key_RIGHT_prestate == 1)
			right_flag = 1;
		key_RIGHT_prestate = 0;
	}
	else
		key_RIGHT_prestate = 1;
}

void INCREASE_detect(void)
{
	if (key_code == 2)
	{
		if (key_INCREASE_prestate == 1)
			increase_flag = 1;
		key_INCREASE_prestate = 0;
	}
	else
		key_INCREASE_prestate = 1;
}

void DECREASE_detect(void)
{
	if (key_code == 8)
	{
		if (key_DECREASE_prestate == 1)
			decrease_flag = 1;
		key_DECREASE_prestate = 0;
	}
	else
		key_DECREASE_prestate = 1;
}

void ANYKEY_detect()
{
	if (key_code != 0)
	{
		if (key_prestate == 1)
			ANY_flag = 1;
		key_prestate = 0;
	}
	else
		key_prestate = 1;
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
	SysTickEnable();									// SysTickʹ��
	SysTickIntEnable();									// SysTick�ж�����
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

	GPIOInit();		   // GPIO��ʼ��
	SysTickInit();	   // ����SysTick�ж�
	IntMasterEnable(); // ���ж�����
}

//*****************************************************************************
//
// ����ԭ�ͣ�void SysTick_Handler(void)
// �������ܣ�SysTick�жϷ������ע����ÿ��20ms�Զ�����һ��
// ������������
// ��������ֵ����
//
//*****************************************************************************
void SysTick_Handler(void) // ��ʱ����Ϊ20ms
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

	// 2���������ʱ������
	/*if (++clock2s >= V_T2s)
	{
		clock2s_flag = 1;       // ��2�뵽ʱ�������־��1
		clock2s = 0;
	}*/

	// 10���������ʱ������
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
		if (++clock10s >= V_T10s) // 10s��ʱ���������
		{
			flag_nokey_10s = 1; // ��10s��ʱ�������־��1
			clock10s = 0;
		}
	}
	else
		clock10s = 0;

	if (act_state == 4)
	{
		if (++clock5s >= V_T5s)
		{
			act4_clock5s_flag = 1;
			clock5s = 0;
		}
	}
	else
		clock5s = 0;

	/*if (pre_key_code != 6 && key_code==6){clock10s_flag = 0; right_flag =1;}                                      //���±�־key_RIGHT_flag
	else if ( pre_key_code != 4 && key_code==4 ){clock10s_flag = 0; left_flag =1;}                                //���±�־left_flag
	else if ( pre_key_code != 2 && key_code==2 ){clock10s_flag = 0; increase_flag = 1;}                      //���±�־increase_flag
	else if ( pre_key_code != 8 && key_code==8 ){clock10s_flag = 0; decrease_flag = 1;}                     //���±�־key_DECREASE_flag
	else if ( pre_key_code != 5 && key_code==5 ){clock10s_flag = 0; enter_flag =1;}                             //���±�־key_ENTER_flag
	else if ( pre_key_code != 1&&key_code==1 || pre_key_code != 3&&key_code==3 || pre_key_code != 7&&key_code==7 || pre_key_code != 9&&key_code==9 )clock10s_flag = 0;
	else if ( ++clock10s >= V_T10s ){flag_nokey_10s = 1; clock10s_flag = 1; clock10s = 0;}                 //���±�־flag_nokey_10s

	// ����ǰ����ֵ����pre_key_code
	pre_key_code = key_code;*/
}
