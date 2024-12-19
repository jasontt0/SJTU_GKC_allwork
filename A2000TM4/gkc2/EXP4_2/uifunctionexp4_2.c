//单界面菜单示例，反白表示游标位置
//可由底板3*3按键控制
//   ⑴   ⑵   ⑶
//   ⑷   ⑸   ⑹
//   ⑺   ⑻   ⑼
// ⑸表示enter
// ⑷⑹控制光标位置
//
//  ————————————
//  |                      |
//  |    工作模式: 模式A    |
//  |    工作参数: 1.1Hz    |
//  |                      |
//  ————————————

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h" 
#include "inc/hw_types.h" 
#include "driverlib/pin_map.h"  
#include "driverlib/sysctl.h"	
#include "JLX12864G.h"

#define KEYTMR_OF  10
extern uint8_t key_code;
extern uint8_t NOKEY_clock10s_flag;

unsigned int opmode=0;
int opmode_counter=0;

unsigned char stringopmode_A[]="模式A";
unsigned char stringopmode_B[]="模式B";
unsigned char stringopmode_C[]="模式C";

struct struct_act
{
	unsigned char num;
	unsigned char *str[20];
	unsigned char x[20],y[20],inverse[20];
} a0;
struct struct_act *act[1];

unsigned char a0_s0[]="工作模式:";
unsigned char *a0_s1;


unsigned int ui_state=0;  //状态号

unsigned int key_ENTER_state=0;
unsigned int key_ENTER_prestate=0;
unsigned int ENTER_key_timer=0;
unsigned int key_ENTER_flag=0; 

unsigned int key_DOWN_state=0;
unsigned int key_DOWN_prestate=0;
unsigned int key_DOWN_timer=0;
unsigned int key_DOWN_flag=0; 

unsigned int key_UP_state=0;
unsigned int key_UP_prestate=0;
unsigned int key_UP_timer=0;
unsigned int key_UP_flag=0; 

unsigned int key_INCREASE_state=0;
unsigned int key_INCREASE_prestate=0;
unsigned int key_INCREASE_timer=0;
unsigned int key_INCREASE_flag=0; 

unsigned int key_DECREASE_state=0;
unsigned int key_DECREASE_prestate=0;
unsigned int key_DECREASE_timer=0;
unsigned int key_DECREASE_flag=0; 


void in_de_opmode(unsigned int opmode)//opmode工作模式增减
{
		switch (opmode_counter)
		{
			case 0: act[0]->str[1]= stringopmode_A;break;
			case 1: act[0]->str[1]= stringopmode_B;break;
			case 2: act[0]->str[1]= stringopmode_C;break;						
			default: break;	 
		} 	 
}

void itoaopmode (int opmode, unsigned char **instropmode)//将后台opmode工作模式转换为lcd屏幕数据
{
	switch (opmode)
	{
		case 0: *instropmode = stringopmode_A;break;
		case 1: *instropmode = stringopmode_B;break;
		case 2: *instropmode = stringopmode_C;break;
		default: break;
	}
}



void ENTER_detect(void)
{
	switch(key_ENTER_state)
	{
		case 0:
			if(key_code==5)
			{key_ENTER_state=1; key_ENTER_flag=1;} break;
		case 1:
			if (key_code!=5)
			{key_ENTER_state=0;} break;
		default: {key_ENTER_state=0;} break;
			
	}
}

void DOWN_detect(void)
{
	if (key_code==6) ///////////////////	 DOWN
	{
		key_DOWN_prestate=key_DOWN_state;		
		key_DOWN_state=0;
		if (key_DOWN_prestate==1) key_DOWN_flag=1;
		
	}
	else
	{
		key_DOWN_prestate = key_DOWN_state; 
		key_DOWN_state=1;	
	}

}


void UP_detect(void)
{
	switch(key_UP_state)
	{
		case 0:
			if(key_code==4)
			{key_UP_state=1; key_UP_flag=1;} break;
		case 1:
			if (key_code!=4)
			{key_UP_state=0;} break;
		default: {key_UP_state=0;} break;
			
	}
}


void INCREASE_detect(void)
{
	if (key_code==2) ///////////////////	 INCREASE	
	{
		key_INCREASE_prestate=key_INCREASE_state;		
		key_INCREASE_state=0;
		if (key_INCREASE_prestate==1) 
		{	key_INCREASE_flag=1;	key_INCREASE_timer =0;	}
		else if (key_INCREASE_prestate==0)
		{
			if 	(++key_INCREASE_timer>=KEYTMR_OF)
			{ key_INCREASE_flag=1; key_INCREASE_timer=0;}  
		}
	}
	else
	{
		key_INCREASE_prestate = key_INCREASE_state; 
		key_INCREASE_state=1;
		key_INCREASE_timer=0;	
	}
}

void DECREASE_detect(void)
{
	if (key_code==8) ///////////////////	 DECREASE	
	{
		key_DECREASE_prestate=key_DECREASE_state;		
		key_DECREASE_state=0;
		if (key_DECREASE_prestate==1) 
		{	key_DECREASE_flag=1;	key_DECREASE_timer =0;	}
		else if (key_DECREASE_prestate==0)
		{
			if 	(++key_DECREASE_timer>=KEYTMR_OF)
			{ key_DECREASE_flag=1; key_DECREASE_timer=0;}  
		}
	}
	else
	{
		key_DECREASE_prestate = key_DECREASE_state; 
		key_DECREASE_state=1;
		key_DECREASE_timer=0;	
	}
}


void display_ui_act(unsigned int i)
{		
	unsigned int j=0;
	clear_screen();

	for (j=0;j<act[i]->num;j++) 
	{
		display_GB2312_string(act[i]->x[j],(act[i]->y[j]-1)*8+1,act[i]->str[j],act[i]->inverse[j]);		
	}
}



void initial_act(void)
{
	itoaopmode(opmode_counter,&a0_s1);

	a0.num=2;
	a0.str[0]=a0_s0; a0.x[0]=3;  a0.y[0]=1;  a0.inverse[0]=0; 
	a0.str[1]=a0_s1; a0.x[1]=3;  a0.y[1]=11;  a0.inverse[1]=0;		///////act0
	act[0]=&a0;

	display_ui_act(0);
}

void ui_proc0(void)
{

	if(key_code!=0)
	{
		key_UP_flag=0;key_DOWN_flag=0;key_INCREASE_flag=0;key_DECREASE_flag=0;key_ENTER_flag=0;
		act[0]->inverse[1]=1; display_GB2312_string(act[0]->x[1],(act[0]->y[1]-1)*8+1,act[0]->str[1],act[0]->inverse[1]);
		ui_state=001;
	}

}

void ui_proc001(void)
{
	if(key_UP_flag)
	{
		key_UP_flag=0;
	}
	else if (key_DOWN_flag)
	{
		key_DOWN_flag=0;
	}
	else if (key_INCREASE_flag)
	{
		key_INCREASE_flag=0;
		opmode_counter++;
		if (opmode_counter>2) opmode_counter=0;
		in_de_opmode(opmode_counter);
		display_GB2312_string(act[0]->x[1],(act[0]->y[1]-1)*8+1,act[0]->str[1],act[0]->inverse[1]);
		ui_state=001;
	}
		else if (key_DECREASE_flag)
	{
		key_DECREASE_flag=0;
		opmode_counter--;
		if (opmode_counter<0) opmode_counter=2;
		in_de_opmode(opmode_counter);
		display_GB2312_string(act[0]->x[1],(act[0]->y[1]-1)*8+1,act[0]->str[1],act[0]->inverse[1]);
		ui_state=001;
	}

	else if(key_ENTER_flag)
	{
		key_ENTER_flag=0;
	}
	if(NOKEY_clock10s_flag==1)
	{
		NOKEY_clock10s_flag = 0;
		act[0]->inverse[1]=0; display_GB2312_string(act[0]->x[1],(act[0]->y[1]-1)*8+1,act[0]->str[1],act[0]->inverse[1]);
		ui_state=0;		
	}

}

void ui_state_proc(unsigned int ui_state)
{
	switch (ui_state)
		{
			case 0: ui_proc0(); break;
			case 001: ui_proc001(); break;

			default: break;
		}

}
