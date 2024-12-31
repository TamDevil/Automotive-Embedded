#include "stm32f10x.h"                  // Device header
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_spi.h"              // Keil::Device:StdPeriph Drivers:SPI



#define SPI_SCK_Pin 	GPIO_Pin_4
#define SPI_MISO_Pin 	GPIO_Pin_5
#define SPI_MOSI_Pin 	GPIO_Pin_6 
#define SPI_CS_Pin 		GPIO_Pin_7
#define SPI_GPIO GPIOA
#define SPI_RCC RCC_APB2Periph_GPIOA

void TIM_Config(){
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStruct.TIM_Prescaler = 7200 - 1;
	TIM_InitStruct.TIM_Period = 0xFFFF;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
	TIM_Cmd(TIM2, ENABLE);
}

void delay_ms(uint32_t time){
	TIM_SetCounter(TIM2, 0);
	while(TIM_GetCounter(TIM2) < time * 10){}
	}

void RCC_Config(){
	RCC_APB2PeriphClockCmd(SPI_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void GPIO_Config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = SPI_SCK_Pin | SPI_MOSI_Pin | SPI_CS_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = SPI_MISO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GPIO, &GPIO_InitStructure);
}


uint8_t SPI_Slave_Receive_Transmit(uint8_t data){
	uint8_t dataReceive = 0x00;	//0b0000 0000
	uint8_t temp = 0x00;
	int i;
	while(GPIO_ReadInputDataBit(SPI_GPIO, SPI_CS_Pin));
	for(i = 0; i < 8; i++){
			while(GPIO_ReadInputDataBit(SPI_GPIO, SPI_SCK_Pin) == Bit_RESET);
			if(data & 0x80){
			GPIO_WriteBit(SPI_GPIO, SPI_MISO_Pin, Bit_SET);
			delay_ms(1);
			}else{
			GPIO_WriteBit(SPI_GPIO, SPI_MISO_Pin, Bit_RESET);
			delay_ms(1);
			}
	
			temp = GPIO_ReadInputDataBit(SPI_GPIO, SPI_MOSI_Pin);
			dataReceive = (dataReceive << 1) | temp;
			data <<= 1;
			while(GPIO_ReadInputDataBit(SPI_GPIO, SPI_SCK_Pin) == Bit_SET);
		}
	while(!GPIO_ReadInputDataBit(SPI_GPIO, SPI_CS_Pin));
	return dataReceive;
}


uint8_t DataTrans[] = {10,13,19,23,15,32,90};//Data
uint8_t DataReceive;
int main(){
	RCC_Config();
	GPIO_Config();
	TIM_Config();
	while(1){
		int i;
		for(i = 0; i < 7; i++){
		DataReceive = SPI_Slave_Receive_Transmit(DataTrans[i]);
			delay_ms(1000);
		}
	}
}