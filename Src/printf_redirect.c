/*
 * printf_redirect.c
 *
 *  Created on: 29 nov. 2017
 *      Author: JLH
 */
#ifdef __cplusplus
extern "C" {
#endif

#if 1
#include "stm32L4xx.h"
#endif

//extern UART_HandleTypeDef huart2,huart4;

#define ITM_ENABLE 1

void Init_ITM()
{
#if ITM_ENABLE

	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;//Enable the ITM function
	DBGMCU->CR=(1<<5); //Enable the traceio in asyncrhonous
	ITM->LAR=0xC5ACCE55; //Unlock the registers of the ITM
	ITM->TCR=0x00010005;
	//ITM->TCR|=ITM_TCR_ITMENA_Msk; //Enable the ITM
	ITM->TER|=1UL; //Enable the PORT0 of the ITM
	ITM->TPR|=1UL; //Enable the PORT0 of the ITM
#endif
}
/**********************************************************************/
int _write(int file, char *ptr, int len)
{


    //HAL_UART_Transmit(&huart2,(uint8_t*)ptr,len,0xFFFF);
    //HAL_UART_Transmit(&huart4,(uint8_t*)ptr,len,0xFFFF);

#if ITM_ENABLE
	while(len--)
	{
		ITM_SendChar(*ptr++);
	}
#endif

	/*
	while(Flag_WaitForTxEnd_UART3_ui8);

	//Activate the TX RS485 driver
	HAL_GPIO_WritePin(RS485_TXE_PB5_GPIO_Port, RS485_TXE_PB5_Pin, SET);
	Flag_WaitForTxEnd_UART3_ui8=1;
	HAL_UART_Transmit_DMA(&huart1,(uint8_t*)ptr,len);
	 */


	//Attendre la fin du transfert DMA pour continuer


	return 0;
}
/**********************************************************************/

#ifdef __cplusplus
}
#endif
