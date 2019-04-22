/*
 * stepper_ah.c
 *
 *  Created on: Nov 24, 2018
 *      Author: JLH
 */

#include <stepper_driver.h>
#include "Arduino_HAL.h"
#include "string.h"

#define GET_CURRENT_SFC_STEP(_pstep) ((_pstep)->driving_mode==INCREMENTAL ? (_pstep)->Etape_SFC_Incremental : \
		(_pstep)->driving_mode==CONTINUOUS ? (_pstep)->Etape_SFC_Continuous:\
				(_pstep)->driving_mode==CALIBRATION ? (_pstep)->Etape_SFC_Calibration : 0)

void UpdateStepper(struct StepperTelescope * pstepper)
{

	//Lecture des fin de courses, et Maj des flags

	pstepper->FcHard=pstepper->readFCHard(); //Lire fin de course
	pstepper->FcSoftMin= pstepper->pos <= pstepper->pos_min;
	pstepper->FcSoftMax= pstepper->pos >= pstepper->pos_max;

	pstepper->Fc= (pstepper->FcHard && pstepper->dir==0) || (pstepper->FcSoftMin && pstepper->dir==0) || (pstepper->FcSoftMax && pstepper->dir==1);

}

void StepperTelescope_init(struct StepperTelescope * pstepper)
{
	memset(pstepper,0,sizeof(struct StepperTelescope));


}


void Calibration_Mode_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition)
{
	UpdateStepper(pstepper);

	switch(pstepper->Etape_SFC_Calibration)
	{
	case 0: //Idle

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Calib_EtapeExecuted)
			{
				//Step code
				//Idle, don't start the calibration
				pstepper->Flag_calib_start=0;
				pstepper->Flag_calib_abort=0;
				pstepper->disable();
			}
		}
		else //Transition
		{
			//Transition code

			if(pstepper->Flag_calib_start)
			{
				//Demarrer la calib
				pstepper->Etape_SFC_Calibration=1;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}

		}
		break;
	case 1: //Init

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Calib_EtapeExecuted)
			{
				//Step code
				//Reset, not calibrated
				pstepper->enable();
				pstepper->IsCalibrated=0;
				pstepper->dir=0;
				pstepper->timeref_ms1=millis();
				pstepper->timeref_ms2=millis();
			}
		}
		else //Transition
		{

			if(pstepper->Flag_calib_abort)
			{
				pstepper->Etape_SFC_Calibration=0;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
				break;
			}

			if(millis() - pstepper->timeref_ms2 >= 10)
			{
				//Transition code
				pstepper->Etape_SFC_Calibration=2;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}

		}
		break;
	case 2: //Running towards fin de course
		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Calib_EtapeExecuted)
			{
				//Step code
				pstepper->step_ref_time_us=micros();
				pstepper->doStep(pstepper);
			}
		}
		else //Transition
		{
			if(pstepper->Flag_calib_abort)
			{
				pstepper->Etape_SFC_Calibration=0;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
				break;
			}

			//Attendre de toucher le fin de course Hard
			if(pstepper->FcHard)
			{

				//change the direction to go in the other direction
				pstepper->dir=!pstepper->dir;
				pstepper->timeref_ms1=millis();
				//Reduced the speed of the stepper

				pstepper->Etape_SFC_Calibration=3;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}
			//Timeout calibration
			else if(millis() - pstepper->timeref_ms1 >= 30000)
			{
				//Quit calibration
				pstepper->Etape_SFC_Calibration=0;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}
			//Transition code
			else if(micros() - pstepper->step_ref_time_us >= pstepper->interStep_delay_us)
			{
				//execute step
				pstepper->Etape_SFC_Calibration=2;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}

		}


		break;
	case 3: //Running away from fin de course until deactivated

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Calib_EtapeExecuted)
			{
				//Step code
				pstepper->step_ref_time_us=micros();
				pstepper->doStep(pstepper);
			}
		}
		else //Transition
		{
			//Transition code

			if(pstepper->Flag_calib_abort)
			{
				//Quitter
				pstepper->Etape_SFC_Calibration=0;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
				break;
			}

			//Attendre de désactiver le fin de course
			if(!pstepper->FcHard)
			{
				//Fin de calibration quand on a relaché le fin de course
				pstepper->Etape_SFC_Calibration=4;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}
			//Timeout calibration
			else if(millis() - pstepper->timeref_ms1 >= 1000)
			{
				//Quitter
				pstepper->Etape_SFC_Calibration=0;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}
			else if(micros() - pstepper->step_ref_time_us >= (pstepper->interStep_delay_us << 1))
			{
				//execute step
				pstepper->Etape_SFC_Calibration=3;
				pstepper->Flag_SFC_Calib_EtapeExecuted=0;
			}

		}

		break;
	case 4: //End of calibration
		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Calib_EtapeExecuted)
			{
				pstepper->IsCalibrated = 1;
				pstepper->pos=0;
				//pstepper->pos_min=0;
				//pstepper->pos_max=12000;
			}
		}
		else //Transition
		{
			//Transition code

			pstepper->Etape_SFC_Calibration=0;
			pstepper->Flag_SFC_Calib_EtapeExecuted=0;

		}


		break;
	}

	//Si on  execute une step, alors la marquer comme executée
	if(stepNtransition)
	{
		pstepper->Flag_SFC_Calib_EtapeExecuted=1;
	}
}

void Incremental_Mode_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition)
{

	UpdateStepper(pstepper);

	switch(pstepper->Etape_SFC_Incremental)
	{
	case 0: //Idle do nothing

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Inc_EtapeExecuted)
			{
				//Step code
				pstepper->disable();
			}
		}
		else //Transition
		{
			//Transition code
			if(!pstepper->Flag_incremental_docmd) //able to run the incremental command ?
			{
				//Rester à l'etape courante
			}
			else if(pstepper->stepsCmd > 0) //a-t-on reçu une commande de deplacement ?
			{
				if(!pstepper->Fc) //Si on est pas en fin de course, exec les steps
				{

					//activer le driver
					pstepper->Etape_SFC_Incremental=1;
					pstepper->Flag_SFC_Inc_EtapeExecuted=0;
				}

			}


		}
		break;

	case 1: //Attente que le signal Enable prenne place

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Inc_EtapeExecuted)
			{
				//Step code
				pstepper->timeref_ms1=millis();
				pstepper->enable();
			}
		}
		else //Transition
		{
			//Transition code
			if(!pstepper->Flag_incremental_docmd) //able to run the incremental command ?
			{
				//Rester à l'etape courante
				pstepper->Etape_SFC_Incremental=0;
				pstepper->Flag_SFC_Inc_EtapeExecuted=0;
			}
			//Attendre 10 ms que le signal Enable soit bien posé.
			else if(millis() - pstepper->timeref_ms1 >= 10) //Si on est pas en fin de course, exec les steps
			{
				pstepper->Etape_SFC_Incremental=2;
				pstepper->Flag_SFC_Inc_EtapeExecuted=0;
			}

		}


		break;

	case 2: //Running turning in Dir direction
		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Inc_EtapeExecuted)
			{
				//Step code
				pstepper->step_ref_time_us=micros();
				pstepper->doStep(pstepper);

				//One step has just been executed
				pstepper->stepsCmd--;
			}
		}
		else //Transition
		{


			if(!pstepper->Flag_incremental_docmd)
			{
				//Arreter les steps, on ne veut plus faire cette commande.
				pstepper->Etape_SFC_Incremental=0;
				pstepper->Flag_SFC_Inc_EtapeExecuted=0;
			}
			//Si on touche le fin de course soft ou hard
			else if(pstepper->Fc)
			{
				//Arreter le mouvement incremental
				pstepper->Etape_SFC_Incremental=0;
				pstepper->Flag_SFC_Inc_EtapeExecuted=0;
			}
			//On a pas touché les FC et on est tjr autorisé à tourner
			else if(pstepper->stepsCmd > 0)
			{
				if(micros() - pstepper->step_ref_time_us >= pstepper->interStep_delay_us)
				{
					pstepper->Etape_SFC_Incremental=2; //Continuer le mouvement
					pstepper->Flag_SFC_Inc_EtapeExecuted=0;
				}

			}
			else
			{
				//Mouvement termine
				//Mode Idle
				pstepper->Etape_SFC_Incremental=0;
				pstepper->Flag_SFC_Inc_EtapeExecuted=0;
			}


		}


		break;

	}

	//Si on  execute une step, alors la marquer comme executée
	if(stepNtransition)
	{
		pstepper->Flag_SFC_Inc_EtapeExecuted=1;
	}
}

void Continuous_Mode_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition)
{

	UpdateStepper(pstepper);


	switch(pstepper->Etape_SFC_Continuous)
	{
	case 0: //IDLE, attente

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Cont_EtapeExecuted)
			{
				//Step code
				pstepper->disable();
			}
		}
		else //Transition
		{
			//Transition code

			if(pstepper->Flag_continuous_move) //on peut tourner
			{

				if(pstepper->IsCalibrated) //Il faut être calibré pour faire du mode continu
				{
					if(!pstepper->Fc) //Si on est pas en fin de course
					{


						pstepper->Etape_SFC_Continuous=1;
						pstepper->Flag_SFC_Cont_EtapeExecuted=0;
					}

				}

			}
		}

		break;

	case 1: //Attente que le signal Enable prenne place

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Cont_EtapeExecuted)
			{
				//Step code
				pstepper->timeref_ms1=millis();
				pstepper->enable();
			}
		}
		else //Transition
		{
			//Transition code
			if(!pstepper->Flag_continuous_move) //able to run the continuous command ?
			{
				//Rester à l'etape courante
				pstepper->Etape_SFC_Continuous=0;
				pstepper->Flag_SFC_Cont_EtapeExecuted=0;
			}
			//Attendre 10 ms que le signal Enable soit bien posé.
			else if(millis() - pstepper->timeref_ms1 >= 10) //Si on est pas en fin de course, exec les steps
			{
				pstepper->Etape_SFC_Continuous=2;
				pstepper->Flag_SFC_Cont_EtapeExecuted=0;
			}

		}


		break;

	case 2: //Tourner tant que pas dans un fin de course et que flag continuous move = 1

		if(stepNtransition) //Execute step
		{
			if(!pstepper->Flag_SFC_Cont_EtapeExecuted)
			{

				//Step code
				pstepper->step_ref_time_us=micros();
				pstepper->doStep(pstepper);
			}
		}
		else //Transition
		{
			//Transition code

			if(!pstepper->Flag_continuous_move) //Le mode continu est desactivé pas besoin de checker le fin de course, on sort d'ici
			{
				//Quitter le mode continue
				//Mode Idle
				pstepper->Etape_SFC_Continuous=0;
				pstepper->Flag_SFC_Cont_EtapeExecuted=0;
			}
			else if(pstepper->Fc) //tjr autorisé a bouger mais en butée sur fin de course
			{
				//Interdire le mouvement
				//pstepper->disable();
				//pstepper->Etape_SFC_Continuous=2; //Bloqué on ne reexecute pas la step 2 tant qu'on est bloqué sur fin de course
				//pstepper->Flag_SFC_Cont_EtapeExecuted=1;
				pstepper->Etape_SFC_Continuous=0;
				pstepper->Flag_SFC_Cont_EtapeExecuted=0;
				pstepper->Flag_continuous_move=0;
			}
			else //pas de fin de course et tjr autorise a tourner
			{
				if(micros() - pstepper->step_ref_time_us >= pstepper->interStep_delay_us)
				{
					pstepper->Etape_SFC_Continuous=2; //Continuer le mouvement
					pstepper->Flag_SFC_Cont_EtapeExecuted=0;
				}
			}

		}



		break;


	}

	//Si on  execute une step, alors la marquer comme executée
	if(stepNtransition)
	{
		pstepper->Flag_SFC_Cont_EtapeExecuted=1;
	}

}

void StepperTelescope_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition)
{


	switch(pstepper->driving_mode)
	{
	//case DISABLED: //Do nothing
	//break;
	case INCREMENTAL:
		Incremental_Mode_Process(pstepper, stepNtransition);
		break;
	case CONTINUOUS:
		Continuous_Mode_Process(pstepper, stepNtransition);
		break;
	case CALIBRATION:
		Calibration_Mode_Process(pstepper, stepNtransition);
		break;
	case MODE_COUNT:
		break;
	}

	if(stepNtransition)
	{

	}
	else //Transition to next driving mode
	{


		//A chaque fois que le processus courant est à l'etape 0
		//Passer au mode de pilotage futur


		//Si on doit aborter le mode courant, l'aborter

		//Doit-t-on changer de mode de pilotage ?
#if 0
		if(pstepper->Flag_incremental_docmd)
		{

			//verifier que le mode a bien aborté
			//Reset du mode
			//Passer au mode suivant

			/*
			if(GET_CURRENT_SFC_STEP(pstepper)==10)
			{

				GET_CURRENT_SFC_STEP(pstepper)=0;
				pstepper->Flag_abort_cmd=0;
			}
			 */


			switch(pstepper->driving_mode)
			{
			case INCREMENTAL:
				//Aborted success
				if(pstepper->Etape_SFC_Incremental==10)
				{
					pstepper->Etape_SFC_Incremental=0;
					pstepper->Flag_SFC_Inc_EtapeExecuted=0;
					pstepper->driving_mode=pstepper->future_driving_mode;
					pstepper->Flag_incremental_docmd=0;
				}
				break;
			case CONTINUOUS:
				//Aborted success

				//When continuous mode is off, the state = 0
				if(pstepper->Etape_SFC_Continuous==0)
				{
					pstepper->Etape_SFC_Continuous=0;
					pstepper->Flag_SFC_Cont_EtapeExecuted=0;
					pstepper->driving_mode=pstepper->future_driving_mode;
					pstepper->Flag_incremental_docmd=0;
				}
				break;
			case CALIBRATION:
				//Aborted success
				if(pstepper->Etape_SFC_Calibration==10)
				{
					pstepper->Etape_SFC_Calibration=0;
					pstepper->Flag_SFC_Calib_EtapeExecuted=0;
					pstepper->driving_mode=pstepper->future_driving_mode;
					pstepper->Flag_incremental_docmd=0;
				}
				break;
			}



		}
#endif

		switch(pstepper->driving_mode)
		{
		case INCREMENTAL:
			//Aborted success
			if(pstepper->Etape_SFC_Incremental==0)
			{
				pstepper->driving_mode=pstepper->future_driving_mode;
			}
			break;
		case CONTINUOUS:
			//Aborted success
			if(pstepper->Etape_SFC_Continuous==0)
			{
				pstepper->driving_mode=pstepper->future_driving_mode;
			}
			break;
		case CALIBRATION:
			//Aborted success
			if(pstepper->Etape_SFC_Calibration==0)
			{
				pstepper->driving_mode=pstepper->future_driving_mode;
			}
			break;
		case MODE_COUNT:
			break;
		}


	}

}

StepperMode_t StepperTelescope_getMode(struct StepperTelescope * pstepper)
{
	return pstepper->driving_mode;
}

//TODO
uint8_t StepperTelescope_switchMode(struct StepperTelescope * pstepper,StepperMode_t mode,uint8_t abort)
{

	//If busy
	if(0)
	{
		return BUSY;
	}

	pstepper->future_driving_mode=mode;
	StepperTelescope_stop(pstepper);
	return OK;
}

double StepperTelescope_getAngle(struct StepperTelescope * pstepper)
{
	return pstepper->degreesPerSteps*pstepper->pos;
}

//TODO
uint8_t StepperTelescope_setSpeed(struct StepperTelescope * pstepper,double stepsperhour)
{
	//Check speed availability: too fast, too slow...

	if(0) //Speed too fast
	{
		return 1; //Error
	}

	pstepper->interStep_delay_us= (uint32_t) (1000UL*1000UL/(stepsperhour/3600));

	return OK;
}

//TODO
uint8_t StepperTelescope_moveEnable(struct StepperTelescope * pstepper,uint8_t dir)
{
	//if check busy
	if(0)
	{
		return 1;
	}


	//Enable continuous mode
	pstepper->Flag_continuous_move=1;
	pstepper->dir=dir;

	pstepper->Etape_SFC_Continuous=0;
	pstepper->Flag_SFC_Cont_EtapeExecuted=0;

	//pstepper->Flag_abort_cmd=0;

	//StepperTelescope_switchMode(pstepper, CONTINUOUS,1);
	return OK;
}

uint8_t StepperTelescope_stop(struct StepperTelescope * pstepper)
{
	switch(pstepper->driving_mode)
	{
	case CONTINUOUS:
		pstepper->Flag_continuous_move=0;
		//pstepper->Flag_abort_cmd=1;
		break;
	case CALIBRATION:
		pstepper->Flag_calib_abort=1;
		break;
	case INCREMENTAL:
	default:
		pstepper->Flag_incremental_docmd=0;
		break;
	}

	return OK;
}

//TODO
uint8_t StepperTelescope_moveAbs(struct StepperTelescope * pstepper,uint32_t steps,uint8_t dir)
{
	// If check busy
	if(0)
	{
		return 1;
	}

	pstepper->stepsCmd=steps;
	pstepper->dir=dir;
	pstepper->Etape_SFC_Incremental=0;
	pstepper->Flag_SFC_Inc_EtapeExecuted=0;
	pstepper->Flag_incremental_docmd=1;

	//StepperTelescope_switchMode(pstepper, INCREMENTAL,1);

	return OK;
}

uint8_t StepperTelescope_move(struct StepperTelescope * pstepper,int32_t steps)
{


	return StepperTelescope_moveAbs(pstepper,steps < 0 ? -steps: steps,steps > 0 ? 1 : 0);
}

uint8_t StepperTelescope_startCalibration(struct StepperTelescope * pstepper){

	if(0)
	{
		return 1;
	}

	/*
	pstepper->Etape_SFC_Calibration=0;
	pstepper->Flag_SFC_Calib_EtapeExecuted=0;
	 */
	pstepper->Flag_calib_start=1;

	//StepperTelescope_switchMode(pstepper, CALIBRATION,1);
	return OK;
}

