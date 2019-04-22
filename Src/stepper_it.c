/*
 * stepper_it.h
 *
 *  Created on: Dec 27, 2018
 *      Author: JLH
 */

#include "stepper_it.h"
#include "Arduino_HAL.h"
#include "tim.h"
#include <string.h>
#include "stepper_adapt2.h"

void Stepper_init(struct Stepper_t * pstepper)
{
	memset(pstepper, 0, sizeof(struct Stepper_t));
}

void UpdateStepper2(struct Stepper_t * pstepper)
{

	//Lecture des fin de courses, et Maj des flags

	pstepper->FcHard = pstepper->readFCHard(); //Lire fin de course
	pstepper->FcSoftMin = pstepper->pos <= pstepper->pos_min;
	pstepper->FcSoftMax = pstepper->pos >= pstepper->pos_max;

	pstepper->Fc = (pstepper->FcHard && pstepper->dir == 0)
			|| (pstepper->FcSoftMin && pstepper->dir == 0)
			|| (pstepper->FcSoftMax && pstepper->dir == 1);

}

static void Stepper_stopIT(struct Stepper_t * pstepper)
{
	HAL_TIM_OC_Stop_IT(pstepper->htim, pstepper->timoc_channel);

	//pstepper->state = STOPPE;
}
void Stepper_stopMove(struct Stepper_t * pstepper)
{

	Stepper_stopIT(pstepper); //Stopper l'interruption
	pstepper->mode = DEACTIVATED; //Couper l'energie dans les coils dans 5 ms
	pstepper->state = RUNNIN;
	Stepper_NextIT(pstepper, 5000);

}

void Stepper_goTo(struct Stepper_t * pstepper, int32_t step)
{
	//On peut pas aller + loin que pos max
	if (step > pstepper->pos_max || step < pstepper->pos_min || pstepper->pos == step)
		return;

	//On coupe l'interruption
	Stepper_stopIT(pstepper);
	uint32_t dt=__HAL_TIM_GET_COUNTER(pstepper->htim);

	//Bouger le delta de steps pour arriver à la position voulue
	//Stepper_move(pstepper, step - pstepper->pos);
	pstepper->mode=POSITION;
	pstepper->posref=step;
	pstepper->dir= pstepper->posref > pstepper->pos ? 1 : 0;

	if(pstepper->state == RUNNIN) //Coils enabled
	{
		pstepper->enable(); //force coils ON
		Stepper_NextIT(pstepper, pstepper->interStep_delay_us - (__HAL_TIM_GET_COUNTER(pstepper->htim) - dt));
	}
	else
	{
		pstepper->enable();
		pstepper->state=RUNNIN;
		Stepper_NextIT(pstepper, 10000);
	}
}


int32_t Stepper_getAbsoluteStepPos(struct Stepper_t * pstepper)
{
	return pstepper->pos;
}


double Stepper_getAbsolutePos(struct Stepper_t * pstepper)
{
	return pstepper->pos*pstepper->degreesPerSteps;
}

double Stepper_getRelativePos(struct Stepper_t * pstepper)
{
	return Stepper_getAbsolutePos(pstepper) - pstepper->ref_angle;
}


double Stepper_getAbsoluteFromRelative(struct Stepper_t * pstepper,double relativeAngle)
{
	return relativeAngle + pstepper->ref_angle;
}

void Stepper_continuousMove(struct Stepper_t * pstepper, uint8_t dir)
{
	pstepper->dir = dir;
	pstepper->mode = CONTINUOU;
	pstepper->state = RUNNIN;

	pstepper->enable();

	Stepper_NextIT(pstepper, 10000);
}

void Stepper_move(struct Stepper_t * pstepper, int32_t steps)
{

	if (steps == 0)
		return;

	Stepper_stopIT(pstepper);

	if (pstepper->state == RUNNIN) //En cours de mouvement
	{
		//Stop IT timer
		pstepper->dir = steps > 0 ? 1 : 0;
		pstepper->stepsCmd = steps < 0 ? -steps : steps;

		pstepper->enable(); //Laisser les phase s'énergiser pendant 10 ms
		pstepper->mode = INCREMENTA;
		pstepper->state = RUNNIN;
		Stepper_NextIT(pstepper, pstepper->interStep_delay_us);

	}
	else if (pstepper->state == STOPPE)
	{
		pstepper->dir = steps > 0 ? 1 : 0;
		pstepper->stepsCmd = steps < 0 ? -steps : steps;

		pstepper->enable(); //Laisser les phase s'énergiser pendant 10 ms
		pstepper->mode = INCREMENTA;
		pstepper->state = RUNNIN;
		Stepper_NextIT(pstepper, 10000);
	}
}

void Stepper_calibrate(struct Stepper_t * pstepper)
{
	//Stop IT timer
	Stepper_stopIT(pstepper);

	pstepper->enable();

	pstepper->mode = CALIBRATIO;
	pstepper->state = RUNNIN;
	pstepper->IsCalibrated = 0;

	pstepper->dir = 0;
	pstepper->timeref_ms1 = millis();

	Stepper_NextIT(pstepper, 10000); //Lancer la calib dans 10 ms

}
void Stepper_NextIT(struct Stepper_t * pstepper, uint32_t us)
{
	HAL_TIM_OC_Stop_IT(pstepper->htim, pstepper->timoc_channel);
	//HAL_TIM_Base_Stop_IT(&htim16);
	//Relancer IT timer en fonction du pstepper
	//__HAL_TIM_SET_COUNTER(&htim16,0);
	//__HAL_TIM_SET_AUTORELOAD(&htim16,us);

	__HAL_TIM_SET_COMPARE(pstepper->htim, pstepper->timoc_channel,
			(uint16_t)(__HAL_TIM_GET_COUNTER(pstepper->htim) + us));

	//HAL_TIM_Base_Start(pstepper->htim);
	HAL_TIM_OC_Start_IT(pstepper->htim, pstepper->timoc_channel);
	//HAL_TIM_Base_Start_IT(&htim16);
}

void Stepper_IT_Process(struct Stepper_t * pstepper)
{
	UpdateStepper2(pstepper);

	//Désactiver l'IT timer
	switch (pstepper->mode)
	{

	case CALIBRATIO: //Calibration steps

		switch (pstepper->mode_state)
		{
		case 0: //Lancement de la calib

			if (pstepper->FcHard)
			{
				pstepper->dir = !pstepper->dir;
				pstepper->timeref_ms1 = millis();

				pstepper->mode_state = 1;

				Stepper_NextIT(pstepper, 100); //100 us

			}
			else if (millis() - pstepper->timeref_ms1 >= 30000) //Timeout calib
			{

				Stepper_stopMove(pstepper);
			}
			else
			{

				pstepper->doStep(pstepper);

				//Faire les steps
				Stepper_NextIT(pstepper, pstepper->interStep_delay_us);
			}

			break;
		case 1: //Running away from fin de course until deactivated

			//Attendre de désactiver le fin de course
			if (!pstepper->FcHard)
			{
				//Fin de calibration quand on a relaché le fin de course
				pstepper->IsCalibrated = 1;
				pstepper->pos = 0;
				pstepper->mode_state = 0;

				pstepper->mode = DEACTIVATED;
				Stepper_NextIT(pstepper, 10000);

			}
			//Timeout calibration
			else if (millis() - pstepper->timeref_ms1 >= 1000)
			{
				//Quitter
				pstepper->mode = DEACTIVATED;
				Stepper_NextIT(pstepper, 10000);
			}
			else
			{

				pstepper->doStep(pstepper);
				//Execute step
				Stepper_NextIT(pstepper, pstepper->interStep_delay_us * 2);

			}

			break;

		}

		break;
	case INCREMENTA: //Incremental steps

		if (pstepper->IsCalibrated)
		{

			if (pstepper->Fc)
			{
				//Arreter le mouvement incremental
				pstepper->mode = DEACTIVATED;

				Stepper_NextIT(pstepper, 10000); //Cut the coils in 1000us

			}
			//On a pas touché les FC et on est tjr autorisé à tourner
			else if (pstepper->stepsCmd > 0)
			{
				pstepper->doStep(pstepper);
				pstepper->stepsCmd--;

				//Calculer le temps interStep
				//Relancer le timer
				if (pstepper->stepsCmd == 0) //Arreter le movement dans 10ms
				{

					pstepper->mode = DEACTIVATED;
					Stepper_NextIT(pstepper, 10000);
				}
				else
					Stepper_NextIT(pstepper, pstepper->interStep_delay_us);
			}
			else
			{
				//Mouvement termine
				pstepper->mode = DEACTIVATED;
				Stepper_NextIT(pstepper, 10);
			}

		}
		else
		{
			//On ne peut pas faire de l'incremental quitter
			//Mouvement termine
			pstepper->mode = DEACTIVATED;
			Stepper_NextIT(pstepper, 10);
		}

		break;

	case CONTINUOU: //Continuous steps
		if (pstepper->IsCalibrated)
		{

			if (pstepper->Fc)
			{
				//Arreter le mouvement incremental

				pstepper->mode = DEACTIVATED;

				Stepper_NextIT(pstepper, 10000); //Cut coils in 100 us

			}
			//On a pas touché les FC et on est tjr autorisé à tourner
			else
			{
				pstepper->doStep(pstepper);

				//Calculer le temps interStep
				//Relancer le timer
				Stepper_NextIT(pstepper, pstepper->interStep_delay_us);
			}

		}
		else
		{
			//On ne peut pas faire de l'incremental quitter

			pstepper->mode = DEACTIVATED;

			Stepper_NextIT(pstepper, 10); //Cut coils in 10 us
		}

		break;

	case POSITION: //Rester sur la consigne
		if (pstepper->IsCalibrated)
		{

			if (pstepper->Fc)
			{
				pstepper->mode = DEACTIVATED;
				Stepper_NextIT(pstepper, 10000); //Cut the coils in 1000us
			}
			//On a pas touché les FC et on est tjr autorisé à tourner
			else if (pstepper->posref != pstepper->pos)
			{
				pstepper->dir= pstepper->posref > pstepper->pos ? 1 : 0;

				pstepper->doStep(pstepper);

				if (pstepper->posref == pstepper->pos)
				{
					pstepper->mode = DEACTIVATED;
					Stepper_NextIT(pstepper, 10000);
				}
				else
				{
					Stepper_NextIT(pstepper, pstepper->interStep_delay_us);
				}
			}
			else //On est sur la position voulue
			{
				//Mouvement termine
				pstepper->mode = DEACTIVATED;
				Stepper_NextIT(pstepper, 10000);
			}
		}
		else
		{
			//Mouvement termine
			pstepper->mode = DEACTIVATED;
			Stepper_NextIT(pstepper, 10);
		}
		break;
	case DEACTIVATED: //Disable coil energize

		pstepper->disable();
		Stepper_stopIT(pstepper);
		pstepper->state = STOPPE;
		break;
	default:
		break;
	}

}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef * htim)
{
	if (htim == stepperAh2.htim)
	{
		if (htim->Channel == stepperAh2.tim_active_channel)
		{
			Stepper_IT_Process(&stepperAh2);
		}
	}

	if (htim == stepperDec2.htim)
	{
		if (htim->Channel == stepperDec2.tim_active_channel)
		{
			Stepper_IT_Process(&stepperDec2);
		}
	}
}

/*
 void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim)
 {

 if(htim->Instance == TIM16)
 {
 Stepper_IT_Process(&stepperAh2);
 }
 }*/
