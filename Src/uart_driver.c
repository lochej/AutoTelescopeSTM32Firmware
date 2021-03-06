/*
 * uart_driver.c
 *
 *  Created on: Dec 2, 2018
 *      Author: JLH
 */
#if 0
#include "uart_driver.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "includes.h"



Serial_UART Serial_GPS,Serial_STLK,Serial_BT;


#if !UART_DRIVER_USE_DYN_ALLOC

uint8_t RxBuffer_UART4[MAX_RX_BUFFER_UART4];
uint8_t TxBuffer_UART4[MAX_TX_BUFFER_UART4];

Fifo_Buffer Fifo_Rx_UART4={0,0,0,MAX_RX_BUFFER_UART4,RxBuffer_UART4};
Fifo_Buffer Fifo_Tx_UART4={0,0,0,MAX_TX_BUFFER_UART4,TxBuffer_UART4};


uint8_t RxBuffer_UART2[MAX_RX_BUFFER_UART2];
uint8_t TxBuffer_UART2[MAX_TX_BUFFER_UART2];

Fifo_Buffer Fifo_Rx_UART2={0,0,0,MAX_RX_BUFFER_UART2,RxBuffer_UART2};
Fifo_Buffer Fifo_Tx_UART2={0,0,0,MAX_TX_BUFFER_UART2,TxBuffer_UART2};
#endif

Serial_UART Serial_init(UART_HandleTypeDef * huart, Fifo_Buffer * rxbuffer,Fifo_Buffer * txbuffer)
{
	Serial_UART s={rxbuffer,txbuffer,huart,1};

	Serial_EnableRx(&s);
	Serial_EnableTx(&s);
	return s;
}

Serial_UART Serial_new(UART_HandleTypeDef * huart, uint16_t max_rx_nb, uint16_t max_tx_nb)
{

	uint8_t * rxbuf=malloc(sizeof(uint8_t)*max_rx_nb);
	uint8_t * txbuf=malloc(sizeof(uint8_t)*max_tx_nb);

	memset(rxbuf,0,sizeof(uint8_t)*max_rx_nb);
	memset(txbuf,0,sizeof(uint8_t)*max_tx_nb);

	Fifo_Buffer * rxbuffer=malloc(sizeof(Fifo_Buffer));



	rxbuffer->get_idx=0;
	rxbuffer->nbr=0;
	rxbuffer->put_idx=0;
	rxbuffer->max_nbr=max_rx_nb;
	rxbuffer->buf=rxbuf;

	Fifo_Buffer * txbuffer=malloc(sizeof(Fifo_Buffer));

	txbuffer->get_idx=0;
	txbuffer->nbr=0;
	txbuffer->put_idx=0;
	txbuffer->max_nbr=max_tx_nb;
	txbuffer->buf=txbuf;

	Serial_UART s = {rxbuffer,txbuffer,huart,1};

	Serial_EnableRx(&s);
	Serial_EnableTx(&s);

	return s;
}

void Serial_EnableRx(Serial_UART * puart) {

	puart->huart->pRxBuffPtr= (uint8_t *) (puart->fifoRx);


	/* Computation of UART mask to apply to RDR register */
	puart->huart->Mask=0x00FFU;

	puart->huart->ErrorCode = HAL_UART_ERROR_NONE;
	puart->huart->RxState = HAL_UART_STATE_BUSY_RX;

	/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	//SET_BIT(puart->huart->Instance->CR3, USART_CR3_EIE);
	__HAL_UART_ENABLE_IT(puart->huart,UART_IT_ERR);

	//Binding the ISR function
	puart->huart->RxISR=Serial_UART_RxISR_8BIT;


	//Enable RXNE IT and parity errors
	//SET_BIT(puart->huart->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);
	__HAL_UART_ENABLE_IT(puart->huart,UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(puart->huart,UART_IT_PE);
}

void Serial_DisableRx(Serial_UART * puart) {

	puart->huart->pRxBuffPtr= NULL;


	/* Computation of UART mask to apply to RDR register */
	puart->huart->Mask=0x00FFU;

	puart->huart->ErrorCode = HAL_UART_ERROR_NONE;
	puart->huart->RxState = HAL_UART_STATE_READY;

	/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	//CLEAR_BIT(puart->huart->Instance->CR3, USART_CR3_EIE);
	__HAL_UART_DISABLE_IT(puart->huart,UART_IT_ERR);

	//Binding the ISR function
	puart->huart->RxISR=NULL;

	//disable RXNE IT and parity errors
	//CLEAR_BIT(puart->huart->Instance->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);
	__HAL_UART_DISABLE_IT(puart->huart,UART_IT_PE);
	__HAL_UART_DISABLE_IT(puart->huart,UART_IT_RXNE);
}

void Serial_EnableTx(Serial_UART * puart) {

	if (puart->huart->gState == HAL_UART_STATE_READY)
	{

		puart->huart->pTxBuffPtr  = (uint8_t *)puart->fifoTx;
		puart->huart->TxISR       = Serial_UART_TxISR_8BIT;

		puart->huart->ErrorCode = HAL_UART_ERROR_NONE;
		puart->huart->gState = HAL_UART_STATE_BUSY_TX;


		/* Enable the Transmit Data Register Empty interrupt */
		//SET_BIT(puart->huart->Instance->CR1, USART_CR1_TXEIE);
	}


}


void Serial_DisableTx(Serial_UART * puart) {

	/* Disable the UART Transmit Complete Interrupt */
	//CLEAR_BIT(puart->huart->Instance->CR1, USART_CR1_TCIE);
	__HAL_UART_DISABLE_IT(puart->huart,UART_IT_TC);

	/* Tx process is ended, restore huart->gState to Ready */
	puart->huart->gState = HAL_UART_STATE_READY;

	/* Cleat TxISR function pointer */
	puart->huart->TxISR = NULL;

}

uint16_t Serial_Available(Serial_UART * puart) {
	return puart->fifoRx->nbr;
}

uint8_t Serial_Read(Serial_UART * puart, char * pbyte) {
	if (puart->fifoRx->nbr == 0)
		return 1;

	*pbyte = puart->fifoRx->buf[puart->fifoRx->get_idx++]; /* Lecture octet du buffer */

	if (puart->fifoRx->get_idx >= puart->fifoRx->max_nbr)
		puart->fifoRx->get_idx = 0; /* MAJ indice fifo */

	puart->fifoRx->nbr--; /* Un octet de moins en fifo */

	return 0;

}


void Serial_FlushRx(Serial_UART * puart) {

	puart->fifoRx->put_idx = 0;
	puart->fifoRx->get_idx = 0;
	puart->fifoRx->nbr = 0;

	uint16_t i_ui16 = 0;
	do {
		puart->fifoRx->buf[i_ui16++] = 0x00;
	} while (i_ui16 < puart->fifoRx->max_nbr);

}

void Serial_printf(Serial_UART * puart, char* format, ...)
{
	static char tmp[256];
	//while(!puart->rdy);

	memset(tmp,0,256);
	//memset(puart->fifoTx->buf,0,puart->fifoTx->max_nbr);

	va_list vargs;

	va_start(vargs,format);

	//vsnprintf((char*) (puart->fifoTx->buf),puart->fifoTx->max_nbr,format,vargs);
	uint16_t len=vsnprintf(tmp,256,format,vargs);

	//Serial_writeBuf(puart, puart->fifoTx->buf, strlen(puart->fifoTx->buf));
	Serial_writeBuf(puart, tmp, len);

	va_end(vargs);

}

void Serial_writeBuf(Serial_UART * puart,char * buf,uint16_t length)
{

//	CLEAR_BIT(puart->huart->Instance->CR1,USART_CR1_TXEIE);

	__HAL_UART_DISABLE_IT(puart->huart,UART_IT_TXE);

	while(length-->0)
	{
		if ( puart->fifoTx->nbr < puart->fifoTx->max_nbr) /* Si fifo pleine */
		{
				puart->fifoTx->buf[puart->fifoTx->put_idx++] = *(buf++); /* Memorisation octet en fifo */

				if (puart->fifoTx->put_idx >= puart->fifoTx->max_nbr) puart->fifoTx->put_idx = 0; /* MAJ indice fifo */

				puart->fifoTx->nbr++; /* Un octet de plus en fifo */
		}

	}

	/* Enable the Transmit Data Register Empty interrupt */
	//SET_BIT(puart->huart->Instance->CR1, USART_CR1_TXEIE);

	__HAL_UART_ENABLE_IT(puart->huart,UART_IT_TXE);
	//while(!puart->rdy);
	//HAL_UART_Transmit_IT(puart->huart, buf, length);
	//puart->rdy=0;
}


/**
 * @brief RX interrrupt handler for 7 or 8 bits data word length .
 * @param huart UART handle.
 * @retval None
 */
void Serial_UART_RxISR_8BIT(UART_HandleTypeDef *huart)
{
	uint16_t uhMask = huart->Mask;
	uint16_t  uhdata;
	Fifo_Buffer * fifoRx= (Fifo_Buffer *) huart->pRxBuffPtr;


	/* Check that a Rx process is ongoing */
	if (huart->RxState == HAL_UART_STATE_BUSY_RX)
	{
		uhdata = (uint16_t) READ_REG(huart->Instance->RDR);


		if ( fifoRx->nbr < fifoRx->max_nbr) /* Si fifo pleine */
		{
			fifoRx->buf[fifoRx->put_idx++] = (uint8_t)(uhdata & (uint8_t)uhMask); /* Memorisation octet en fifo */

			if (fifoRx->put_idx >= fifoRx->max_nbr) fifoRx->put_idx = 0; /* MAJ indice fifo */

			fifoRx->nbr++; /* Un octet de plus en fifo */
		}

	}
	else
	{
		/* Clear RXNE interrupt flag */
		__HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
	}


}

/**
 * @brief RX interrrupt handler for 7 or 8 bits data word length .
 * @param huart UART handle.
 * @retval None
 */
void Serial_UART_TxISR_8BIT(UART_HandleTypeDef *huart)
{
	uint8_t byte;
	Fifo_Buffer * fifoTx= (Fifo_Buffer *) huart->pTxBuffPtr;

	/* Check that a Tx process is ongoing */
	if (huart->gState == HAL_UART_STATE_BUSY_TX)
	{


		if(fifoTx->nbr == 0) //Termine plus rien a envoyer
		{
			/* Disable the UART Transmit Data Register Empty Interrupt */
			//CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
			__HAL_UART_DISABLE_IT(huart,UART_IT_TXE);
			/* Enable the UART Transmit Complete Interrupt */
			//SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
		}
		else
		{

			byte = fifoTx->buf[fifoTx->get_idx++]; /* Lecture octet du buffer */

			if (fifoTx->get_idx >= fifoTx->max_nbr)
				fifoTx->get_idx = 0; /* MAJ indice fifo */

			fifoTx->nbr--; /* Un octet de moins en fifo */

			huart->Instance->TDR = (uint8_t)(byte);
		}

	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart)
{

}




