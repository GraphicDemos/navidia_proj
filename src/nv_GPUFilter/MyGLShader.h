/*********************************************************************NVMH3****
File:  $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/src/GPUFilter/MyGLShader.h#1 $

Copyright NVIDIA Corporation 2005
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Comments:

******************************************************************************/

#ifndef _MYGLSHADER_
#define _MYGSHADER_

#include <shared/GLShader.h>
#include <shared/ErrorHandling.h>

#include <fstream>
// Adds a necessary bit of functionality to some kind soul's PBuffer class
class MyGLShader : GLShader
{
public:
  MyGLShader(ShaderType type, char *name) : GLShader(type,name) {}

  void AddCodeFromFile(const char *file,char *defaultCode)
  {
	std::ifstream in;
	ASSERT(file);
	in.open(file);
	if(!in.is_open()) {
		AddCode(defaultCode);
		return;
	}
	in.close();
	GLShader::AddCodeFromFile(file);
  }

protected:
private:
};

#endif