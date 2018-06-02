#include "adc.h"
#include "delay.h"		 
#define RHEOSTAT_ADC_CDR_ADDR    ((uint32_t)0x40012308) // ADC数据存储地址

//__IO uint32_t = volatile uint32_t   
// volatile 性质详见txt文档
__IO uint32_t ADC_ConvertedValue[3];								  	// ADC1转换的电压值通过DMA方式传到SRAM
int i=0;         																			  // i为选择ADx的数据
float AD_value;																					// 转换好的ad值

//初始化ADC函数（包括DMA初始化）														   
void  Adc_Init(void)
{    
  GPIO_InitTypeDef  			GPIO_InitStructure;
	DMA_InitTypeDef 				DMA_InitStructure;
	ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
	ADC_InitTypeDef       	ADC_InitStructure;
	
//----------------------------------------ADC 使用引脚初始化---------------------------------------------
	//先初始化ADC1通道2 IO口
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;						 //PA2 通道2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;				 //模拟输入
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;		 //不带上下拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);							 //初始化  
 
//----------------------------------------DMA 初始化-----------------------------------------------------
	// ADC1使用DMA2，数据流0，通道0 *****较低的通道拥有比较高通道更高的优先级******
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);  						// 开启DMA2时钟 
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = RHEOSTAT_ADC_CDR_ADDR;	// ADC数据存储地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)ADC_ConvertedValue;  // 存储器地址，实际上就是一个内部SRAM的变量	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	 				  // 数据传输方向为外设到存储器	
	DMA_InitStructure.DMA_BufferSize = 3;															// 缓冲区大小为，指一次传输的数据量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	// 外设寄存器只有一个，地址不用递增  若有多个外设连接则需要递增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					  // 内存递增模式 存储器地址固定
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // 数据宽度为32位 外设数据大小为半字，即两个字节 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;		// 数据宽度为32位 内存数据大小为半字，即两个字节 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  									// 循环传输模式	不断使用DMA传输AD数据 若只传输则只需要单词模式 若按指令操作来访问 则可以使用通用模式DMA_Mode_Normal					
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; 							// DMA 传输通道优先级为高，分为4级VeryHigh,High,Medium,Low
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  					// 禁止DMA FIFO	，使用直连模式 
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; // FIFO 大小，FIFO模式禁止时，这个不用配置	
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				// 在地址增量模式下才有用(多外设DMA)设置一个非中断传输的数据量
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//同上，为外围传输的突发传输配置  
  DMA_InitStructure.DMA_Channel = DMA_Channel_0; 										// 选择 DMA 通道0 ，通道存在于流中
  DMA_Init(DMA2_Stream0, &DMA_InitStructure); 											// 初始化 DMA流0 ，流相当于一个大的管道，管道里面有很多通道
  DMA_Cmd(DMA2_Stream0, ENABLE);																		// 使能 DMA流0

//----------------------------------------ADC 初始化----------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	 											 // 使能ADC1时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);											   // 使能ADC2时钟 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);	 											 // 使能ADC2时钟 
 
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;										 // 独立模式 此模式下每个adc独立工作
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;// 两个采样阶段之间的延迟5个时钟 手册上写为最低17个时钟（此项应该可改）
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2; 						 // DMA 模式2适用于交替模式和规则同时模式(仅适合规则双重ADC模式)中文参考手册p261
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;									 // 预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);																		 // 初始化
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;											 // ADC分辨率 12位
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;																 // 非扫描模式 多通道模式下需要扫描模式	
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;													 // 开启连续转换
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;	 // 禁止触发检测，使用软件触发
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;											 // 数据右对齐	
  ADC_InitStructure.ADC_NbrOfConversion = 1;																	 // 1个转换通道
 
  ADC_Init(ADC1, &ADC_InitStructure);																					 // ADC1初始化
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);	 // 配置 ADC 通道转换顺序为1，第一个转换，采样时间为3个时钟周期
	ADC_Init(ADC2, &ADC_InitStructure);																					 // ADC2初始化
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);	 // 配置 ADC 通道转换顺序为1，第一个转换，采样时间为3个时钟周期
	ADC_Init(ADC3, &ADC_InitStructure);																					 // ADC3初始化
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);	 // 配置 ADC 通道转换顺序为1，第一个转换，采样时间为3个时钟周期

	ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_Cmd(ADC1, ENABLE);				// 开启ADC1转换器	
	ADC_Cmd(ADC2, ENABLE);				// 开启ADC2转换器	
	ADC_Cmd(ADC3, ENABLE);				// 开启ADC3转换器	
 
	ADC_SoftwareStartConv(ADC1);  // 开启adc转换，软件触发转换开始
//	ADC_ContinuousModeCmd(ADC1, ENABLE); //此函数用于控制ADC连续模式下的启停 (未测试)
}				  

//-----------------------------------获取AD转换后的值-----------------------------------
u16 Get_Adc(void)   
{
	AD_value = (float)((uint16_t)ADC_ConvertedValue[i]*3.3/4096); 
	if(i==2)
		i=0;
	else
		i++;
	return AD_value;
}

	 








