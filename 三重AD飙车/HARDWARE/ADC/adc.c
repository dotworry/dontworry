#include "adc.h"
#include "delay.h"		 
#define RHEOSTAT_ADC_CDR_ADDR    ((uint32_t)0x40012308) // ADC���ݴ洢��ַ

//__IO uint32_t = volatile uint32_t   
// volatile �������txt�ĵ�
__IO uint32_t ADC_ConvertedValue[3];								  	// ADC1ת���ĵ�ѹֵͨ��DMA��ʽ����SRAM
int i=0;         																			  // iΪѡ��ADx������
float AD_value;																					// ת���õ�adֵ

//��ʼ��ADC����������DMA��ʼ����														   
void  Adc_Init(void)
{    
  GPIO_InitTypeDef  			GPIO_InitStructure;
	DMA_InitTypeDef 				DMA_InitStructure;
	ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
	ADC_InitTypeDef       	ADC_InitStructure;
	
//----------------------------------------ADC ʹ�����ų�ʼ��---------------------------------------------
	//�ȳ�ʼ��ADC1ͨ��2 IO��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;						 //PA2 ͨ��2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;				 //ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;		 //����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);							 //��ʼ��  
 
//----------------------------------------DMA ��ʼ��-----------------------------------------------------
	// ADC1ʹ��DMA2��������0��ͨ��0 *****�ϵ͵�ͨ��ӵ�бȽϸ�ͨ�����ߵ����ȼ�******
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);  						// ����DMA2ʱ�� 
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = RHEOSTAT_ADC_CDR_ADDR;	// ADC���ݴ洢��ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)ADC_ConvertedValue;  // �洢����ַ��ʵ���Ͼ���һ���ڲ�SRAM�ı���	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	 				  // ���ݴ��䷽��Ϊ���赽�洢��	
	DMA_InitStructure.DMA_BufferSize = 3;															// ��������СΪ��ָһ�δ����������
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	// ����Ĵ���ֻ��һ������ַ���õ���  ���ж��������������Ҫ����ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					  // �ڴ����ģʽ �洢����ַ�̶�
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // ���ݿ��Ϊ32λ �������ݴ�СΪ���֣��������ֽ� 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;		// ���ݿ��Ϊ32λ �ڴ����ݴ�СΪ���֣��������ֽ� 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  									// ѭ������ģʽ	����ʹ��DMA����AD���� ��ֻ������ֻ��Ҫ����ģʽ ����ָ����������� �����ʹ��ͨ��ģʽDMA_Mode_Normal					
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; 							// DMA ����ͨ�����ȼ�Ϊ�ߣ���Ϊ4��VeryHigh,High,Medium,Low
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  					// ��ֹDMA FIFO	��ʹ��ֱ��ģʽ 
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; // FIFO ��С��FIFOģʽ��ֹʱ�������������	
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				// �ڵ�ַ����ģʽ�²�����(������DMA)����һ�����жϴ����������
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//ͬ�ϣ�Ϊ��Χ�����ͻ����������  
  DMA_InitStructure.DMA_Channel = DMA_Channel_0; 										// ѡ�� DMA ͨ��0 ��ͨ������������
  DMA_Init(DMA2_Stream0, &DMA_InitStructure); 											// ��ʼ�� DMA��0 �����൱��һ����Ĺܵ����ܵ������кܶ�ͨ��
  DMA_Cmd(DMA2_Stream0, ENABLE);																		// ʹ�� DMA��0

//----------------------------------------ADC ��ʼ��----------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	 											 // ʹ��ADC1ʱ�� 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);											   // ʹ��ADC2ʱ�� 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);	 											 // ʹ��ADC2ʱ�� 
 
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;										 // ����ģʽ ��ģʽ��ÿ��adc��������
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;// ���������׶�֮����ӳ�5��ʱ�� �ֲ���дΪ���17��ʱ�ӣ�����Ӧ�ÿɸģ�
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_2; 						 // DMA ģʽ2�����ڽ���ģʽ�͹���ͬʱģʽ(���ʺϹ���˫��ADCģʽ)���Ĳο��ֲ�p261
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;									 // Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);																		 // ��ʼ��
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;											 // ADC�ֱ��� 12λ
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;																 // ��ɨ��ģʽ ��ͨ��ģʽ����Ҫɨ��ģʽ	
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;													 // ��������ת��
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;	 // ��ֹ������⣬ʹ���������
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;											 // �����Ҷ���	
  ADC_InitStructure.ADC_NbrOfConversion = 1;																	 // 1��ת��ͨ��
 
  ADC_Init(ADC1, &ADC_InitStructure);																					 // ADC1��ʼ��
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);	 // ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ3��ʱ������
	ADC_Init(ADC2, &ADC_InitStructure);																					 // ADC2��ʼ��
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);	 // ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ3��ʱ������
	ADC_Init(ADC3, &ADC_InitStructure);																					 // ADC3��ʼ��
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);	 // ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ3��ʱ������

	ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_Cmd(ADC1, ENABLE);				// ����ADC1ת����	
	ADC_Cmd(ADC2, ENABLE);				// ����ADC2ת����	
	ADC_Cmd(ADC3, ENABLE);				// ����ADC3ת����	
 
	ADC_SoftwareStartConv(ADC1);  // ����adcת�����������ת����ʼ
//	ADC_ContinuousModeCmd(ADC1, ENABLE); //�˺������ڿ���ADC����ģʽ�µ���ͣ (δ����)
}				  

//-----------------------------------��ȡADת�����ֵ-----------------------------------
u16 Get_Adc(void)   
{
	AD_value = (float)((uint16_t)ADC_ConvertedValue[i]*3.3/4096); 
	if(i==2)
		i=0;
	else
		i++;
	return AD_value;
}

	 








