//*****************************************************************************
//
// Copyright: 2020-2021, 上海交通大学电子工程系实验教学中心
// File name: exp2_0.c
// Description:
//    1.开机或复位后，底板上右边4位数码管自动显示计时数值，最低位对应单位是0.1秒；
//    2.开机或复位后，底板上8个LED灯以跑马灯形式由左向右循环变换，约0.5秒变换1次；
//    3.当没有按键按下时，从左边数第二位数码管显示“0”；
//      当人工按下某键，数码管显示该键的编号；
//      此刻四位计时数码管暂停变化，停止计时，直到放开按键后自动继续计时。
// Author:	上海交通大学电子工程系实验教学中心
// Version: 1.0.0.20201228
// Date：2020-12-28
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
#include "inc/hw_memmap.h"		 // 基址宏定义
#include "inc/hw_types.h"		 // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"	 // 调试用
#include "driverlib/gpio.h"		 // 通用IO口宏定义
#include "driverlib/pin_map.h"	 // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	 // 系统控制定义
#include "driverlib/systick.h"	 // SysTick Driver 原型
#include "driverlib/interrupt.h" // NVIC Interrupt Controller Driver 原型

#include "tm1638.h" // 与控制TM1638芯片有关的函数
#include "JLX12864.h"
#include <string.h>

//*****************************************************************************
//
// 宏定义
//
//*****************************************************************************
#define SYSTICK_FREQUENCY 50 // SysTick频率为50Hz，即循环定时周期20ms

#define V_T100ms 5	// 0.1s软件定时器溢出值，5个20ms
#define V_T500ms 25 // 0.5s软件定时器溢出值，25个20ms
#define V_T2s 100	// 2s软件定时器溢出值，100个20ms
#define V_T5s 250	// 5s软件定时器溢出值，250个20ms
#define V_T10s 500	// 10s软件定时器溢出值，500个20ms

//*****************************************************************************
//
// 函数原型声明
//
//*****************************************************************************
void GPIOInit(void);	// GPIO初始化
void SysTickInit(void); // 设置SysTick中断
void DevicesInit(void); // MCU器件初始化，注：会调用上述函数

void init_act0(void); // act0画面初始化
void init_act1(void); // act1画面初始化
void init_act2(void); // act2画面初始化
void init_act3(void); // act3画面初始化
void init_act4(void); // act4画面初始化

void act0_ui_state_proc(uint8_t ui_state); // act0总画面
void act1_ui_state_proc(uint8_t ui_state); // act1总画面
void act2_ui_state_proc(uint8_t ui_state); // act2总画面
void act3_ui_state_proc(uint8_t ui_state); // act3总画面
void act4_ui_state_proc(uint8_t ui_state); // act4总画面

void act0_ui_proc0(void);	 // act0初始（没有光标）
void act0_ui_proc_set(void); // act0的“设置”

void act1_ui_proc0(void);		   // act1初始没有光标
void act1_ui_proc_mode(void);	   // act1“工作模式”
void act1_ui_proc_parameter(void); // act1“工作参数”
void act1_ui_proc_return(void);	   // act1“返回”

void act2_ui_proc0(void);		 // act2初始没有光标
void act2_ui_proc_modeABC(void); // act2“模式x”
void act2_ui_proc_yes(void);	 // act2“确定”
void act2_ui_proc_no(void);		 // act2“取消”

void act3_ui_proc0(void);	  // act3初始没有光标
void act3_ui_proc_ones(void); // act3的“(个位数)”
void act3_ui_proc_tens(void); // act3的“(十分位数)”
void act3_ui_proc_yes(void);  // act3的“确定”
void act3_ui_proc_no(void);	  // act3的“取消”

void change_number(uint8_t w, char *actstring);	// 加减个/十分位数字
void change_Letter(uint8_t w, char *actstring); // 加减“模式X”中的大写字母

void ENTER_detect(void);
void LEFT_detect(void);
void RIGHT_detect(void);
void INCREASE_detect(void);
void DECREASE_detect(void);
void ANYKEY_detect(void);
void clear_flags(void);
//*****************************************************************************
//
// 变量定义
//
//*****************************************************************************

// 软件定时器计数
uint8_t clock100ms = 0;
uint8_t clock500ms = 0;
uint8_t clock2s = 0;
uint16_t clock5s = 0;
uint16_t clock10s = 0;

// 画面状态号
uint8_t act_state = 0;

// 某画面下LCD屏幕状态机状态
uint8_t ui_state = 0;

// 记录之前的LCD状态
uint8_t ui_prestate = 0;

// 软件定时器溢出标志
uint8_t clock100ms_flag = 0;
uint8_t clock500ms_flag = 0;
uint8_t clock2s_flag = 0;
uint8_t act4_clock5s_flag = 0;
uint8_t clock10s_flag = 0;

// 按键按下标志
uint8_t right_flag = 0;//右键
uint8_t left_flag = 0;//左键
uint8_t increase_flag = 0;//上键
uint8_t decrease_flag = 0;//下键
uint8_t enter_flag = 0;//中键
uint8_t ANY_flag = 0;//是否有任何按键按下

// 经过10s没有按键按下标志
uint8_t flag_nokey_10s = 0;

// 测试用计数器
uint32_t test_counter = 0;

// 8位数码管显示的数字或字母符号，先定义板上从左到右第1个(下标4)和第3个数码位(下标6)中显示下划线
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t digit[8] = {' ', ' ', ' ', ' ', '_', ' ', '_', ' '};

// 8位小数点 1亮  0灭。先使板上从右到左第2个数码位小数点亮，注意该16进制数从低位到高位下标为0~7
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
uint8_t pnt = 0x04;

// 8个LED指示灯状态，0灭，1亮
// 注：板上指示灯从左到右序号排列为7、6、5、4、3、2、1、0
//     对应元件LED8、LED7、LED6、LED5、LED4、LED3、LED2、LED1
// uint8_t led[] = {1, 1, 1, 1, 1, 1, 1, 0};
uint8_t led[] = {0, 0, 0, 0, 0, 0, 0, 0}; ///////////////////////////////////////

// 当前按键值
uint8_t key_code = 0;

// 上次按键状态标志。 1：未按下   0：按下
// uint8_t pre_key_code = 0;
uint8_t key_ENTER_prestate = 1;
uint8_t key_LEFT_prestate = 1;
uint8_t key_RIGHT_prestate = 1;
uint8_t key_INCREASE_prestate = 1;
uint8_t key_DECREASE_prestate = 1;
uint8_t key_prestate = 1;

// 将显示部分分为0~6共7个模块，存储各模块的首坐标
// 对应模块     0    1   2   3    4    5    6
// uint8_t x[] = {3,   3,  5,   5,   5,   5,   5};
// uint8_t y[] = {1, 11,  1, 11, 12, 13, 14};

// 只将模式A，"(个位数)1"，"(十分位数)1"设为字符串“变量”,下标0~2(原各模块内容中，共13个字符串)
// /*unsigned*/ char str[13][10] = {"工作模式" , "模式A" , "工作参数" ,  "1",   "."  ,  "1", "Hz" , "：" , "设置" , "返回" , "确定" , "取消" , "工作参数不合法"};
char str[3][10] = {"模式A", "1", "1"};
char initial_modeX[10];					 // 用于保存某次则进入act2画面时初始的“模式X”内容
char initial_unit[10], initial_tens[10]; // 用于保存某次则进入act3画面时初始的个位和十分位

// 系统时钟频率
uint32_t ui32SysClock;

//*****************************************************************************
//
// 主程序
//
//*****************************************************************************
int main(void)
{
	uint8_t temp, i;

	DevicesInit(); //  MCU器件初始化

	while (clock100ms < 3)
		; // 延时>60ms,等待TM1638上电完成

	TM1638_Init(); // 初始化TM1638
	initial_lcd(); // LCD初始化

	init_act0(); // 画面初始化为act0的画面

	while (1)
	{
		if (clock100ms_flag == 1) // 检查0.1秒定时是否到
		{
			clock100ms_flag = 0;
			// 每0.1秒累加计时值在数码管上以十进制显示，有键按下(key_code!=0)时暂停计时(即暂停更新test_counter和digit[0~3])
			if (key_code == 0)
			{
				if (++test_counter >= 10000)
					test_counter = 0;
				digit[0] = test_counter / 1000;		// 计算百位数
				digit[1] = test_counter / 100 % 10; // 计算十位数
				digit[2] = test_counter / 10 % 10;	// 计算个位数
				digit[3] = test_counter % 10;		// 计算十分位数
			}
		}

		/*if (clock500ms_flag == 1)   // 检查0.5秒定时是否到
		{
			clock500ms_flag = 0;
			// 8个指示灯以走马灯方式，每0.5秒向右（循环）移动一格(对应唯一不亮的那个灯向右(循环)移动一格)
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
	display_GB2312_string(7, 1, "设置", 0);
}

//*************************************************************
void init_act1(void)
{
	clear_screen();

	display_GB2312_string(3, 17, "工作模式", 0); //(3 - 1) * 8 + 1=17
	display_GB2312_string(5, 17, "工作参数", 0); //(3 - 1) * 8 + 1=17
	display_GB2312_string(7, 105, "返回", 0);	 //(13 - 1) * 8 + 1=97
}

//********************************************************************
void init_act2(void)
{
	clear_screen();

	strcpy(initial_modeX, str[0]); // 先保存初始的“模式X”内容

	display_GB2312_string(3, 1, "工作模式：", 0);
	display_GB2312_string(3, 81, str[0], 0); //(11 - 1) * 8 + 1=81
	display_GB2312_string(7, 1, "确定", 0);
	display_GB2312_string(7, 97, "取消", 0); //(13 - 1) * 8 + 1=97
}

//*****************************************************************
void init_act3(void)
{
	clear_screen();

	strcpy(initial_unit, str[1]);
	strcpy(initial_tens, str[2]); // 先保存初始的个位和十分位内容

	display_GB2312_string(3, 1, "工作参数：", 0);
	display_GB2312_string(3, 81, str[1], 0); //(11 - 1) * 8 + 1=81
	display_GB2312_string(3, 89, ".", 0);	 //(12 - 1) * 8 + 1=89
	display_GB2312_string(3, 97, str[2], 0); //(13 - 1) * 8 + 1=97
	display_GB2312_string(3, 105, "Hz", 0);	 //(14 - 1) * 8 + 1=105
	display_GB2312_string(7, 1, "确定", 0);
	display_GB2312_string(7, 97, "取消", 0); //(13 - 1) * 8 + 1=97
}

//***************************************************************************
void init_act4(void)
{
	clear_screen();

	display_GB2312_string(3, 1, "工作参数不合法", 0);
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
	// display_GB2312_string(3, 1, "工作参数不合法", 0);
	if (act4_clock5s_flag == 1) // 跳转到act3
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
		display_GB2312_string(7, 1, "设置", 1);
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
	else if (enter_flag) // 跳转到act1
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
		display_GB2312_string(7, 1, "设置", 0);
		ANY_flag = 0; // 最后一次按下的不算
		ui_state = 0;
	}
}


//********************************************************
void act1_ui_proc0(void)
{
	if (ANY_flag)
	{
		ANY_flag = 0;
		display_GB2312_string(3, 17, "工作模式", 1);//(3 - 1) * 8 + 1=17
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (3 - 1) * 8 + 1, "工作模式", 0);
		display_GB2312_string(5, (3 - 1) * 8 + 1, "工作参数", 1);
		ui_state = 2;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(3, (3 - 1) * 8 + 1, "工作模式", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "返回", 1);
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
	else if (enter_flag) // 跳转到act2
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
		display_GB2312_string(3, (3 - 1) * 8 + 1, "工作模式", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(5, (3 - 1) * 8 + 1, "工作参数", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "返回", 1);
		ui_state = 3;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(5, (3 - 1) * 8 + 1, "工作参数", 0);
		display_GB2312_string(3, (3 - 1) * 8 + 1, "工作模式", 1);
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
	else if (enter_flag) // 跳转到act3
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
		display_GB2312_string(5, (3 - 1) * 8 + 1, "工作参数", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "返回", 0);
		display_GB2312_string(3, (3 - 1) * 8 + 1, "工作模式", 1);
		ui_state = 1;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "返回", 0);
		display_GB2312_string(5, (3 - 1) * 8 + 1, "工作参数", 1);
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
	else if (enter_flag) // 跳转到act0
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
		display_GB2312_string(7, (13 - 1) * 8 + 1, "返回", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 0);
		display_GB2312_string(7, 1, "确定", 1);
		ui_state = 2;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 1);
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
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, 1, "确定", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 1);
		ui_state = 3;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, 1, "确定", 0);
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
	else if (enter_flag) // 跳转到act1，且模式X设置成功，需更新初始值
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
		display_GB2312_string(7, 1, "确定", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 0);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[0], 1);
		ui_state = 1;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 0);
		display_GB2312_string(7, 1, "确定", 1);
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
	else if (enter_flag) // 跳转到act1，且模式X设置失败，需复原为初始值
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
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
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
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 1);
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
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(3, (13 - 1) * 8 + 1, str[2], 0);
		display_GB2312_string(7, 1, "确定", 1);
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
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, 1, "确定", 0);
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 1);
		ui_state = 4;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, 1, "确定", 0);
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
	else if (enter_flag) // 跳转到act1或act4 !
	{
		uint8_t value_10times = (str[1][0] - '0') * 10 + (str[2][0] - '0');
		if (value_10times >= 7 && value_10times <= 77) // 若参数合法，则跳转到act1，且个位和十位设置成功，需更新初始值
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
		else // 若参数不合法，则跳转到act4，且个位和十位设置失败，需复原为初始值
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
		display_GB2312_string(7, 1, "确定", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
	} // 从状态0到状态1对标志位的影响不算
	if (right_flag)
	{
		right_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 0);
		display_GB2312_string(3, (11 - 1) * 8 + 1, str[1], 1);
		ui_state = 1;
	}
	else if (left_flag)
	{
		left_flag = 0;
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 0);
		display_GB2312_string(7, 1, "确定", 1);
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
	else if (enter_flag) // 跳转到act1，且个位和十分位设置失败，均需复原为初始值
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
		display_GB2312_string(7, (13 - 1) * 8 + 1, "取消", 0);
		ANY_flag = 0; // 最后一次按下的不算
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
void change_number(uint8_t w, /*unsigned*/ char *actstring) // w=1: increase; w=2: decrease; 数字增减
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
void change_Letter(uint8_t w, /*unsigned*/ char *actstring) // w=1: increase; w=2: decrease; 字母增减
{
	actstring += 4; // 含汉字的字符串中一个汉字占数组的两项 
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
	SysTickEnable();									// SysTick使能
	SysTickIntEnable();									// SysTick中断允许
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

	GPIOInit();		   // GPIO初始化
	SysTickInit();	   // 设置SysTick中断
	IntMasterEnable(); // 总中断允许
}

//*****************************************************************************
//
// 函数原型：void SysTick_Handler(void)
// 函数功能：SysTick中断服务程序，注意其每隔20ms自动触发一次
// 函数参数：无
// 函数返回值：无
//
//*****************************************************************************
void SysTick_Handler(void) // 定时周期为20ms
{
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

	// 2秒钟软件定时器计数
	/*if (++clock2s >= V_T2s)
	{
		clock2s_flag = 1;       // 当2秒到时，溢出标志置1
		clock2s = 0;
	}*/

	// 10秒钟软件定时器计数
	/*if (++clock10s >= V_T10s)
	{
		clock10s_flag = 1;     // 当10秒到时，溢出标志置1
		clock10s = 0;
	}*/

	// 刷新显示板上的全部数码管、小数点和LED指示灯
	TM1638_RefreshDIGIandLED(digit, pnt, led);

	// 检查当前键盘输入，0代表无键操作，1-9表示有对应按键
	// 键号显示在从左到右第2位数码管(下标5)上
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
		if (++clock10s >= V_T10s) // 10s定时器软件计数
		{
			flag_nokey_10s = 1; // 当10s到时，溢出标志置1
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

	/*if (pre_key_code != 6 && key_code==6){clock10s_flag = 0; right_flag =1;}                                      //更新标志key_RIGHT_flag
	else if ( pre_key_code != 4 && key_code==4 ){clock10s_flag = 0; left_flag =1;}                                //更新标志left_flag
	else if ( pre_key_code != 2 && key_code==2 ){clock10s_flag = 0; increase_flag = 1;}                      //更新标志increase_flag
	else if ( pre_key_code != 8 && key_code==8 ){clock10s_flag = 0; decrease_flag = 1;}                     //更新标志key_DECREASE_flag
	else if ( pre_key_code != 5 && key_code==5 ){clock10s_flag = 0; enter_flag =1;}                             //更新标志key_ENTER_flag
	else if ( pre_key_code != 1&&key_code==1 || pre_key_code != 3&&key_code==3 || pre_key_code != 7&&key_code==7 || pre_key_code != 9&&key_code==9 )clock10s_flag = 0;
	else if ( ++clock10s >= V_T10s ){flag_nokey_10s = 1; clock10s_flag = 1; clock10s = 0;}                 //更新标志flag_nokey_10s

	// 将当前按键值存入pre_key_code
	pre_key_code = key_code;*/
}
