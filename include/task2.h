#ifndef TASK1
#define TASK1

#include "stdint.h"

/**
 * Create input and output typedefs, put all necessary values WITHIN the structs only.
**/
typedef struct 
{
    uint32_t coefficients[8];
} data_in_t;

typedef struct 
{
    uint32_t coefficients[8];
} data_out_t;

/**
 * Local data type definition, do not modify.
 * */
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
} local_data_t;

#endif /*TASK1*/