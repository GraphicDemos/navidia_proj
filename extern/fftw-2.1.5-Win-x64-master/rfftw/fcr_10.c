/*
 * Copyright (c) 1997-1999, 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Mon Mar 24 02:07:03 EST 2003 */

#include "fftw-int.h"
#include "fftw.h"

/* Generated by: /homee/stevenj/cvs/fftw/gensrc/genfft -magic-alignment-check -magic-twiddle-load-all -magic-variables 4 -magic-loopi -hc2real 10 */

/*
 * This function contains 34 FP additions, 14 FP multiplications,
 * (or, 26 additions, 6 multiplications, 8 fused multiply/add),
 * 20 stack variables, and 20 memory accesses
 */
static const fftw_real K500000000 =
FFTW_KONST(+0.500000000000000000000000000000000000000000000);
static const fftw_real K1_902113032 =
FFTW_KONST(+1.902113032590307144232878666758764286811397268);
static const fftw_real K1_175570504 =
FFTW_KONST(+1.175570504584946258337411909278145537195304875);
static const fftw_real K2_000000000 =
FFTW_KONST(+2.000000000000000000000000000000000000000000000);
static const fftw_real K1_118033988 =
FFTW_KONST(+1.118033988749894848204586834365638117720309180);

/*
 * Generator Id's : 
 * $Id: exprdag.ml,v 1.43 2003/03/16 23:43:46 stevenj Exp $
 * $Id: fft.ml,v 1.44 2003/03/16 23:43:46 stevenj Exp $
 * $Id: to_c.ml,v 1.26 2003/03/16 23:43:46 stevenj Exp $
 */

void fftw_hc2real_10(const fftw_real *real_input,
		     const fftw_real *imag_input, fftw_real *output,
		     int real_istride, int imag_istride, int ostride)
{
     fftw_real tmp3;
     fftw_real tmp11;
     fftw_real tmp23;
     fftw_real tmp31;
     fftw_real tmp20;
     fftw_real tmp30;
     fftw_real tmp10;
     fftw_real tmp28;
     fftw_real tmp14;
     fftw_real tmp16;
     fftw_real tmp18;
     fftw_real tmp19;
     ASSERT_ALIGNED_DOUBLE;
     {
	  fftw_real tmp1;
	  fftw_real tmp2;
	  fftw_real tmp21;
	  fftw_real tmp22;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp1 = real_input[0];
	  tmp2 = real_input[5 * real_istride];
	  tmp3 = tmp1 - tmp2;
	  tmp11 = tmp1 + tmp2;
	  tmp21 = imag_input[4 * imag_istride];
	  tmp22 = imag_input[imag_istride];
	  tmp23 = tmp21 - tmp22;
	  tmp31 = tmp21 + tmp22;
     }
     tmp18 = imag_input[2 * imag_istride];
     tmp19 = imag_input[3 * imag_istride];
     tmp20 = tmp18 - tmp19;
     tmp30 = tmp18 + tmp19;
     {
	  fftw_real tmp6;
	  fftw_real tmp12;
	  fftw_real tmp9;
	  fftw_real tmp13;
	  ASSERT_ALIGNED_DOUBLE;
	  {
	       fftw_real tmp4;
	       fftw_real tmp5;
	       fftw_real tmp7;
	       fftw_real tmp8;
	       ASSERT_ALIGNED_DOUBLE;
	       tmp4 = real_input[2 * real_istride];
	       tmp5 = real_input[3 * real_istride];
	       tmp6 = tmp4 - tmp5;
	       tmp12 = tmp4 + tmp5;
	       tmp7 = real_input[4 * real_istride];
	       tmp8 = real_input[real_istride];
	       tmp9 = tmp7 - tmp8;
	       tmp13 = tmp7 + tmp8;
	  }
	  tmp10 = tmp6 + tmp9;
	  tmp28 = K1_118033988 * (tmp6 - tmp9);
	  tmp14 = tmp12 + tmp13;
	  tmp16 = K1_118033988 * (tmp12 - tmp13);
     }
     output[5 * ostride] = tmp3 + (K2_000000000 * tmp10);
     {
	  fftw_real tmp32;
	  fftw_real tmp34;
	  fftw_real tmp29;
	  fftw_real tmp33;
	  fftw_real tmp27;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp32 = (K1_175570504 * tmp30) - (K1_902113032 * tmp31);
	  tmp34 = (K1_902113032 * tmp30) + (K1_175570504 * tmp31);
	  tmp27 = tmp3 - (K500000000 * tmp10);
	  tmp29 = tmp27 - tmp28;
	  tmp33 = tmp28 + tmp27;
	  output[7 * ostride] = tmp29 - tmp32;
	  output[3 * ostride] = tmp29 + tmp32;
	  output[ostride] = tmp33 - tmp34;
	  output[9 * ostride] = tmp33 + tmp34;
     }
     output[0] = tmp11 + (K2_000000000 * tmp14);
     {
	  fftw_real tmp24;
	  fftw_real tmp26;
	  fftw_real tmp17;
	  fftw_real tmp25;
	  fftw_real tmp15;
	  ASSERT_ALIGNED_DOUBLE;
	  tmp24 = (K1_175570504 * tmp20) - (K1_902113032 * tmp23);
	  tmp26 = (K1_902113032 * tmp20) + (K1_175570504 * tmp23);
	  tmp15 = tmp11 - (K500000000 * tmp14);
	  tmp17 = tmp15 - tmp16;
	  tmp25 = tmp16 + tmp15;
	  output[2 * ostride] = tmp17 - tmp24;
	  output[8 * ostride] = tmp17 + tmp24;
	  output[6 * ostride] = tmp25 - tmp26;
	  output[4 * ostride] = tmp25 + tmp26;
     }
}

fftw_codelet_desc fftw_hc2real_10_desc = {
     "fftw_hc2real_10",
     (void (*)()) fftw_hc2real_10,
     10,
     FFTW_BACKWARD,
     FFTW_HC2REAL,
     235,
     0,
     (const int *) 0,
};
