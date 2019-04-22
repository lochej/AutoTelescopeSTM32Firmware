/*
 * uart_driver.h
 *
 *  Created on: Dec 2, 2018
 *      Author: JLH
 */

#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_


#include "stm32l4xx_hal.h"
#include "usart.h"
#include "stdarg.h"

#define UART_DRIVER_USE_DYN_ALLOC 1

#define MAX_RX_BUFFER_UART4 (128)
#define MAX_TX_BUFFER_UART4 (64)

#define MAX_RX_BUFFER_UART2 (256)
#define MAX_TX_BUFFER_UART2 (256)



struct Fifo_buf_struct
{
	uint16_t put_idx;
	uint16_t get_idx;
	uint16_t nbr;
	uint16_t max_nbr;
	uint8_t * buf;
};



typedef struct Fifo_buf_struct Fifo_Buffer;

struct Serial_UART_struct
{
	Fifo_Buffer * fifoRx;
	Fifo_Buffer * fifoTx;
	UART_HandleTypeDef * huart;
	uint8_t rdy;
};

typedef struct Serial_UART_struct Serial_UART;


extern Serial_UART Serial_GPS;
extern Serial_UART Serial_STLK;
extern Serial_UART Serial_BT;



#if !UART_DRIVER_USE_DYN_ALLOC

extern uint8_t RxBuffer_UART4[MAX_RX_BUFFER_UART4];
extern uint8_t TxBuffer_UART4[MAX_TX_BUFFER_UART4];

extern Fifo_Buffer Fifo_Rx_UART4;
extern Fifo_Buffer Fifo_Tx_UART4;



extern uint8_t RxBuffer_UART2[MAX_RX_BUFFER_UART2];
extern uint8_t TxBuffer_UART2[MAX_TX_BUFFER_UART2];

extern Fifo_Buffer Fifo_Rx_UART2;
extern Fifo_Buffer Fifo_Tx_UART2;

#endif

void Serial_UART_TxISR_8BIT(UART_HandleTypeDef *huart);

void Serial_UART_RxISR_8BIT(UART_HandleTypeDef *huart);

Serial_UART Serial_init(UART_HandleTypeDef * huart,Fifo_Buffer * rxbuffer,Fifo_Buffer * txbuffer);

Serial_UART Serial_new(UART_HandleTypeDef * huart, uint16_t max_rx_nb, uint16_t max_tx_nb);

void Serial_EnableRx(Serial_UART * puart);
void Serial_EnableTx(Serial_UART * puart);

void Serial_DisableRx(Serial_UART * puart);

uint16_t Serial_Available(Serial_UART * puart);

uint8_t Serial_Read(Serial_UART * puart,char * pbyte);

void Serial_printf(Serial_UART * puart, char* format, ...);

void Serial_writeBuf(Serial_UART * puart,char * buf,uint16_t length);

void Serial_FlushRx(Serial_UART * puart);





#endif /* UART_DRIVER_H_ */
