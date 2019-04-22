/*
 * stepper_it.h
 *
 *  Created on: Dec 27, 2018
 *      Author: JLH
 */

#ifndef STEPPER_IT_H_
#define STEPPER_IT_H_

#include <stdint.h>
#include "tim.h"

#define CALIBRATIO 0
#define INCREMENTA 1
#define CONTINUOU 2
#define POSITION 4
#define DEACTIVATED 3


#define RUNNIN 1
#define STOPPE 0

struct Stepper_t
{
	uint8_t Fc; //Fin de course : b0 = FcHard b1=FcSoftMin && Dir==0 b2=FcSoftMax && Dir==1
	uint8_t FcSoftMin;
	uint8_t FcSoftMax;
	uint8_t FcHard;

	//Position info
	int32_t pos; //absolute position of the stepper from its home position (0)
	int32_t pos_min; //The min value of pos aka the minimum angle of the stepper from 0 position (should be 0)
	int32_t pos_max; //The max value of pos aka the maximum angle of the stepper from 0 position
	uint8_t dir;

	uint32_t interStep_delay_us;

	//Axe calibre
	uint8_t IsCalibrated;

	uint8_t mode;
	uint8_t state;
	uint8_t mode_state;
	uint32_t timeref_ms1;
	uint32_t timeref_ms2;

	uint32_t stepsCmd;
	int32_t posref;

	double degreesPerSteps;
	double ref_angle;

	TIM_HandleTypeDef * htim;
	uint32_t timoc_channel;
	HAL_TIM_ActiveChannel tim_active_channel;


	void (*doStep)(struct Stepper_t * pstepper);
	void (*enable)(void);
	void (*disable)(void);
	uint8_t (*readFCHard)(void);
};

void Stepper_NextIT(struct Stepper_t * pstepper,uint32_t us);

static inline void Stepper_enable(struct Stepper_t * pstepper){ pstepper->enable();}
static inline void Stepper_disable(struct Stepper_t * pstepper){pstepper->disable();}

static inline uint8_t Stepper_getState(struct Stepper_t * pstepper){ return pstepper->state;}

void UpdateStepper2(struct Stepper_t * pstepper);


void Stepper_init(struct Stepper_t * pstepper);
//void Stepper_stopIT(struct Stepper_t * pstepper); //static declaration
void Stepper_stopMove(struct Stepper_t * pstepper);

void Stepper_goTo(struct Stepper_t * pstepper, int32_t step);
void Stepper_move(struct Stepper_t * pstepper, int32_t steps);
void Stepper_continuousMove(struct Stepper_t * pstepper, uint8_t dir);

double Stepper_getAbsoluteFromRelative(struct Stepper_t * pstepper,double relativeAngle);
int32_t Stepper_getAbsoluteStepPos(struct Stepper_t * pstepper);
double Stepper_getAbsolutePos(struct Stepper_t * pstepper);
double Stepper_getRelativePos(struct Stepper_t * pstepper);

void Stepper_calibrate(struct Stepper_t * pstepper);
void Stepper_NextIT(struct Stepper_t * pstepper,uint32_t us);

void Stepper_IT_Process(struct Stepper_t * pstepper);

#endif /* STEPPER_IT_H_ */
