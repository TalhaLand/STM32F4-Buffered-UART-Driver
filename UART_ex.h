/*
 * UART_ex.h
 *
 *  Created on: 8 Ağu 2026
 *      Author: Mansu
 */

#ifndef INC_UART_EX_H_
#define INC_UART_EX_H_

#include "main.h"
#include "Circuler_Buffer.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

typedef struct
{
	UART_HandleTypeDef 		*huart;
	Circuler_Buffer_t		*cbIn;
	Circuler_Buffer_t		*cbOut;
}UART_Ex_t;

void UARTx_Initialization(UART_Ex_t	*uart, UART_HandleTypeDef *huart, Circuler_Buffer_t *cbIn , Circuler_Buffer_t *cbOut);

void UARTx_Write_Char(UART_Ex_t	*uart, char ch);

void UARTx_Put_String(UART_Ex_t	*uart, char *str);

int UARTx_Printf(UART_Ex_t	*uart, const char *format, ...);

bool UARTx_ReadLine(UART_Ex_t *uart, char *lineBuffer, uint16_t maxLen);



#endif /* INC_UART_EX_H_ */
