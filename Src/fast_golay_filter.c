/*
 * fast_golay_filter.c
 *
 *  Created on: Aug 2, 2018
 *      Author: JLH
 */
#include "stdint.h"
#include "fast_golay_filter.h"

/*//official algorithm
  int mean=value;
	int i=filter->size;

	//Sum everything in the buffer
	while(i-->0)
	{
		mean+=filter->Buffer_Data[i];
	}

	filter->insert_idx = (filter->insert_idx +1)%filter->size;

	filter->Buffer_Data[filter->insert_idx] = value;

	return mean/(filter->size +1);
 */


#define BUFFER_INSERT_NOT_POWER_OF_TWO(filter,value) (filter->insert_idx = (filter->insert_idx +1)%filter->size);\
		(filter->Buffer_Data[filter->insert_idx] = value);

#define BUFFER_INSERT_POWER_OF_TWO(filter,value) (filter->insert_idx = (filter->insert_idx +1)&(filter->size-1));\
		(filter->Buffer_Data[filter->insert_idx] = value);

//If you are only going to use power of two size of buffer, then use the appropriate macro to operate the ring buffer
#define BUFFER_INSERT(filter,value) BUFFER_INSERT_POWER_OF_TWO(filter,value)


int32_t Golay_apply_filtering_i32(Golay_Filter_Fast_i32 * filter,int32_t value)
{
	int32_t mean=value;

	uint16_t i=filter->size;

	while( i-- >0)
	{
		mean+=filter->Buffer_Data[i];
	}

	BUFFER_INSERT(filter,value)

	return mean/(filter->size+1);
}

int16_t Golay_apply_filtering_i16(Golay_Filter_Fast_i16 * filter,int16_t value)
{
	int32_t mean=value;

	uint16_t i=filter->size;

	while( i-- >0)
	{
		mean+=filter->Buffer_Data[i];
	}

	BUFFER_INSERT(filter,value)

	return (int16_t) mean/(filter->size+1);
}

uint32_t Golay_apply_filtering_ui32(Golay_Filter_Fast_ui32 * filter,uint32_t value)
{
	uint32_t mean=value;

	uint16_t i=filter->size;

	while( i-- >0)
	{
		mean+=filter->Buffer_Data[i];
	}

	BUFFER_INSERT(filter,value)

	return mean/(filter->size+1);
}

uint16_t Golay_apply_filtering_ui16(Golay_Filter_Fast_ui16 * filter,uint16_t value)
{
	uint32_t mean=value;

	uint16_t i=filter->size;

	while( i-- >0)
	{
		mean+=filter->Buffer_Data[i];
	}

	BUFFER_INSERT(filter,value)

	return mean/(filter->size+1);
}

float Golay_apply_filtering_f(Golay_Filter_Fast_float * filter,float value)
{
	float mean=value;

	uint16_t i=filter->size;

	while( i-- >0)
	{
		mean+=filter->Buffer_Data[i];
	}

	BUFFER_INSERT(filter,value)

	return mean/(filter->size+1);
}

double Golay_apply_filtering_d(Golay_Filter_Fast_double * filter,double value)
{
	double mean=value;

	uint16_t i=filter->size;

	while( i-- >0)
	{
		mean+=filter->Buffer_Data[i];
	}

	BUFFER_INSERT(filter,value)

	return mean/(filter->size+1);
}




