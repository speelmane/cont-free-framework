/* TASK 1
    Purely based on a TaclE benchmark - jfdctint
*/

#include <string.h>
#include <stdio.h>
#include "pico/platform.h"
#include "common.h"


/* 
    custom defines (e.g. from the benchmark) and type declarations
*/
#define DCTSIZE 8
#define DESCALE(x,n)  (((x) + (((int)1) << ((n)-1))) >> (n))

#define CONST_BITS  13
#define PASS1_BITS  2

#define FIX_0_298631336  ((int)  2446)  /* FIX(0.298631336) */
#define FIX_0_390180644  ((int)  3196)  /* FIX(0.390180644) */
#define FIX_0_541196100  ((int)  4433)  /* FIX(0.541196100) */
#define FIX_0_765366865  ((int)  6270)  /* FIX(0.765366865) */
#define FIX_0_899976223  ((int)  7373)  /* FIX(0.899976223) */
#define FIX_1_175875602  ((int)  9633)  /* FIX(1.175875602) */
#define FIX_1_501321110  ((int)  12299) /* FIX(1.501321110) */
#define FIX_1_847759065  ((int)  15137) /* FIX(1.847759065) */
#define FIX_1_961570560  ((int)  16069) /* FIX(1.961570560) */
#define FIX_2_053119869  ((int)  16819) /* FIX(2.053119869) */
#define FIX_2_562915447  ((int)  20995) /* FIX(2.562915447) */
#define FIX_3_072711026  ((int)  25172) /* FIX(3.072711026) */


/**
 * Create input and output typedefs, put all necessary values WITHIN the structs only.
**/
typedef struct 
{
    int checksum;
    int initial_coefficients[64];
} data_in_t;

typedef struct 
{
    int transformation_result;
    int transformed_coefficients[64];
} data_out_t;

/**
 * Local data type definition, do not modify.
 * */
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
} local_data_t;


#define __task1_runtime_copy(group) __attribute__((used, section(".task1_runtime_copy." group)))

/* Initialize data in with corresponding input values of your choice
Note: for this benchmark, since the initialization cannot use the % operator, the initial values are provided based on the operator but in the raw form
*/
static __scratch_y("task1") data_in_t data_in = {
    .initial_coefficients = {214, 28543, 60805, 26341, 30079, 2953, 65155, 15076, 39139, 28303, 28885, 40756, 46759, 58738, 13570, 35446, 61414, 41803, 54940, 32716, 25999, 50128, 48070, 36496, 4459, 3313, 47500, 26221, 14119, 42928, 7960, 10201, 46114, 38488, 7255, 47506, 27019, 54718, 3190, 31141, 13129, 42328, 59230, 13471, 22279, 14113, 42130, 32896, 49939, 22933, 35560, 11041, 26764, 20803, 14410, 16096, 43729, 48958, 23530, 49426, 20239, 4933, 820, 43606},
    .checksum = 1668124};
static __scratch_y("task1") data_out_t data_out;


void __inline_external(jfdctint_jpeg_fdct_islow)(int jfdctint_data[])
{
  int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  int tmp10, tmp11, tmp12, tmp13;
  int z1, z2, z3, z4, z5;
  int *dataptr;
  int ctr;

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true DCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  dataptr = jfdctint_data;
  _Pragma( "loopbound min 8 max 8" )
  for ( ctr = DCTSIZE - 1; ctr >= 0; ctr-- ) {

    tmp0 = dataptr[ 0 ] + dataptr[ 7 ];
    tmp7 = dataptr[ 0 ] - dataptr[ 7 ];
    tmp1 = dataptr[ 1 ] + dataptr[ 6 ];
    tmp6 = dataptr[ 1 ] - dataptr[ 6 ];
    tmp2 = dataptr[ 2 ] + dataptr[ 5 ];
    tmp5 = dataptr[ 2 ] - dataptr[ 5 ];
    tmp3 = dataptr[ 3 ] + dataptr[ 4 ];
    tmp4 = dataptr[ 3 ] - dataptr[ 4 ];

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[ 0 ] = ( int ) ( ( tmp10 + tmp11 ) << PASS1_BITS );
    dataptr[ 4 ] = ( int ) ( ( tmp10 - tmp11 ) << PASS1_BITS );

    z1 = ( tmp12 + tmp13 ) * FIX_0_541196100;
    dataptr[ 2 ] = ( int ) DESCALE( z1 + tmp13 * FIX_0_765366865,
                                    CONST_BITS - PASS1_BITS );
    dataptr[ 6 ] = ( int ) DESCALE( z1 + tmp12 * ( - FIX_1_847759065 ),
                                    CONST_BITS - PASS1_BITS );

    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = ( z3 + z4 ) * FIX_1_175875602; /* sqrt(2) * c3 */

    tmp4 = tmp4 * FIX_0_298631336; /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp5 = tmp5 * FIX_2_053119869; /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp6 = tmp6 * FIX_3_072711026; /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp7 = tmp7 * FIX_1_501321110; /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = z1 * ( - FIX_0_899976223 ); /* sqrt(2) * (c7-c3) */
    z2 = z2 * ( - FIX_2_562915447 ); /* sqrt(2) * (-c1-c3) */
    z3 = z3 * ( - FIX_1_961570560 ); /* sqrt(2) * (-c3-c5) */
    z4 = z4 * ( - FIX_0_390180644 ); /* sqrt(2) * (c5-c3) */

    z3 += z5;
    z4 += z5;

    dataptr[ 7 ] = ( int ) DESCALE( tmp4 + z1 + z3, CONST_BITS - PASS1_BITS );
    dataptr[ 5 ] = ( int ) DESCALE( tmp5 + z2 + z4, CONST_BITS - PASS1_BITS );
    dataptr[ 3 ] = ( int ) DESCALE( tmp6 + z2 + z3, CONST_BITS - PASS1_BITS );
    dataptr[ 1 ] = ( int ) DESCALE( tmp7 + z1 + z4, CONST_BITS - PASS1_BITS );

    dataptr += DCTSIZE;   /* advance pointer to next row */
  }

  dataptr = jfdctint_data;
  _Pragma( "loopbound min 8 max 8" )
  for ( ctr = DCTSIZE - 1; ctr >= 0; ctr-- ) {
    tmp0 = dataptr[ DCTSIZE * 0 ] + dataptr[ DCTSIZE * 7 ];
    tmp7 = dataptr[ DCTSIZE * 0 ] - dataptr[ DCTSIZE * 7 ];
    tmp1 = dataptr[ DCTSIZE * 1 ] + dataptr[ DCTSIZE * 6 ];
    tmp6 = dataptr[ DCTSIZE * 1 ] - dataptr[ DCTSIZE * 6 ];
    tmp2 = dataptr[ DCTSIZE * 2 ] + dataptr[ DCTSIZE * 5 ];
    tmp5 = dataptr[ DCTSIZE * 2 ] - dataptr[ DCTSIZE * 5 ];
    tmp3 = dataptr[ DCTSIZE * 3 ] + dataptr[ DCTSIZE * 4 ];
    tmp4 = dataptr[ DCTSIZE * 3 ] - dataptr[ DCTSIZE * 4 ];

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    dataptr[ DCTSIZE * 0 ] = ( int ) DESCALE( tmp10 + tmp11, PASS1_BITS );
    dataptr[ DCTSIZE * 4 ] = ( int ) DESCALE( tmp10 - tmp11, PASS1_BITS );

    z1 = ( tmp12 + tmp13 ) * FIX_0_541196100;
    dataptr[ DCTSIZE * 2 ] = ( int ) DESCALE( z1 + tmp13 * FIX_0_765366865,
                             CONST_BITS + PASS1_BITS );
    dataptr[ DCTSIZE * 6 ] = ( int ) DESCALE( z1
                             + tmp12 * ( - FIX_1_847759065 ),
                             CONST_BITS + PASS1_BITS );

    z1 = tmp4 + tmp7;
    z2 = tmp5 + tmp6;
    z3 = tmp4 + tmp6;
    z4 = tmp5 + tmp7;
    z5 = ( z3 + z4 ) * FIX_1_175875602; /* sqrt(2) * c3 */

    tmp4 = tmp4 * FIX_0_298631336; /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp5 = tmp5 * FIX_2_053119869; /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp6 = tmp6 * FIX_3_072711026; /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp7 = tmp7 * FIX_1_501321110; /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = z1 * ( - FIX_0_899976223 ); /* sqrt(2) * (c7-c3) */
    z2 = z2 * ( - FIX_2_562915447 ); /* sqrt(2) * (-c1-c3) */
    z3 = z3 * ( - FIX_1_961570560 ); /* sqrt(2) * (-c3-c5) */
    z4 = z4 * ( - FIX_0_390180644 ); /* sqrt(2) * (c5-c3) */

    z3 += z5;
    z4 += z5;

    dataptr[ DCTSIZE * 7 ] = ( int ) DESCALE( tmp4 + z1 + z3,
                             CONST_BITS + PASS1_BITS );
    dataptr[ DCTSIZE * 5 ] = ( int ) DESCALE( tmp5 + z2 + z4,
                             CONST_BITS + PASS1_BITS );
    dataptr[ DCTSIZE * 3 ] = ( int ) DESCALE( tmp6 + z2 + z3,
                             CONST_BITS + PASS1_BITS );
    dataptr[ DCTSIZE * 1 ] = ( int ) DESCALE( tmp7 + z1 + z4,
                             CONST_BITS + PASS1_BITS );

    dataptr++;      /* advance pointer to next column */
  }

}

/* Place an attribute to note that this is an exec function */
void __task1_runtime_copy("task1")(task1_E)(local_data_t * local_data)
{
    /* Init routine modified since a modulo operator was used, data simply copy pasted to local data */
    int jfdctint_data[ 64 ];

    for (int i = 0; i < 64; i++ ) {
        jfdctint_data[ i ] = local_data->local_data_in.initial_coefficients[ i ];
    }

    /* Main entrypoint */

    jfdctint_jpeg_fdct_islow(jfdctint_data);

      int checksum = 0;
    int i;
    _Pragma( "loopbound min 64 max 64" )
    for ( i = 0; i < 64; ++i )
    {
        checksum += jfdctint_data[ i ];
        local_data->local_data_out.transformed_coefficients[ i ] = jfdctint_data[ i ];
    }
    local_data->local_data_out.transformation_result = ( ( checksum == local_data->local_data_in.checksum ) ? 0 : -1 );
}

/* This will execute from the flash, except for the task_exec function
*  Future implementation: subschedule the phases - 
*  DELAY between task_read and task_exec functionality 
*  subschedule.sleep_func(subschedule.r_to_e_wait_time)
*/
void task1(subschedule_t subschedule) //add relative waiting times as a parameter here
{


    #ifdef DEBUG
        printf("Task 1 entered \n");
    #endif

    #ifdef TIMESTAMP
        uint64_t timestamp_READ = subschedule.timestamp_func();
    #endif

    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    extern char __task1_runtime_copy_start__[],  __task1_runtime_copy_end__[];

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));

    int func_size = (int) (__task1_runtime_copy_end__) - (int)(__task1_runtime_copy_start__);

    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task1_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but code must execute from an odd address (little endian)

    #ifdef DEBUG
        printf("Task1 Runtime copy start: %p\n", __task1_runtime_copy_start__);
        printf("Task1 Runtime copy end: %p\n", __task1_runtime_copy_end__);
        printf("Task1 Core end used: %p\n", subschedule.exec_copy_func_dst);
        printf("Task1 Data in (should be SRAM_5) address: %p\n", &data_in);
        printf("Task1 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task1 Local data OUT address: %p\n", &local_data.local_data_out);
        printf("Task1 copied func pointer: %p of size: %x\n", exec_copy_func, func_size);
    #endif

    /* End of Init + read routine */

    #ifdef TIMESTAMP
        uint64_t timestamp_EXECUTE = subschedule.timestamp_func();
    #endif

    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);
    /* End of Exec routine */

    #ifdef TIMESTAMP
        uint64_t timestamp_WRITE = subschedule.timestamp_func();
    #endif

    /* Write routine*/
    memcpy(&data_out, &local_data.local_data_out, sizeof(data_out));
    /* End of Write routine*/


    #ifdef TIMESTAMP
        uint64_t timestamp_PASS = subschedule.timestamp_func();
        printf("\n\nCORE %d, T1\nRead: %lli, execute: %lli, write: %lli, pass: %lli\n", subschedule.cpu_id, timestamp_READ, timestamp_EXECUTE, timestamp_WRITE, timestamp_PASS);
        printf("Return value: %d\n", data_out.transformation_result);
    #endif

    /* End of Write routine and end of task job, return to the scheduler */
}
