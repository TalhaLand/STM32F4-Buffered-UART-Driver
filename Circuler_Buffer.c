/*
 * Circuler_Buffer.c
 *
 *  Created on: 7 Ağu 2026
 *      Author: Mansu
 */

#include "Circuler_Buffer.h"

//Buffer initialization
void Circuler_Buffer_Init(Circuler_Buffer_t *circulerBuffer)
{
	memset(circulerBuffer->buffer, 0 , circuler_buffer_size);

	circulerBuffer->head		= 0;
	circulerBuffer->tail		= 0;
}


bool circuler_buffer_is_empty(Circuler_Buffer_t *circulerBuffer)
{
	return(circulerBuffer->head == circulerBuffer->tail) ? true : false;
}


bool circuler_buffer_is_fully(Circuler_Buffer_t *circulerBuffer)
{
	int div = circulerBuffer->head - circulerBuffer->tail;

	if(div < 0)
	{
		div = div + circuler_buffer_size;
	}
	return(div == (circuler_buffer_size - 1)) ? true : false;

}


bool Circuler_Buffer_Enqueue(Circuler_Buffer_t *circulerBuffer , uint8_t data)
{
	if(circuler_buffer_is_fully(circulerBuffer))
	{
		return false;
	}
	circulerBuffer->buffer[circulerBuffer->head] = data;

	circulerBuffer->head = (circulerBuffer->head + 1) % circuler_buffer_size;

	return true;
}

uint8_t Circuler_Buffer_Dequeue(Circuler_Buffer_t *circulerBuffer, uint8_t *data)
{
	if(circuler_buffer_is_empty(circulerBuffer))
		{
			return false;
		}
	*data = circulerBuffer->buffer[circulerBuffer->tail];
	circulerBuffer->tail = (circulerBuffer->tail + 1) % circuler_buffer_size;

	return true;

}

uint8_t Circuler_Buffer_Count(Circuler_Buffer_t *circulerBuffer)
{
	if(circulerBuffer->head >= circulerBuffer->tail)
	{
		return circulerBuffer->head - circulerBuffer->tail;
	}
	else
	{
		return (circuler_buffer_size - circulerBuffer->tail + circulerBuffer->head);
	}

}

