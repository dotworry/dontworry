#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "adc.h"


 //����AD����    
 //stmf407zet6
 //2018��6��3��03:16:14
 
int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);    //��ʼ����ʱ����
	uart_init(115200);	//��ʼ�����ڲ�����Ϊ115200
 	LCD_Init();         //��ʼ��LCD�ӿ�
	Adc_Init();         //��ʼ��ADC
	delay_ms(100);
	while(1)
	{ 
	Get_Adc();
	}
}



