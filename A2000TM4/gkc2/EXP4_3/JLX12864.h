#ifndef __JXL12864_H__
#define __JXL12864_H__

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // 基址宏定义
#include "inc/hw_types.h"         // 数据类型宏定义，寄存器访问函数
#include "driverlib/debug.h"      // 调试用
#include "driverlib/gpio.h"       // 通用IO口宏定义
#include "driverlib/pin_map.h"    // TM4C系列MCU外围设备管脚宏定义
#include "driverlib/sysctl.h"	  // 系统控制定义
#include "driverlib/systick.h"    // SysTick Driver 原型
#include "driverlib/interrupt.h"  // NVIC Interrupt Controller Driver 原型

#include "tm1638.h"               // 与控制TM1638芯片有关的函数
#define LCD_CS_H GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4)        //LCD_CS <--> PB4
#define LCD_CS_L GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4,0)
#define LCD_SCK_H GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_5,GPIO_PIN_5)        //LCD_SCK <--> PB5
#define LCD_SCK_L GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5,0)
#define LCD_SDA_H GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4)     //LCD_SDA <--> PE4
#define LCD_SDA_L GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4,0)
#define LCD_RST_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0)      //LCD_RST <--> PKO
#define LCD_RST_L GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_0,0)
#define LCD_RS_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, GPIO_PIN_1)        //LCD_RS <--> PK1
#define LCD_RS_L GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_1,0)

#define LCD_ROM_IN_H GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, GPIO_PIN_2)  //ROM_IN <--> PK2
#define LCD_ROM_IN_L GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2,0)
#define LCD_ROM_SCK_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5,GPIO_PIN_5) //ROM_SCK <--> PC5
#define LCD_ROM_SCK_L GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,0)
#define LCD_ROM_CS_H GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4)  //ROM_CS <--> PC4
#define LCD_ROM_CS_L GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,0)

#define LCD_ROM_OUT GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_6)                         //ROM_OUT <--> PC6

#define TEST_H GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1)                  //LCD_delay时间测试管脚PF1
#define TEST_L GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,0)

extern uint8_t  bmp1[];
extern uint8_t  jiong1[];
extern uint8_t  lei1[];

extern void LCD_PORT_init(void);
extern void LCD_delay(int n_ms);
extern void delay_us(int n_us);
extern void transfer_command_lcd(int data1);
extern void transfer_data_lcd(int data1);
extern void initial_lcd(void);
extern void lcd_address(uint32_t page,uint32_t column);
extern void clear_screen(void);
extern void display_128x64(uint8_t *dp);
extern void display_graphic_16x16(uint8_t page,uint8_t column,uint8_t *dp);
extern void display_graphic_8x16(uint8_t page,uint8_t column,uint8_t *dp);
extern void display_graphic_5x8(uint8_t page,uint8_t column,uint8_t *dp);
extern void send_command_to_ROM( uint8_t datu );
//static uint8_t get_data_from_ROM( );
extern void get_and_write_16x16(unsigned long fontaddr,uint8_t page,uint8_t column,uint8_t inverse);
extern void get_and_write_8x16(unsigned long fontaddr,uint8_t page,uint8_t column,uint8_t inverse);
extern void get_and_write_5x8(unsigned long fontaddr,uint8_t page,uint8_t column,uint8_t inverse);
extern void display_GB2312_string(uint8_t page,uint8_t column,unsigned char *text,uint8_t inverse);
extern void display_string_5x8(uint8_t page,uint8_t column,unsigned char *text,uint8_t inverse);


#endif
