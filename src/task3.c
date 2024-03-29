/* TASK 3
    Purely based on a TaclE benchmark - bsort
*/

#include <string.h>
#include <stdio.h>
// subschedule and function prototype location
#include "common.h" 
#include "pico/platform.h"


/* 
    custom defines (e.g. from the benchmark) and type declarations
*/
#define bsort_SIZE 100


/*
    Define data in /data out data types, structs can also be empty (size 0).
*/
typedef struct
{
} data_in_t;

typedef struct 
{
  int bsort_Array[ bsort_SIZE ];
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
    char volatile * write_release_flag;

} local_data_t;


#define __task3_runtime_copy(group) __attribute__((used, section(".task3_runtime_copy." group)))

/* 
    Initialize data input to necessary values (independent tasks)
    If none used, keep a:
    static const data_in_t data_in = {};
*/
static __scratch_y("task3") data_in_t data_in = {};
static __scratch_y("task3") data_out_t data_out;

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

  _Pragma( "loopbound min 100 max 100" )
  for (int i = 0; i< bsort_SIZE; i++)
  {
    local_data->local_data_out.bsort_Array[i] = bsort_Array[i];
  }

  // MANDATORY ROUTINE IN ALL TASKS 
  while(!(*(local_data->write_release_flag))); // wait for the write flag to be released
  *(local_data->write_release_flag) = 0; // reset the write flag
}

/* This will execute from the flash, except for the task_exec function
*  Future implementation: subschedule the phases - 
*  DELAY between task_read and task_exec functionality 
*  subschedule.sleep_func(subschedule.r_to_e_wait_time)
*/
void task3(subschedule_t subschedule) //add relative waiting times as a parameter here
{
    #ifdef DEBUG
        printf("Task 3 entered \n");
        uint64_t timestamp_before_read_phase = subschedule.timestamp_func();
    #endif

    #ifdef TIMESTAMP
        uint64_t timestamp_READ = subschedule.timestamp_func();
    #endif

    /* Init + read routine (FLASH) */
    local_data_t local_data;
    void (*exec_copy_func)(local_data_t * local_data);

    extern char __task3_runtime_copy_start__[],  __task3_runtime_copy_end__[];

    /* Perform memcpy on data and code */
    memcpy(&local_data.local_data_in, &data_in, sizeof(data_in));
    local_data.write_release_flag = subschedule.write_release_flag;

    int func_size = (int) (__task3_runtime_copy_end__) - (int)(__task3_runtime_copy_start__);

    /* If multiple functions are called, multiple can be copied but destination location for following functions must be adjusted according to the size of the previous function */
    exec_copy_func = (memcpy(subschedule.exec_copy_func_dst, __task3_runtime_copy_start__, func_size) + 1); // note the +1 because the return address is even but the function must execue from an odd address (little endian)

    #ifdef DEBUG
        uint64_t timestamp_after_read_phase = subschedule.timestamp_func();
        printf("Task3 copied func pointer: %p of size: %x\n", exec_copy_func, func_size);
        printf("Task3 Runtime copy start: %p\n", __task3_runtime_copy_start__);
        printf("Task3 Runtime copy end: %p\n", __task3_runtime_copy_end__);
        printf("Task3 Core end used: %p\n", subschedule.exec_copy_func_dst);
        printf("Task3 Data in (should be SRAM_5) address, size: %d: %p\n", &data_in, sizeof(data_in));
        printf("Task3 Data out (should be SRAM_5) address, size: %d: %p\n", &data_in, sizeof(data_out));
        printf("Task3 Local data IN address: %p\n", &local_data.local_data_in);
        printf("Task3 Local data OUT address: %p\n", &local_data.local_data_out);
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
        printf("\n\nCORE %d, T3\nRead: %lli, execute: %lli, write: %lli, pass: %lli\n", subschedule.cpu_id, timestamp_READ, timestamp_EXECUTE, timestamp_WRITE, timestamp_PASS);
        printf("Return value: %d\n", data_out.sorted);
        
    #endif

    /* End of Write routine and end of task job, return to the scheduler */
}