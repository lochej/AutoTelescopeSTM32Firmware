/*
 * stepper_adapt.h
 *
 *  Created on: Dec 8, 2018
 *      Author: JLH
 */

#ifndef STEPPER_ADAPT_H_
#define STEPPER_ADAPT_H_


#include "stepper_driver.h"
#include "Arduino_HAL.h"

extern struct StepperTelescope stepperAh,stepperDec;


void StepperTelescope_initSteppers();


void doStepAh(struct StepperTelescope * pstepper);
void doStepDec(struct StepperTelescope * pstepper);

uint8_t readFCAh();
uint8_t readFCDec();

void enableAh();
void enableDec();

void disableAh();
void disableDec();

#endif /* STEPPER_ADAPT_H_ */
