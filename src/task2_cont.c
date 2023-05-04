#include <string.h>
#include <stdio.h>
#include "pico/platform.h"
#include "common.h"

void insertsort_initialize( unsigned int *array );
void insertsort_init( void );
int insertsort_return( void );
void insertsort_main( void );
int main( void );

unsigned int __core_0_data("data") insertsort_a[ 11 ];
int __core_0_data("data") insertsort_iters_i, __core_0_data("data") insertsort_min_i, __core_0_data("data") insertsort_max_i;
int __core_0_data("data") insertsort_iters_a, __core_0_data("data") insertsort_min_a, __core_0_data("data") insertsort_max_a;

__core_0_code("code") void insertsort_initialize( unsigned int *array )
{

  register volatile int i;
  _Pragma( "loopbound min 11 max 11" )
  for ( i = 0; i < 11; i++ )
    insertsort_a[ i ] = array[ i ];

}


__core_0_code("code") void insertsort_init()
{
  unsigned int a[ 11 ] = {0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};

  insertsort_iters_i = 0;
  insertsort_min_i = 100000;
  insertsort_max_i = 0;
  insertsort_iters_a = 0;
  insertsort_min_a = 100000;
  insertsort_max_a = 0;

  insertsort_initialize( a );
}

__core_0_code("code") int insertsort_return()
{
  int i, returnValue = 0;

  _Pragma( "loopbound min 11 max 11" )
  for ( i = 0; i < 11; i++ )
    returnValue += insertsort_a[ i ];

  return ( returnValue + ( -65 ) ) != 0;
}

__core_0_code("code") void _Pragma( "entrypoint" ) insertsort_main()
{
  int  i, j, temp;
  i = 2;

  insertsort_iters_i = 0;

  _Pragma( "loopbound min 9 max 9" )
  while ( i <= 10 ) {

    insertsort_iters_i++;

    j = i;

    insertsort_iters_a = 0;

    _Pragma( "loopbound min 1 max 9" )
    while ( insertsort_a[ j ] < insertsort_a[ j - 1 ] ) {
      insertsort_iters_a++;

      temp = insertsort_a[ j ];
      insertsort_a[ j ] = insertsort_a[ j - 1 ];
      insertsort_a[ j - 1 ] = temp;
      j--;
    }

    if ( insertsort_iters_a < insertsort_min_a )
      insertsort_min_a = insertsort_iters_a;
    if ( insertsort_iters_a > insertsort_max_a )
      insertsort_max_a = insertsort_iters_a;

    i++;
  }

  if ( insertsort_iters_i < insertsort_min_i )
    insertsort_min_i = insertsort_iters_i;
  if ( insertsort_iters_i > insertsort_max_i )
    insertsort_max_i = insertsort_iters_i;
}

__core_0_code("code") void task2(subschedule_t subschedule)
{
  insertsort_init();
  insertsort_main();
  int return_value = ( insertsort_return() );
}
