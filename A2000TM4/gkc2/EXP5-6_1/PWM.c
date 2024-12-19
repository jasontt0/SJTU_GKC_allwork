//*****************************************************************************
//
// PWM.c - API for PWM.
//
// Copyright��2020-2021,�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// 
// Author:	�Ϻ���ͨ��ѧ���ӹ���ϵʵ���ѧ����
// Version: 1.0.0.20210508 
// Date��2021-05-08
// History��
//
//*****************************************************************************

#include "PWM.h"
extern uint32_t g_ui32SysClock;    // ϵͳʱ��
//extern uint32_t ui32SysClock;    // ϵͳʱ��

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void PWMInit()
// �������ܣ���������PG0ʹ�ø��ù���M0PWM4
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void PWMInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // PWM0ʹ��   
    
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true); // ʹ��(����)PWM0_4�����
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);             //ʹ��PWM0ģ���2�ŷ�����(��Ϊ4��PWM��2�ŷ�����������)
    //PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, ui32SysClock / ui32Freq_Hz); // ����Freq_Hz����PWM����
   
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // ʹ��GPIOG
    GPIOPinConfigure(GPIO_PF3_M0PWM3);              // �������Ÿ���
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);    // ����ӳ��
    
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);   //����PWM������
    //PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //����ռ�ձ�Ϊ50%
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void PWMStart(uint32_t ui32Freq_Hz)
// �������ܣ�����Ƶ��Ϊui32Freq_Hz�ķ���(ռ�ձ�Ϊ50%��PWM)���������ΪM0PWM4(PG0)
//          �ú�����Ϊ�˷����û�û���źŷ�����ʱ���������źŶ���д�ġ�
// ����������ui32Freq_Hz ��Ҫ�����ķ�����Ƶ��
// ��������ֵ����
//
//*******************************************************************************************************
void PWMStart(uint32_t ui32Freq_Hz)
{

  PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //ʹ��PWM0ģ���2�ŷ�����(��Ϊ4��PWM��2�ŷ�����������)     
	//PWMGenDisable(PWM0_BASE, PWM_GEN_2);     //ʹ��PWM0ģ���2�ŷ�����(��Ϊ4��PWM��2�ŷ�����������)   
    
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, g_ui32SysClock / ui32Freq_Hz); // ����Freq_Hz����PWM����
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4,(PWMGenPeriodGet(PWM0_BASE, PWM_GEN_2)/ 2)); //����ռ�ձ�Ϊ50%
    
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);     //ʹ��PWM0ģ���2�ŷ�����(��Ϊ4��PWM��2�ŷ�����������)   
}

//*******************************************************************************************************
// 
// ����ԭ�ͣ�void PWMStop()
// �������ܣ�M0PWM4(PG0)ֹͣ����PWM�ź�
// ������������
// ��������ֵ����
//
//*******************************************************************************************************
void PWMStop()
{
    PWMGenDisable(PWM0_BASE, PWM_GEN_2);   // M0PWM4(PG0)ֹͣ����PWM�ź�

}
