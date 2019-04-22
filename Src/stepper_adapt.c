/*
 * stepper_adapt.c
 *
 *  Created on: Dec 8, 2018
 *      Author: JLH
 */

#include "stepper_adapt.h"


struct StepperTelescope stepperAh,stepperDec;

void StepperTelescope_initSteppers()
{

	StepperTelescope_init(&stepperAh);

	stepperAh.pos = 0;
	stepperAh.pos_min=0;
	stepperAh.pos_max=12000;
	stepperAh.degreesPerSteps = 360.0/24000.0;
	stepperAh.interStep_delay_us = 1000 * 1000 / 500; //5 steps/sec
	stepperAh.doStep = &doStepAh;
	stepperAh.readFCHard = &readFCAh;
	stepperAh.enable = &enableAh;
	stepperAh.disable = &disableAh;

	stepperAh.disable();

	StepperTelescope_switchMode(&stepperAh, CALIBRATION, 1);

	StepperTelescope_init(&stepperDec);

	stepperDec.pos = 0;
	stepperDec.pos_min=0;
	stepperDec.pos_max=20000;
	stepperDec.degreesPerSteps = mapd(1.0,0.0,24000.0,0.0,360.0);
	stepperDec.interStep_delay_us = 1000 * 1000 / 200; //5 steps/sec
	stepperDec.doStep = &doStepDec;
	stepperDec.readFCHard = &readFCDec;
	stepperDec.enable = &enableDec;
	stepperDec.disable = &disableDec;

	stepperDec.disable();

	StepperTelescope_switchMode(&stepperDec, CALIBRATION, 1);

}


void doStepAh(struct StepperTelescope * pstepper) {

	digitalWrite(DIR_AH_PIN, !pstepper->dir);
	digitalWrite(STEP_AH_PIN, HIGH);

	uint32_t ms = micros();

	pstepper->dir == 1 ? pstepper->pos++:pstepper->pos--;
	while (micros() - ms <= 1);

	digitalWrite(STEP_AH_PIN, LOW);
}

uint8_t readFCAh() {
	return digitalRead(FC_AH_PIN);
}

void enableAh()
{
	digitalWrite(ENABLE_AH_PIN,LOW);
}

void disableAh()
{
	digitalWrite(ENABLE_AH_PIN,HIGH);
}


void doStepDec(struct StepperTelescope * pstepper) {

	digitalWrite(DIR_DEC_PIN, !pstepper->dir);
	digitalWrite(STEP_DEC_PIN, HIGH);

	uint32_t ms = micros();

	pstepper->dir == 1 ? pstepper->pos++:pstepper->pos--;
	while (micros() - ms <= 1);


	digitalWrite(STEP_DEC_PIN, LOW);

}

uint8_t readFCDec() {
	return digitalRead(FC_DEC_PIN);
}

void enableDec()
{
	digitalWrite(ENABLE_DEC_PIN,LOW);
}

void disableDec()
{
	digitalWrite(ENABLE_DEC_PIN,HIGH);
}
