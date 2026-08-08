/*
 * Circuler_Buffer.h
 *
 *  Created on: 7 Ağu 2026
 *      Author: Mansu
 */

#ifndef INC_CIRCULER_BUFFER_H_
#define INC_CIRCULER_BUFFER_H_

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#define circuler_buffer_size		512



typedef struct
{
	uint8_t buffer[circuler_buffer_size];
	uint16_t head;	// head → yazma noktası
	uint16_t tail;  // tail → okuma noktası

}Circuler_Buffer_t;



//Buffer initialization
void Circuler_Buffer_Init(Circuler_Buffer_t *circulerBuffer);

// is buffer empty // is buffer full?

bool circuler_buffer_is_fully(Circuler_Buffer_t *circulerBuffer);

bool circuler_buffer_is_empty(Circuler_Buffer_t *circulerBuffer);

bool Circuler_Buffer_Enqueue(Circuler_Buffer_t *circulerBuffer , uint8_t data);

uint8_t Circuler_Buffer_Dequeue(Circuler_Buffer_t *circulerBuffer , uint8_t *data);

uint8_t Circuler_Buffer_Count(Circuler_Buffer_t *circulerBuffer);

#endif /* INC_CIRCULER_BUFFER_H_ */
