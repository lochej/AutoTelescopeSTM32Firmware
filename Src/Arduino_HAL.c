/*
 * Arduino_HAL.c
 *
 *  Created on: Nov 19, 2018
 *      Author: JLH
 */

#include "Arduino_HAL.h"
#include "tim.h"

struct GPIO_structs
{
	GPIO_TypeDef * port;
	uint32_t pin;
};

struct GPIO_structs gpios[GPIO_NUMBER]=
{
		{B1_GPIO_Port,B1_Pin},
		{LD2_GPIO_Port,LD2_Pin},
		{STEP_AH_GPIO_Port,STEP_AH_Pin},
		{DIR_AH_GPIO_Port,DIR_AH_Pin},
		{ENABLE_AH_GPIO_Port,ENABLE_AH_Pin},
		{FC_AH_GPIO_Port,FC_AH_Pin},
		{STEP_DEC_GPIO_Port,STEP_DEC_Pin},
		{DIR_DEC_GPIO_Port,DIR_DEC_Pin},
		{ENABLE_DEC_GPIO_Port,ENABLE_DEC_Pin},
		{FC_DEC_GPIO_Port,FC_DEC_Pin}
};





int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


double mapd(double x, double in_min, double in_max, double out_min, double out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


unsigned long micros()
{
	static uint32_t micros=0;

	do
	{
		micros=__HAL_TIM_GetCounter(&htim2);
	}while(micros!= __HAL_TIM_GetCounter(&htim2));

	return micros;
}

//Inlined

uint32_t millis()
{
	return HAL_GetTick();
}


void yield()
{

}


void pinMode(int pin,int mode)
{

}

void digitalWrite(int pin, int level)
{
	HAL_GPIO_WritePin(gpios[pin].port, gpios[pin].pin,level ? GPIO_PIN_SET: GPIO_PIN_RESET);
}

int digitalRead(int pin)
{
	return HAL_GPIO_ReadPin(gpios[pin].port, gpios[pin].pin) ? HIGH : LOW;
}
