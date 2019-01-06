/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Media\programs\D3D9_WaterInteraction\
File:  WaterInteractionConstants.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

'
Comments:

-------------------------------------------------------------------------------|--------------------*/

#define PCN_MULTFACTOR_1		 0
#define PCR_MULTFACTOR_1		c0
#define PCN_MULTFACTOR_2         1
#define PCR_MULTFACTOR_2		c1
#define PCN_EQ_REST_FAC          2
#define PCR_EQ_REST_FAC			c2

		// similar to the red & green masks for normal map creation, this
		// value scales the du and dv offsets in creating an embm map
#define	PCN_EMBM_SCALE			 3
#define PCR_EMBM_SCALE			c3

		// For CA_Water2, PCN_POSITIONMASK, which would ordinarily use values
		// of 1 or 0, uses values slightly less than 1.0 in order to dampen
		// the oscillations of the position.  This dissipates energy from 
		// the system so that it does not run away to saturate the components
#define PCN_POSITIONMASK		 4
#define PCR_POSITIONMASK		c4

#define PCN_RED_MASK             5
#define PCR_RED_MASK			c5
#define PCN_GREEN_MASK           6
#define PCR_GREEN_MASK			c6

//-----------------------------------------------------------------------------

#define CV_ZERO					0
#define CV_ONE					1

#define CV_WORLDVIEWPROJ_0		2
#define CV_WORLDVIEWPROJ_1		3
#define CV_WORLDVIEWPROJ_2		4
#define CV_WORLDVIEWPROJ_3		5

#define CV_EYE_OBJSPC			6

#define CV_BUMPSCALE			7




#define	CV_CONSTS_1				29
#define CV_CONSTS_2				30

#define CV_ONOFF_1				31	


#define CV_WORLD_0				32
#define CV_WORLD_1				33
#define CV_WORLD_2				34



#define CV_TEXCOORD_BASE		35

#define CV_TILE_SIZE			36

#define CV_BASISTRANSFORM_0		37
#define CV_BASISTRANSFORM_1		38
#define CV_BASISTRANSFORM_2		39


#define CV_CALC_SXT             51

#define CV_DISPSCALE			52

// for scaling embm offsets overall
#define CV_OFFSETSCALE			53


#define CV_PTS_BASE				54
//	CV_PTS_BASE					55
//	CV_PTS_BASE					56
//	CV_PTS_BASE					57


// These must be 4 consecutive constants
//  starting with CV_T0_OFFSET
#define CV_T0_OFFSET			60
#define CV_T1_OFFSET			61
#define CV_T2_OFFSET			62
#define CV_T3_OFFSET			63

