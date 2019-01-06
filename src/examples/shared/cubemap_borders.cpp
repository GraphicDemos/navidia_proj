#ifdef _WIN32
# include <windows.h>
#endif

#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif


#define _BOT_ROW(i,L)     ((i)*4)
#define _TOP_ROW(i,L)     ((((L)*(L)-(L))+(i))*4)
#define _LEFT_COL(i,L)    ((L)*(i)*4)
#define _RIGHT_COL(i,L)   ((((L)*(i)+(L)-1))*4)

#define BOT_ROW(i)     _BOT_ROW((i),w)
#define TOP_ROW(i)     _TOP_ROW((i),w)
#define LEFT_COL(i)    _LEFT_COL((i),w)
#define RIGHT_COL(i)   _RIGHT_COL((i),w)

#define B_BOT_ROW(i)     _BOT_ROW((i+1),W)
#define B_TOP_ROW(i)     _TOP_ROW((i+1),W)
#define B_LEFT_COL(i)    _LEFT_COL((i+1),W)
#define B_RIGHT_COL(i)   _RIGHT_COL((i+1),W)

#define _INDEX(i,j,L)  (((j)*(L)+(i))*4)

#define INDEX(i,j)     _INDEX((i),(j),w)
#define B_INDEX(i,j)   _INDEX((i),(j),W)
#define B_INDEX2(i,j)  _INDEX((i+1),(j+1),W)

void cubemap_borders()
{
	for(int level=0; true; level++)
	{
		GLint w,h;
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, level, GL_TEXTURE_WIDTH, & w);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, level, GL_TEXTURE_WIDTH, & h);
		GLint W = w+2;

		if((w*h) == 0)
			break;

		GLubyte *buffer = new GLubyte[w*h*4*6];
		GLubyte *img[6];
		GLubyte *bbuffer = new GLubyte[(w+2)*(h+2)*4*6];
		GLubyte *bimg[6];
		for(int jj = 0; jj < 6; jj++)
		{
			img[jj] = buffer + w*h*4*jj;
			bimg[jj] = bbuffer + (w+2)*(h+2)*4*jj;
		}

		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, level, GL_RGBA, GL_UNSIGNED_BYTE, img[0]);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, level, GL_RGBA, GL_UNSIGNED_BYTE, img[1]);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, level, GL_RGBA, GL_UNSIGNED_BYTE, img[2]);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, level, GL_RGBA, GL_UNSIGNED_BYTE, img[3]);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, level, GL_RGBA, GL_UNSIGNED_BYTE, img[4]);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, level, GL_RGBA, GL_UNSIGNED_BYTE, img[5]);

		// put the non-bordered images into the bordered buffer
		{
			for(int f = 0; f < 6; f++)
			{
				for(int j=0; j < h; j++)
				{
					for(int i=0; i < w; i++)
					{
						bimg[f][B_INDEX2(i,j)  ] = img[f][INDEX(i,j)  ];
						bimg[f][B_INDEX2(i,j)+1] = img[f][INDEX(i,j)+1];
						bimg[f][B_INDEX2(i,j)+2] = img[f][INDEX(i,j)+2];
						bimg[f][B_INDEX2(i,j)+3] = img[f][INDEX(i,j)+3];
					}
				}
			}
		}
		
		// do the shared edges
		{
			for(int c=0; c < 4; c++)
			{
				for(int i=0; i < w; i++)
				{
					
					// +x +z shared edge
					{
						// +x border @ +z edge
						bimg[0][B_LEFT_COL(i)+c] = img[4][RIGHT_COL(i)+c];
						
						// +z border @ +x edge
						bimg[4][B_RIGHT_COL(i)+c] = img[0][LEFT_COL(i)+c];
					}
					
					// +z -x shared edge
					{
						// +z border @ -x edge
						bimg[4][B_LEFT_COL(i)+c] = img[1][RIGHT_COL(i)+c];
						
						// -x border @ +z edge
						bimg[1][B_RIGHT_COL(i)+c] = img[4][LEFT_COL(i)+c];
					}
					
					// -x -z shared edge
					{
						// -x border @ -z edge
						bimg[1][B_LEFT_COL(i)+c] = img[5][RIGHT_COL(i)+c];
						
						// -z border @ -x edge
						bimg[5][B_RIGHT_COL(i)+c] = img[1][LEFT_COL(i)+c];
					}
					
					// -z +x shared edge
					{
						// -z border @ +x edge
						bimg[5][B_LEFT_COL(i)+c] = img[0][RIGHT_COL(i)+c];
						
						// +x border @ -z edge
						bimg[0][B_RIGHT_COL(i)+c] = img[5][LEFT_COL(i)+c];
					}
					
					
					
					// +x +y shared edge
					{
						// +x border @ +y edge
						bimg[0][B_BOT_ROW(i)+c] = img[2][RIGHT_COL(w-1-i)+c];
						
						// +y border @ +x edge
						bimg[2][B_RIGHT_COL(w-1-i)+c] = img[0][BOT_ROW(i)+c];
					}
					
					// -x +y shared edge
					{
						// -x border @ +y edge
						bimg[1][B_BOT_ROW(i)+c] = img[2][LEFT_COL(i)+c];
						
						// +y border @ -x edge
						bimg[2][B_LEFT_COL(i)+c] = img[1][BOT_ROW(i)+c];
					}
					
					
					// +z +y shared edge
					{
						// +z border @ +y edge
						bimg[4][B_BOT_ROW(i)+c] = img[2][TOP_ROW(i)+c];
						
						// +y border @ +z edge
						bimg[2][B_TOP_ROW(i)+c] = img[4][BOT_ROW(i)+c];
					}
					
					// -z +y shared edge
					{
						// -z border @ +y edge
						bimg[5][B_BOT_ROW(i)+c] = img[2][BOT_ROW(w-1-i)+c];
						
						// +y border @ -z edge
						bimg[2][B_BOT_ROW(w-1-i)+c] = img[5][BOT_ROW(i)+c];
					}
					
					// +x -y shared edge
					{
						// +x border @ -y edge
						bimg[0][B_TOP_ROW(i)+c] = img[3][RIGHT_COL(i)+c];
						
						// -y border @ +x edge
						bimg[3][B_RIGHT_COL(i)+c] = img[0][TOP_ROW(i)+c];
					}
					
					// -x -y shared edge
					{
						// -x border @ -y edge
						bimg[1][B_TOP_ROW(i)+c] = img[3][LEFT_COL(w-1-i)+c];
						
						// -y border @ -x edge
						bimg[3][B_LEFT_COL(w-1-i)+c] = img[1][TOP_ROW(i)+c];
					}
					
					
					// +z -y shared edge
					{
						// +z border @ -y edge
						bimg[4][B_TOP_ROW(i)+c] = img[3][BOT_ROW(i)+c];
						
						// -y border @ +z edge
						bimg[3][B_BOT_ROW(i)+c] = img[4][TOP_ROW(i)+c];
					}
					
					// -z -y shared edge
					{
						// -z border @ -y edge
						bimg[5][B_BOT_ROW(i)+c] = img[3][TOP_ROW(i)+c];
						
						// -y border @ -z edge
						bimg[3][B_TOP_ROW(i)+c] = img[5][BOT_ROW(i)+c];
					}
				}
			}
		}

		// put the non-bordered images into the bordered buffer
		{
			float t = 0.3333f;
			for(int f = 0; f < 6; f++)
			{
				for(int c = 0; c < 4; c++)
				{
					bimg[f][B_INDEX(  0,  0)+c] = (GLubyte)(bimg[f][B_INDEX(  1,  0)+c]*t + bimg[f][B_INDEX(  1,  1)+c]*t + bimg[f][B_INDEX(  0,  1)+c]*t);
					bimg[f][B_INDEX(W-1,  0)+c] = (GLubyte)(bimg[f][B_INDEX(W-2,  0)+c]*t + bimg[f][B_INDEX(W-2,  1)+c]*t + bimg[f][B_INDEX(W-1,  1)+c]*t);
					bimg[f][B_INDEX(  0,W-1)+c] = (GLubyte)(bimg[f][B_INDEX(  1,W-1)+c]*t + bimg[f][B_INDEX(  1,W-2)+c]*t + bimg[f][B_INDEX(  0,W-2)+c]*t);
					bimg[f][B_INDEX(W-1,W-1)+c] = (GLubyte)(bimg[f][B_INDEX(W-2,W-1)+c]*t + bimg[f][B_INDEX(W-2,W-2)+c]*t + bimg[f][B_INDEX(W-1,W-2)+c]*t);
				}
			}
		}


		// load the new bordered texture images...
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, level, GL_RGBA, W, W, 1, GL_RGBA,  GL_UNSIGNED_BYTE, bimg[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, level, GL_RGBA, W, W, 1, GL_RGBA,  GL_UNSIGNED_BYTE, bimg[1]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, level, GL_RGBA, W, W, 1, GL_RGBA,  GL_UNSIGNED_BYTE, bimg[2]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, level, GL_RGBA, W, W, 1, GL_RGBA,  GL_UNSIGNED_BYTE, bimg[3]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, level, GL_RGBA, W, W, 1, GL_RGBA,  GL_UNSIGNED_BYTE, bimg[4]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, level, GL_RGBA, W, W, 1, GL_RGBA,  GL_UNSIGNED_BYTE, bimg[5]);


		delete [] buffer;
		delete [] bbuffer;
	}

}
