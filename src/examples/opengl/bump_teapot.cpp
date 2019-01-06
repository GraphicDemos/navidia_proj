#include <glh/glh_extensions.h>
#include "bump_teapot.h"

#if defined(WIN32)
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#endif


/* Copyright (c) Mark J. Kilgard, 1994, 2001. */

/**
(c) Copyright 1993, Silicon Graphics, Inc.

ALL RIGHTS RESERVED

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that
both the copyright notice and this permission notice appear in
supporting documentation, and that the name of Silicon
Graphics, Inc. not be used in advertising or publicity
pertaining to distribution of the software without specific,
written prior permission.

THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU
"AS-IS" AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR
OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  IN NO
EVENT SHALL SILICON GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE
ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE,
SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR
NOT SILICON GRAPHICS, INC.  HAS BEEN ADVISED OF THE POSSIBILITY
OF SUCH LOSS, HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
ARISING OUT OF OR IN CONNECTION WITH THE POSSESSION, USE OR
PERFORMANCE OF THIS SOFTWARE.

US Government Users Restricted Rights

Use, duplication, or disclosure by the Government is subject to
restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
(c)(1)(ii) of the Rights in Technical Data and Computer
Software clause at DFARS 252.227-7013 and/or in similar or
successor clauses in the FAR or the DOD or NASA FAR
Supplement.  Unpublished-- rights reserved under the copyright
laws of the United States.  Contractor/manufacturer is Silicon
Graphics, Inc., 2011 N.  Shoreline Blvd., Mountain View, CA
94039-7311.

OpenGL(TM) is a trademark of Silicon Graphics, Inc.
*/


/* Rim, body, lid, and bottom data must be reflected in x and
   y; handle and spout data across the y axis only.  */

static const int patchdata[][16] =
{
    /* rim */
  {102, 103, 104, 105, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15},
    /* body */
  {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27},
  {24, 25, 26, 27, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40},
    /* lid */
  {96, 96, 96, 96, 97, 98, 99, 100, 101, 101, 101,
    101, 0, 1, 2, 3,},
  {0, 1, 2, 3, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 115, 116, 117},
    /* bottom */
  {118, 118, 118, 118, 124, 122, 119, 121, 123, 126,
    125, 120, 40, 39, 38, 37},
    /* handle */
  {41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    53, 54, 55, 56},
  {53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    28, 65, 66, 67},
    /* spout */
  {68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83},
  {80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
    92, 93, 94, 95}
};
/* *INDENT-OFF* */

static const float cpdata[][3] =
{
    {0.2, 0, 2.7}, {0.2, -0.112, 2.7}, {0.112, -0.2, 2.7}, {0,
    -0.2, 2.7}, {1.3375, 0, 2.53125}, {1.3375, -0.749, 2.53125},
    {0.749, -1.3375, 2.53125}, {0, -1.3375, 2.53125}, {1.4375,
    0, 2.53125}, {1.4375, -0.805, 2.53125}, {0.805, -1.4375,
    2.53125}, {0, -1.4375, 2.53125}, {1.5, 0, 2.4}, {1.5, -0.84,
    2.4}, {0.84, -1.5, 2.4}, {0, -1.5, 2.4}, {1.75, 0, 1.875},
    {1.75, -0.98, 1.875}, {0.98, -1.75, 1.875}, {0, -1.75,
    1.875}, {2, 0, 1.35}, {2, -1.12, 1.35}, {1.12, -2, 1.35},
    {0, -2, 1.35}, {2, 0, 0.9}, {2, -1.12, 0.9}, {1.12, -2,
    0.9}, {0, -2, 0.9}, {-2, 0, 0.9}, {2, 0, 0.45}, {2, -1.12,
    0.45}, {1.12, -2, 0.45}, {0, -2, 0.45}, {1.5, 0, 0.225},
    {1.5, -0.84, 0.225}, {0.84, -1.5, 0.225}, {0, -1.5, 0.225},
    {1.5, 0, 0.15}, {1.5, -0.84, 0.15}, {0.84, -1.5, 0.15}, {0,
    -1.5, 0.15}, {-1.6, 0, 2.025}, {-1.6, -0.3, 2.025}, {-1.5,
    -0.3, 2.25}, {-1.5, 0, 2.25}, {-2.3, 0, 2.025}, {-2.3, -0.3,
    2.025}, {-2.5, -0.3, 2.25}, {-2.5, 0, 2.25}, {-2.7, 0,
    2.025}, {-2.7, -0.3, 2.025}, {-3, -0.3, 2.25}, {-3, 0,
    2.25}, {-2.7, 0, 1.8}, {-2.7, -0.3, 1.8}, {-3, -0.3, 1.8},
    {-3, 0, 1.8}, {-2.7, 0, 1.575}, {-2.7, -0.3, 1.575}, {-3,
    -0.3, 1.35}, {-3, 0, 1.35}, {-2.5, 0, 1.125}, {-2.5, -0.3,
    1.125}, {-2.65, -0.3, 0.9375}, {-2.65, 0, 0.9375}, {-2,
    -0.3, 0.9}, {-1.9, -0.3, 0.6}, {-1.9, 0, 0.6}, {1.7, 0,
    1.425}, {1.7, -0.66, 1.425}, {1.7, -0.66, 0.6}, {1.7, 0,
    0.6}, {2.6, 0, 1.425}, {2.6, -0.66, 1.425}, {3.1, -0.66,
    0.825}, {3.1, 0, 0.825}, {2.3, 0, 2.1}, {2.3, -0.25, 2.1},
    {2.4, -0.25, 2.025}, {2.4, 0, 2.025}, {2.7, 0, 2.4}, {2.7,
    -0.25, 2.4}, {3.3, -0.25, 2.4}, {3.3, 0, 2.4}, {2.8, 0,
    2.475}, {2.8, -0.25, 2.475}, {3.525, -0.25, 2.49375},
    {3.525, 0, 2.49375}, {2.9, 0, 2.475}, {2.9, -0.15, 2.475},
    {3.45, -0.15, 2.5125}, {3.45, 0, 2.5125}, {2.8, 0, 2.4},
    {2.8, -0.15, 2.4}, {3.2, -0.15, 2.4}, {3.2, 0, 2.4}, {0, 0,
    3.15}, {0.8, 0, 3.15}, {0.8, -0.45, 3.15}, {0.45, -0.8,
    3.15}, {0, -0.8, 3.15}, {0, 0, 2.85}, {1.4, 0, 2.4}, {1.4,
    -0.784, 2.4}, {0.784, -1.4, 2.4}, {0, -1.4, 2.4}, {0.4, 0,
    2.55}, {0.4, -0.224, 2.55}, {0.224, -0.4, 2.55}, {0, -0.4,
    2.55}, {1.3, 0, 2.55}, {1.3, -0.728, 2.55}, {0.728, -1.3,
    2.55}, {0, -1.3, 2.55}, {1.3, 0, 2.4}, {1.3, -0.728, 2.4},
    {0.728, -1.3, 2.4}, {0, -1.3, 2.4}, {0, 0, 0}, {1.425,
    -0.798, 0}, {1.5, 0, 0.075}, {1.425, 0, 0}, {0.798, -1.425,
    0}, {0, -1.5, 0.075}, {0, -1.425, 0}, {1.5, -0.84, 0.075},
    {0.84, -1.5, 0.075}
};

	
struct patch_indexer_teapot
{
	int u_order, v_order;
	GLfloat *p;
};

typedef struct patch_indexer_teapot patch_indexer_teapot;

static GLfloat * index(int i, int j, patch_indexer_teapot * pip)
{
	return pip->p + 4*pip->u_order*j + 4*i;
}


static void build_maps(GLfloat * patch_position)
{
	// position map
	patch_indexer_teapot p;
	p.u_order = p.v_order = 4;
	p.p = patch_position;

    glMap2f(GL_MAP2_VERTEX_4, 0, 1, 4, 4, 0, 1, 16, 4, patch_position);
    
	
	// dp/du map
	GLfloat patch_dpdu[48];
	patch_indexer_teapot du;
	du.u_order = 3;
	du.v_order = 4;
	du.p = patch_dpdu;
	
	{
		for(int i=0; i < 3; i++)
			for(int j=0; j < 4; j++)
			{
				GLfloat * u = index(i,j, & du);
				GLfloat * pi0 = index(i, j, & p);
				GLfloat * pi1 = index(i+1, j, & p);

				for(int k=0; k < 3; k++)
					*u++ = *pi1++ - *pi0++;
				*u = 1;
			}
	}
	
    glMap2f(GL_MAP2_TEXTURE_COORD_4, 0, 1, 4, 3, 0, 1, 12, 4, patch_dpdu);
	
	// dp/dv map
	GLfloat patch_dpdv[48];
	patch_indexer_teapot dv;
	dv.u_order = 4;
	dv.v_order = 3;
	dv.p = patch_dpdv;
	
	{
		for(int i=0; i < 4; i++)
			for(int j=0; j < 3; j++)
			{
				GLfloat * v = index(i,j,& dv);
				GLfloat * pj0 = index(i, j, & p);
				GLfloat * pj1 = index(i, j+1, & p);

				for(int k=0; k < 3; k++)
					*v++ = *pj1++ - *pj0++;
				*v = 1;
			}
	}
	
    glMap2f(GL_MAP2_NORMAL, 0, 1, 4, 4, 0, 1, 16, 3, patch_dpdv);
}


/* *INDENT-ON* */

static void
teapot(GLdouble scale, GLenum type)
{
  float p[4][4][4], q[4][4][4], r[4][4][4], s[4][4][4];
  long i, j, k, l;

  	// simple texture coordinate map
	GLfloat patch_uv[16] =
	{
		1, 1, 0, 1,
		0, 1, 0, 1,
		1, 0, 0, 1,
		0, 0, 0, 1
	};
	
    glMap2f(GL_MAP2_COLOR_4, 0, 1, 4, 2, 0, 1, 8, 2, patch_uv);

  for (i = 0; i < 10; i++)
  {
	  for (j = 0; j < 4; j++)
	  {
		  for (k = 0; k < 4; k++)
		  {
			  for (l = 0; l < 3; l++)
			  {
				  p[j][k][l] = cpdata[patchdata[i][j * 4 + k]][l] * .25;
				  q[j][k][l] = cpdata[patchdata[i][j * 4 + (3 - k)]][l] * .25;
				  if (l == 1)
				  {
					  q[j][k][l] *= -1.0;
				  }
				  if (i < 6)
				  {
					  r[j][k][l] =
						  cpdata[patchdata[i][j * 4 + (3 - k)]][l] * .25;
					  if (l == 0)
					  {
						  r[j][k][l] *= -1.0;
					  }
					  s[j][k][l] = cpdata[patchdata[i][j * 4 + k]][l] * .25;
					  if (l == 0)
					  {
						  s[j][k][l] *= -1.0;
					  }
					  if (l == 1)
					  {
						  s[j][k][l] *= -1.0;
					  }
				  }
			  }
			  p[j][k][3] = q[j][k][3] = r[j][k][3] = s[j][k][3] = 1;
		  }
	  }

	  // tex coord map
	  // vertex map -- p
	  build_maps(&p[0][0][0]);
      glMapGrid2d(20, 0, 1, 20, 0, 1);
      glEvalMesh2(GL_FILL, 0, 20, 0, 20);

      // vertex map -- q
	  build_maps(&q[0][0][0]);
      glMapGrid2d(20, 0, 1, 20, 0, 1);
      glEvalMesh2(GL_FILL, 0, 20, 0, 20);

	  if (i < 6) {
		  // vertex map -- r
		  build_maps(&r[0][0][0]);
          glMapGrid2d(20, 0, 1, 20, 0, 1);
          glEvalMesh2(GL_FILL, 0, 20, 0, 20);

		  // vertex map -- s
		  build_maps(&s[0][0][0]);
          glMapGrid2d(20, 0, 1, 20, 0, 1);
          glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	  }
  }
}

void render_teapot()
{
	teapot(1, GL_FILL);
}