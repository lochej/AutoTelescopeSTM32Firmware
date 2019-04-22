/*
 * Arduino_HAL.h
 *
 *  Created on: Nov 19, 2018
 *      Author: JLH
 */

#ifndef ARDUINO_HAL_H_
#define ARDUINO_HAL_H_





#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <math.h>



enum GPIO_MAP
{
	B1_PIN,
	LD2_PIN,
	STEP_AH_PIN,
	DIR_AH_PIN,
	ENABLE_AH_PIN,
	FC_AH_PIN,
	STEP_DEC_PIN,
	DIR_DEC_PIN,
	ENABLE_DEC_PIN,
	FC_DEC_PIN,
	GPIO_NUMBER
};



#define max(a,b) ( (a) > (b) ? (a) : (b))

#define HIGH (1)
#define LOW (0)
#define OUTPUT (0)
#define INPUT (1)


int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

double mapd(double x, double in_min, double in_max, double out_min, double out_max);



uint32_t micros();


uint32_t millis();


void yield();


void pinMode(int pin,int mode);

void digitalWrite(int pin, int level);

int digitalRead(int pin);


#ifdef __cplusplus
}
#endif


#endif /* ARDUINO_HAL_H_ */
