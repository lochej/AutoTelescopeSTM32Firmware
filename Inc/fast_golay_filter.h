/*
 * fast_golay_filter.h
 *
 *  Created on: Aug 2, 2018
 *      Author: JLH
 */

#ifndef FAST_GOLAY_FILTER_H_
#define FAST_GOLAY_FILTER_H_

#include "stdint.h"

#define MAX_GOLAY_BUFFER_SIZE 65536
#define GOLAY_BUFFER_STANDARD_SIZE (0x0001<<6) // 2^6 standard size is 64 samples


#define GOLAY_FILTER_INIT(struct_golay,_size) (struct_golay.size=(_size));\
												(struct_golay.insert_idx=0);

//DOUBLE
struct Golay_Filter_Fast_double
{
	uint16_t size;
	uint16_t insert_idx;
	double Buffer_Data[GOLAY_BUFFER_STANDARD_SIZE];
};

typedef struct Golay_Filter_Fast_double Golay_Filter_Fast_double;
//end DOUBLE


//FLOAT
struct Golay_Filter_Fast_float
{
	uint16_t size;
	uint16_t insert_idx;
	float Buffer_Data[GOLAY_BUFFER_STANDARD_SIZE];
};

typedef struct Golay_Filter_Fast_float Golay_Filter_Fast_float;
//end FLOAT


//I32
struct Golay_Filter_Fast_i32
{
	uint16_t size; //Must be power of 2
	uint16_t insert_idx;
	int32_t Buffer_Data[GOLAY_BUFFER_STANDARD_SIZE];
};

typedef struct Golay_Filter_Fast_i32 Golay_Filter_Fast_i32;
//end I32

//UI32
struct Golay_Filter_Fast_ui32
{
	uint16_t size; //Must be power of 2
	uint16_t insert_idx;
	uint32_t Buffer_Data[GOLAY_BUFFER_STANDARD_SIZE];
};

typedef struct Golay_Filter_Fast_ui32 Golay_Filter_Fast_ui32;
//end UI32


//I16
struct Golay_Filter_Fast_i16
{
	uint16_t size; //Must be power of 2
	uint16_t insert_idx;
	int16_t Buffer_Data[GOLAY_BUFFER_STANDARD_SIZE];
};

typedef struct Golay_Filter_Fast_i16 Golay_Filter_Fast_i16;
//end I16


//UI16
struct Golay_Filter_Fast_ui16
{
	uint16_t size; //Must be power of 2
	uint16_t insert_idx;
	uint16_t Buffer_Data[GOLAY_BUFFER_STANDARD_SIZE];
};

typedef struct Golay_Filter_Fast_ui16 Golay_Filter_Fast_ui16;
//end UI16


int32_t Golay_apply_filtering_i32(Golay_Filter_Fast_i32 * filter,int32_t value);

int16_t Golay_apply_filtering_i16(Golay_Filter_Fast_i16 * filter,int16_t value);

uint32_t Golay_apply_filtering_ui32(Golay_Filter_Fast_ui32 * filter,uint32_t value);

uint16_t Golay_apply_filtering_ui16(Golay_Filter_Fast_ui16 * filter,uint16_t value);

float Golay_apply_filtering_f(Golay_Filter_Fast_float * filter,float value);

double Golay_apply_filtering_d(Golay_Filter_Fast_double * filter,double value);


#endif /* FAST_GOLAY_FILTER_H_ */
