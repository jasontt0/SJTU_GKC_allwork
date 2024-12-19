//**************************************************************************************
//
// Copyright: 2020-2021, �Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// File name: exp0_debug.c
// Description: LED4(D4-PF0)��Լ��6000����Ϊ���ڻ�����˸��
//              ������PUSH1(USR_SW1-PJ0)����LED4(D4-PF0)��Լ��100����Ϊ���ڿ�����˸��
//              �ɿ�PUSH1(USR_SW1-PJ0)����LED4(D4-PF0)�ָ���500����Ϊ���ڻ�����˸��
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20201228
// Date��2020-12-28
// History��
//
//**************************************************************************************

#define DEBUG_CONSOLE

//**************************************************************************************
//
// ͷ�ļ�
//
//**************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"        // ��ַ�궨��
#include "inc/hw_types.h"         // �������ͺ궨�壬�Ĵ������ʺ���
#include "driverlib/debug.h"      // ������
#include "driverlib/gpio.h"       // ͨ��IO�ں궨��
#include "driverlib/pin_map.h"    // TM4Cϵ��MCU��Χ�豸�ܽź궨��
#include "driverlib/sysctl.h"     // ϵͳ���ƺ궨��


#include "driverlib/uart.h"       // UART��غ궨��
#include "utils/uartstdio.h"      // UART0��Ϊ����̨��غ���ԭ������
// uartstdio.h��uartstdio.c������utilsĿ¼��
//**************************************************************************************
//
// �궨��
//
//**************************************************************************************
#define  MilliSecond      4000    // �γ�1msʱ������ѭ������ 
#define  FASTFLASHTIME    150	  // ����ʱ��100ms��
#define  SLOWFLASHTIME    6000     // ����ʱ��6s��

//**************************************************************************************
//
// ����ԭ������
//
//**************************************************************************************
void  DelayMilliSec(uint32_t ui32DelaySecond);		// �ӳ�һ��ʱ������λΪ����
void  GPIOInit(void);                               // GPIO��ʼ��
void  PN_Switch(uint8_t ui8KeyValue_0, uint8_t ui8KeyValue_1);      // ���ݴ���İ���ֵ������PF0����������
void  InitConsole(void);      // UART0��ʼ��


uint32_t g_ui32SysClock;
//**************************************************************************************
//
// ������
//
//**************************************************************************************
int main(void)
{
  uint8_t ui8KeyValue_0, ui8KeyValue_1;

//**************************************************************************************
//�ڲ�ʱ��ԴPIOSC(16MHz)����ϵͳʱ��ԴOSCֱ��ʹ�ã���ƵΪ16/12/8 MHz
//	g_ui32SysClock = SysCtlClockFreqSet(SYSCTL_OSC_INT|SYSCTL_USE_OSC,12000000);


//�ⲿʱ��ԴMOSC(25MHz)����ϵͳʱ��ԴOSCֱ��ʹ�ã���ƵΪ25/12/1 MHz
//	g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_OSC | SYSCTL_CFG_VCO_480), 1000000);


//�ⲿʱ��ԴMOSC(25MHz)����ϵͳʱ��ԴPLL��Ƶ��480MHz����ƵΪ25/20/8 MHz
  g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 25000000);


//�ڲ�ʱ��ԴPIOSC(16MHz)����ϵͳʱ��ԴPLL��Ƶ��480MHz����ƵΪ20/8/1 MHz
//    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_INT|SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),1000000);
//**************************************************************************************


  GPIOInit();             // GPIO��ʼ��

#ifdef DEBUG_CONSOLE
  InitConsole();          // UART0��ʼ��
  UARTprintf("Hello Everyone\n");
  UARTprintf("System Clock = %d Hz\n", g_ui32SysClock);
#endif

  while(1)// ����ѭ��
    {

      // ��ȡ PJ0 ��ֵ  0-���� 1-�ɿ�
      ui8KeyValue_0 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0);
      ui8KeyValue_1 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1);

#ifdef DEBUG_CONSOLE
      UARTprintf("PJ0= %d\t%d\n", ui8KeyValue_0,ui8KeyValue_1);
#endif

      PN_Switch(ui8KeyValue_0, ui8KeyValue_1);          // ���ݴ���İ�������������PF0����������
    }
}


//**************************************************************************************
//
// ����ԭ�ͣ�void DelayMilliSec(uint32_t ui32DelaySecond)
// �������ܣ��ӳ�һ��ʱ������λΪ����
// ����������ui32DelaySecond���ӳٺ�����

void DelayMilliSec(uint32_t ui32DelaySecond)
{
  uint32_t ui32Loop;

  ui32DelaySecond = ui32DelaySecond * MilliSecond;
  for(ui32Loop = 0; ui32Loop < ui32DelaySecond; ui32Loop++) { };
}
//**************************************************************************************




//**************************************************************************************
//
// ����ԭ�ͣ�void GPIOInit(void)
// �������ܣ�GPIO��ʼ����ʹ��PortF������PF0Ϊ�����ʹ��PortJ������PJ0Ϊ����
// ������������
//
//**************************************************************************************
void GPIOInit(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);		   // ʹ�ܶ˿� F
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));	   // �ȴ��˿� F׼�����

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);		   // ʹ�ܶ˿� J
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}; // �ȴ��˿� J׼�����

  // ���ö˿� N �ĵ�0λ��PF0��Ϊ�������
//    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

//    // ���ö˿� N �ĵ�4λ��PF4��Ϊ�������
//    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

//    // ���ö˿� N�ĵ�0��4λ��PF0��PF4��Ϊ�������
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0|GPIO_PIN_1);

  // ���ö˿� J�ĵ�0λ��PJ0��Ϊ��������
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);

  // �˿� J�ĵ�0λ��Ϊ�������룬�������óɡ�����������
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  // �˿� J�ĵ�1λ��Ϊ�������룬�������óɡ�����������
  GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

}

//**************************************************************************************
//
// ����ԭ�ͣ�void PF0Flash(uint8_t ui8KeyValue)
// �������ܣ����ݴ���İ���ֵ������PF0������������0-������1-����
// ����������ui8KeyValue������ֵ
//
//**************************************************************************************
void PN_Switch(uint8_t ui8KeyValue_0, uint8_t ui8KeyValue_1)
{
  uint32_t ui32DelayTime;

  if (ui8KeyValue_0	== 0) GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); //  PUSH1(USR_SW1-PJ0) ����,���� D4-PN0
  else GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00); // �ر� D4-PN0

  if (ui8KeyValue_1	== 0) GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1); //  PUSH1(USR_SW1-PJ0) ����,���� D4-PN1
  else GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00);        // �ر� D4-PN1



  /*
  	//DelayMilliSec(ui32DelayTime);                          // ��ʱui32DelayTime����

      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);        // �ر� LED4(D4-PF0)

  //    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00);        // �ر� LED3(D3-PF4)
  //    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, 0x00); // �ر� LED4(D4-PF0),LED3(D3-PF4)
  //    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_4); //


      DelayMilliSec(ui32DelayTime);                          // ��ʱui32DelayTime����
  */

}


//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void InitConsole(void)
{
  //
  // Enable GPIO port A which is used for UART0 pins.
  // TODO: change this to whichever GPIO port you are using.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

  //
  // Configure the pin muxing for UART0 functions on port A0 and A1.
  // This step is not necessary if your part does not support pin muxing.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);

  //
  // Enable UART0 so that we can configure the clock.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

  //
  // Select the alternate (UART) function for these pins.
  // TODO: change this to select the port/pin you are using.
  //
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  //
  // Initialize the UART for console I/O.
  //
  UARTStdioConfig(0, 115200, g_ui32SysClock);

}
