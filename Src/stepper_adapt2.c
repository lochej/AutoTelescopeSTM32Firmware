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

	stepperAh2.interStep_delay_us = 1000 * 1000 / 500; //5 steps/sec
	//stepperAh2.degreesPerSteps = 360.0/24000.0;
	stepperAh2.degreesPerSteps = 172.5/12400.0; //172.5° pour 12400 steps

	stepperAh2.pos_max=190/stepperAh2.degreesPerSteps;
	stepperAh2.ref_angle = 93;
	//stepperAh2.ref_angle = 177;
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
	stepperDec2.degreesPerSteps = 221.0/8000.0;
	stepperDec2.pos_max=200.0/stepperDec2.degreesPerSteps; //200 deg de course maximum
	stepperDec2.interStep_delay_us = 1000 * 1000 / 500; //5 steps/sec
	stepperDec2.ref_angle=100.3; //100.3 deg du point de reference dec=90 deg donc le zero est -10.3 deg le FC est a 10.3 du point 0 deg
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


uint8_t needsReturnedMode(double ah)
{
	return 0<=ah && ah <180;
}
/**
 * Pour une declinaison donnee donne l angle relatif la verticale jusqu'a langle voulu
 * @param dec
 * @param mode_retournee
 * @return
 */
double getRelativeAngleFromDec(double dec,uint8_t mode_retournee)
{

	if(dec<0 || dec>90)
	{
		return 0;
	}

	//Le zero du moteur DEC correspond à un angle 0 on vise tout droit.
	//l'angle va vers 90 a gauche et -90 a droite

	//La declinaison vaut 90 a la position zero du moteur
	//elle vaut 0 si le moteur est a -90 en mode normal non retournee et 0 a 90 si mode retournee

	//Donc quoi qu'il arrive, la reponse de cette fonction doit se trouver entre -90 et 90.

	if(mode_retournee)
	{
		return 90 - dec;
	}
	else
	{
		return dec - 90;
	}

	return 0;

}

/**
 * Pour un AH donne, on calcul la position depuis le point zero de reference du
 * @param ah
 * @param mode_retournee
 * @return
 */
double getRelativeAngleFromAh(double ah,uint8_t mode_retournee)
{

	if( ah <0 && ah > 360)
		return 0;

	if(mode_retournee)
	{
		return ah-90;
	}
	else
	{
		return 270-ah;
	}
}
