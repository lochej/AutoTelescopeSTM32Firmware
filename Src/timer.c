/*
 * timer.c
 *
 *  Created on: 26 mars 2019
 *      Author: JLH
 */
#include "includes.h"

volatile uint32_t *DWT_CYCCNT = (uint32_t *) 0xE0001004; //address of the register
volatile uint32_t *DWT_CONTROL = (uint32_t *) 0xE0001000; //address of the register
volatile uint32_t *SCB_DEMCR = (uint32_t *) 0xE000EDFC; //address of the register



void TimerCycInit()
{
	*SCB_DEMCR = *SCB_DEMCR | 0x01000000;
	*DWT_CONTROL = *DWT_CONTROL | 1; // enable the counter
}


uint32_t TimerCycCount()
{
	return *DWT_CYCCNT;
}
