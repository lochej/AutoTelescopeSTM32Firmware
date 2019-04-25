/*
 * stepper_adapt.h
 *
 *  Created on: Dec 8, 2018
 *      Author: JLH
 */

#ifndef STEPPER_ADAPT2_H_
#define STEPPER_ADAPT2_H_


#include "stepper_it.h"


extern struct Stepper_t stepperAh2,stepperDec2;


void Stepper_initSteppers();

uint8_t needsReturnedMode(double ah);
double getRelativeAngleFromDec(double dec,uint8_t mode_retournee);
double getRelativeAngleFromAh(double ah,uint8_t mode_retournee);
double getDecAngleFromRelative(double rel,uint8_t mode_retournee);
double getAhAngleFromRelative(double rel,uint8_t mode_retournee);


void doStepAh2(struct Stepper_t * pstepper);
void doStepDec2(struct Stepper_t * pstepper);

uint8_t readFCAh2();
uint8_t readFCDec2();

void enableAh2();
void enableDec2();

void disableAh2();
void disableDec2();

#endif /* STEPPER_ADAPT2_H_ */
