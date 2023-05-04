#include <string.h>
#include <stdio.h>
#include "pico/platform.h"
#include "common.h"

#define DCTSIZE 8
#define DESCALE(x,n)  (((x) + (((int)1) << ((n)-1))) >> (n))

void jfdctint_init();
int jfdctint_return();
void jfdctint_main();
int main( void );


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


int jfdctint_data[ 64 ];


const int __core_0_data("data") jfdctint_CHECKSUM = 1668124;

__core_0_code("code") void jfdctint_init()
{
  int i, seed;

  /* Worst case settings */
  /* Set array to random values */
  seed = 1;

  _Pragma( "loopbound min 64 max 64" )
  for ( i = 0; i < 64; i++ ) {
    seed = ( ( seed * 133 ) + 81 ) % 65535;
    jfdctint_data[ i ] = seed;
  }

}

__core_0_code("code") int jfdctint_return()
{
  int checksum = 0;
  int i;
  _Pragma( "loopbound min 64 max 64" )
  for ( i = 0; i < 64; ++i )
    checksum += jfdctint_data[ i ];
  return ( ( checksum == jfdctint_CHECKSUM ) ? 0 : -1 );
}

__core_0_code("code") void jfdctint_jpeg_fdct_islow( void )
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

__core_0_code("code") void _Pragma ( "entrypoint" ) jfdctint_main( void )
{
  jfdctint_jpeg_fdct_islow();
}


__core_0_code("code") void task1(subschedule_t subschedule)
{
  jfdctint_init();
  jfdctint_main();

  int return_value = ( jfdctint_return() );
}