/*
 * UART_ex.c
 *
 *  Created on: 8 Ağu 2026
 *      Author: Mansu
 */

#include "UART_ex.h"

void UARTx_Initialization(UART_Ex_t	*uart, UART_HandleTypeDef *huart, Circuler_Buffer_t *cbIn , Circuler_Buffer_t *cbOut)
{
	uart->huart 	= 	huart;
	uart->cbIn 		= 	cbIn;
	uart->cbOut		=	cbOut;

	Circuler_Buffer_Init(uart->cbIn);
	Circuler_Buffer_Init(uart->cbOut);

	__HAL_UART_ENABLE_IT(uart->huart, UART_IT_RXNE);    //RX interrupt aktif et
	__HAL_UART_ENABLE_IT(uart->huart, UART_IT_TXE); 	//TX interrupt aktif et

}

void UARTx_Write_Char(UART_Ex_t	*uart, char ch)
{
	if(Circuler_Buffer_Enqueue(uart->cbOut, ch))
	{
		if(!(uart->huart->Instance->CR1 & USART_CR1_TXEIE))
		{
			uint8_t ch;
			if(Circuler_Buffer_Dequeue(uart->cbOut, &ch))
			{

				uart->huart->Instance->DR = ch;
				__HAL_UART_ENABLE_IT(uart->huart, UART_IT_TXE);
			}
		}


	}

}

void UARTx_Put_String(UART_Ex_t	*uart, char *str)  //Karakterlerimi tek tek while ile uarta gönderecek
{
	while(*str)
	{

		UARTx_Write_Char(uart, *str);

		str++;
	}


}

int UARTx_Printf(UART_Ex_t	*uart, const char *format, ...)
{
	char tx_Buffer[256];

	va_list args;

	va_start(args, format);

	int lenght = vsnprintf(tx_Buffer, sizeof(tx_Buffer), format, args);

	va_end(args);


	UARTx_Put_String(uart, tx_Buffer);

	return lenght;
}

bool UARTx_ReadLine(UART_Ex_t *uart, char *lineBuffer, uint16_t maxLen)
{
	static uint16_t 	index 			= 	0;
	static bool			messageReady 	=  	false;
	static bool 		lastCr 			=	false;
	uint8_t				ch				=	0;

	if(messageReady)
	{

		//TODO Buffer aktar

		lineBuffer[index]	= 		'\0'	;
		index 				=		0		;
		messageReady 		= 		false	;

		return true;
	}

	while(Circuler_Buffer_Dequeue(uart->cbIn, &ch))
	{
		if(lastCr && ch == '\n' )
		{
			messageReady = true;
			lastCr = false;
			break;
		}
		if(ch == '\r')
		{
			lastCr = true;
		}
		else
		{
			if(lastCr)
			{
				if(index < (maxLen - 1))
				{
					lineBuffer[index] = '\r';
				}

				lastCr = false;

			}
			if(index < (maxLen - 1))
			{
				lineBuffer[index++] = ch;

			}

		}

	}

	return false;

}

