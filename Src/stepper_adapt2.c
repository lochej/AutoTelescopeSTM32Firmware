/*
 * stepper_adapt.c
 *
 *  Created on: Dec 8, 2018
 *      Author: JLH
 */

#include "stepper_adapt2.h"
#include "Arduino_HAL.h"
#include "stepper_it.h"

struct Stepper_t stepperAh2,stepperDec2;

void Stepper_initSteppers()
{

	Stepper_init(&stepperAh2);
	Stepper_init(&stepperDec2);

	stepperAh2.pos = 0;
	stepperAh2.pos_min=0;
	stepperAh2.pos_max=12500;
	stepperAh2.interStep_delay_us = 1000 * 1000 / 500; //5 steps/sec
	//stepperAh2.degreesPerSteps = 360.0/24000.0;
	stepperAh2.degreesPerSteps = 172.5/12400.0; //172.5° pour 12400 steps
	stepperAh2.ref_angle = 93;
	stepperAh2.doStep = &doStepAh2;
	stepperAh2.readFCHard = &readFCAh2;
	stepperAh2.enable = &enableAh2;
	stepperAh2.disable = &disableAh2;
	stepperAh2.htim = &htim15;
	stepperAh2.timoc_channel= TIM_CHANNEL_1;
	stepperAh2.tim_active_channel= HAL_TIM_ACTIVE_CHANNEL_1;


	stepperAh2.disable();

	stepperDec2.pos = 0;
	stepperDec2.pos_min=0;
	stepperDec2.pos_max=10000;
	stepperDec2.interStep_delay_us = 1000 * 1000 / 500; //5 steps/sec
	stepperDec2.degreesPerSteps = 221.0/8000.0;
	stepperDec2.ref_angle=100.3;
	stepperDec2.doStep = &doStepDec2;
	stepperDec2.readFCHard = &readFCDec2;
	stepperDec2.enable = &enableDec2;
	stepperDec2.disable = &disableDec2;
	stepperDec2.htim = &htim15;
	stepperDec2.timoc_channel= TIM_CHANNEL_2;
	stepperDec2.tim_active_channel= HAL_TIM_ACTIVE_CHANNEL_2;


	stepperDec2.disable();

	HAL_TIM_Base_Start_IT(stepperAh2.htim);


}


void doStepAh2(struct Stepper_t * pstepper) {

	digitalWrite(DIR_AH_PIN, !pstepper->dir);
	digitalWrite(STEP_AH_PIN, HIGH);

	uint32_t ms = micros();

	pstepper->dir == 1 ? pstepper->pos++:pstepper->pos--;
	while (micros() - ms <= 1);

	digitalWrite(STEP_AH_PIN, LOW);
}

uint8_t readFCAh2() {
	return digitalRead(FC_AH_PIN);
}

void enableAh2()
{
	digitalWrite(ENABLE_AH_PIN,LOW);
}

void disableAh2()
{
	digitalWrite(ENABLE_AH_PIN,HIGH);
}


void doStepDec2(struct Stepper_t * pstepper) {

	digitalWrite(DIR_DEC_PIN, !pstepper->dir);
	digitalWrite(STEP_DEC_PIN, HIGH);

	uint32_t ms = micros();

	pstepper->dir == 1 ? pstepper->pos++:pstepper->pos--;
	while (micros() - ms <= 2);


	digitalWrite(STEP_DEC_PIN, LOW);

}

uint8_t readFCDec2() {
	return digitalRead(FC_DEC_PIN);
}

void enableDec2()
{
	digitalWrite(ENABLE_DEC_PIN,LOW);
}

void disableDec2()
{
	digitalWrite(ENABLE_DEC_PIN,HIGH);
}
