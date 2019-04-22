/*
 * stepper_ah.h
 *
 *  Created on: Nov 24, 2018
 *      Author: JLH
 */

#ifndef STEPPER_DRIVER_H_
#define STEPPER_DRIVER_H_


#ifdef __cplusplus
extern "C"{
#endif


#include <stdint.h>




enum StepperSFCStatus{
	IDLE,
	RUNNING,
	ABORTED
};


enum StepperMode
{
	CONTINUOUS,
	INCREMENTAL,
	CALIBRATION,
	MODE_COUNT
};


enum StepperState
{
	OK=0, //Attente d'instruction
	BUSY
};


typedef enum StepperMode StepperMode_t;
typedef enum StepperSFCStatus StepperSFCStatus_t;


struct StepperTelescope
{

	uint8_t Fc; //Fin de course : b0 = FcHard b1=FcSoftMin && Dir==0 b2=FcSoftMax && Dir==1
	uint8_t FcSoftMin;
	uint8_t FcSoftMax;
	uint8_t FcHard;

	//Axe calibre
	uint8_t IsCalibrated;

	//Both incremental et Continuous
	double degreesPerSteps;
	int32_t pos; //absolute position of the stepper from its home position (0)
	int32_t pos_min; //The min value of pos aka the minimum angle of the stepper from 0 position (should be 0)
	int32_t pos_max; //The max value of pos aka the maximum angle of the stepper from 0 position
	uint8_t dir;
	uint32_t step_ref_time_us;
	uint32_t interStep_delay_us;

	//uint8_t driving_mode; //Continuous or Incremental or Calibration
	StepperMode_t future_driving_mode;
	StepperMode_t driving_mode;
	uint8_t Etape_SFC_Driver;


	//Continuous
	uint8_t Flag_continuous_move;

	uint8_t Etape_SFC_Continuous;
	uint8_t Flag_SFC_Cont_EtapeExecuted;
	uint8_t Status_SFC_Continuous;



	//Incremental mode
	uint32_t stepsCmd;
	uint8_t Flag_incremental_docmd;

	uint8_t Etape_SFC_Incremental;
	uint8_t Flag_SFC_Inc_EtapeExecuted;
	uint8_t Status_SFC_Incremental;

	//Calibration mode

	uint32_t timeref_ms1;
	uint32_t timeref_ms2;
	uint8_t Etape_SFC_Calibration;
	uint8_t Flag_SFC_Calib_EtapeExecuted;
	uint8_t Status_SFC_Calibration;

	uint8_t Flag_calib_start;
	uint8_t Flag_calib_abort;

	void (*doStep)(struct StepperTelescope * pstepper);
	void (*enable)(void);
	void (*disable)(void);
	uint8_t (*readFCHard)(void);

};

/**
 * Returns the mode of operation of the stepper,
 * @return StepperMode_t : Calibration, Continuous, Incremental
 */
StepperMode_t StepperTelescope_getMode(struct StepperTelescope * pstepper);

/**
 * Swtich the mode of operation of the stepper to the given mode
 * If abot
 * Also see stop method
 * @param mode : the mode to set Calibration, Continuous of Incremental see StepperMode_t
 * @param abort: abort current mode and switch to new mode
 * @return 0: Mode will change Other: Busy see states
 */
uint8_t StepperTelescope_switchMode(struct StepperTelescope * pstepper,StepperMode_t mode,uint8_t abort);

/**
 * Returns the angle from 0 point after calibration
 * @return
 */
double StepperTelescope_getAngle(struct StepperTelescope * pstepper);

/**
 * Configurer la vitesse du stepper en stepsperhour
 * @param stepsperhour : double number of steps per hour
 * @return 0: Speed set, Other error
 */
uint8_t StepperTelescope_setSpeed(struct StepperTelescope * pstepper,double stepsperhour);

/**
 * Enable continuous move mode on the stepper in the given dir
 * @param dir : 0 move toward the limit switch 1 move away
 * @return 0: Mode enabled, Other : busy see states
 */
uint8_t StepperTelescope_moveEnable(struct StepperTelescope * pstepper,uint8_t dir);

/**
 * Stops any movement either in incremental mode, calibration or continuous mode
 * You should always call Stop before changing states or modifying dir
 * @return 0: Stop OK Other: busy see states
 */
uint8_t StepperTelescope_stop(struct StepperTelescope * pstepper);

/**
 * Move "steps" in the direction dir
 * @param steps : number of steps to do
 * @param dir : the direction 0 or 1
 * @return 0: Move will start Other: busy see states
 */
uint8_t StepperTelescope_moveAbs(struct StepperTelescope * pstepper,uint32_t steps,uint8_t dir);
uint8_t StepperTelescope_move(struct StepperTelescope * pstepper,int32_t steps);

/**
 * starts the limit switch finding on the current stepper
 * @param pstepper
 * @return 0: OK procedure started Other : busy see states
 */
uint8_t StepperTelescope_startCalibration(struct StepperTelescope * pstepper);

void StepperTelescope_init(struct StepperTelescope * pstepper);

/**
 * Internal method, process of calibration: SFC
 * You should always call in the same loop the function with stepNtransition=1 and 0
 * @param pstepper
 * @param stepNtransition
 */
void Calibration_Mode_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition);

/**
 * Internal method, process of incremental move : SFC
 * You should always call in the same loop the function with stepNtransition=1 and 0
 * @param pstepper
 * @param stepNtransition
 */
void Incremental_Mode_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition);

/**
 * Internal method, process of continuous move : SFC
 * You should always call in the same loop the function with stepNtransition=1 and 0
 * @param pstepper
 * @param stepNtransition
 */
void Continuous_Mode_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition);

/**
 * Call this function in main to authorize the stepper driving
 * Sequencer of the Different operation modes
 * Process scheduler and chooser for the stepper driving modes
 */
void StepperTelescope_Process(struct StepperTelescope * pstepper,uint8_t stepNtransition);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_DRIVER_H_ */
