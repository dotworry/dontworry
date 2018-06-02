#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "lcd.h"
#include "adc.h"


 //三重AD采样    
 //stmf407zet6
 //2018年6月3日03:16:14
 
int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);    //初始化延时函数
	uart_init(115200);	//初始化串口波特率为115200
 	LCD_Init();         //初始化LCD接口
	Adc_Init();         //初始化ADC
	delay_ms(100);
	while(1)
	{ 
	Get_Adc();
	}
}



