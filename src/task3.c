/* TASK 3
    Purely based on a TaclE benchmark - bsort
*/

#include <string.h>
#include <stdio.h>
// subschedule and function prototype location
#include "common.h" 

/*
    Additional type declarations
*/

// empty

/*
    Define data in /data out data types, structs can also be empty (size 0).
*/
typedef struct
{
} data_in_t;

typedef struct 
{
   int sorted;
} data_out_t;

/*
    Local data type definition.
    DO NOT MODIFY!
*/
typedef struct 
{
    data_in_t local_data_in;
    data_out_t local_data_out;
} local_data_t;


#define __task3_runtime_copy(group) __attribute__((used, section(".task3_runtime_copy." group)))
#define __inline_external __attribute__((always_inline))
#define DEBUG 0

/* 
    custom defines (e.g. from the benchmark)
*/
#define bsort_SIZE 100

/* 
    Initialize global variables, initialization and a !!const!! modifier MANDATORY!
    If none used, keep a:
    static const data_in_t data_in = {};
*/
static const data_in_t data_in = {};

/*
    Place the attribute to note that this is an exec function
*/
int __inline_external(bsort_return)( int bsort_Array[] )
{
  int Sorted = 1;
  int Index;

  _Pragma( "loopbound min 99 max 99" )
  for ( Index = 0; Index < bsort_SIZE - 1; Index ++ )
    Sorted = Sorted && ( bsort_Array[ Index ] < bsort_Array[ Index + 1 ] );

  return 1 - Sorted;
}

int __inline_external(bsort_Initialize)( int Array[] )
{
  int Index;

  _Pragma( "loopbound min 100 max 100" )
  for ( Index = 0; Index < bsort_SIZE; Index ++ )
    Array[ Index ] = ( Index + 1 ) * -1;

  return 0;
}

int __inline_external(bsort_BubbleSort)( int Array[] )
{
  int Sorted = 0;
  int Temp, Index, i;

  _Pragma( "loopbound min 99 max 99" )
  for ( i = 0; i < bsort_SIZE - 1; i ++ ) {
    Sorted = 1;
    _Pragma( "loopbound min 3 max 99" )
    for ( Index = 0; Index < bsort_SIZE - 1; Index ++ ) {
      if ( Index > bsort_SIZE - i )
        break;
      if ( Array[ Index ] > Array[Index + 1] ) {
        Temp = Array[ Index ];
        Array[ Index ] = Array[ Index + 1 ];
        Array[ Index + 1 ] = Temp;
        Sorted = 0;
      }
    }

    if ( Sorted )
      break;
  }

  return 0;
}

void __task3_runtime_copy("task3")(task3_E)(local_data_t * local_data)
{
    int bsort_Array[ bsort_SIZE ];

    bsort_Initialize(bsort_Array);

    bsort_BubbleSort(bsort_Array);

    int sorted_int = bsort_return(bsort_Array); // returns 0 if sorted correctly

    local_data->local_data_out.sorted = sorted_int;
}

/*
    This will execute from the flash, except for the task_exec function
*/
void task3(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    #ifdef DEBUG
        printf("Task 2 entered \n");

        uint64_t timestamp_before = subschedule.timestamp_func();

    #endif


    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    #ifdef DEBUG
        printf("Task3 Data in (should be flash) address: %p, size: %d\n", &data_in, sizeof(data_in));
        printf("Task3 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task3 Local data OUT address: %p\n", &local_data.local_data_out);
    #endif

    extern char __task3_runtime_copy_start__[],  __task3_runtime_copy_end__[];

    #ifdef DEBUG
        printf("Task3 Runtime copy start: %p\n", __task3_runtime_copy_start__);
        printf("Task3 Runtime copy end: %p\n", __task3_runtime_copy_end__);
        printf("Task3 Core end used: %p\n", subschedule.exec_copy_func_dst);
    #endif

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));

    int func_size = (int) (__task3_runtime_copy_end__) - (int)(__task3_runtime_copy_start__);

    #ifdef DEBUG
        printf("task3 exec Size: %x\n", func_size);
    #endif


    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task3_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        printf("task3 Copied func pointer: %p\n", exec_copy_func);

        uint64_t timestamp_after = subschedule.timestamp_func();

        printf("task3 Setup timestamp before: %lli, after: %lli\n", timestamp_before, timestamp_after);
    #endif


    /* End of Init + read routine */


    /* DELAY between task_read and task_exec functionality */
    subschedule.sleep_func(subschedule.r_to_e_wait_time);


    /* Exec routine (RAM)*/
    exec_copy_func(&local_data);

    /* End of Exec routine */

    subschedule.sleep_func(subschedule.e_to_w_wait_time);
   
    #ifdef DEBUG
        printf("task3 Flash routine here again\n");
    #endif

    /* Write routine (FLASH)*/

    /* TMP!! 
    * SOME PRINTS HERE TO CHECK VALUES ? */
    #ifdef DEBUG
        printf("task3 Local data out sorted is: %d\n", local_data.local_data_out.sorted);
    #endif


    /* End of Write routine and end of task job, return to the scheduler */
}