//
// $Id: //sw/devtools/SDK/9.5/SDK/DEMOS/OpenGL/inc/shared/ErrorHandling.h#1 $
//

#ifndef _ERRORHANDLING_
#define _ERRORHANDLING_

#include <assert.h>

const bool gl_Invalid_Enum = true;
const bool gl_Invalid_Value = true;
const bool gl_Invalid_Operation = true;
const bool gl_Stack_Overflow = true;
const bool gl_Stack_Underflow = true;
const bool gl_Out_Of_Memory = true;
const bool gl_Table_Too_Large = true;

// Simple assertion macros
// Could use some better info feedback mechanism. I'll look into it later. It's plenty good now.
#ifdef _DEBUG
  #define ASSERT(x) assert(x);
  #define GLASSERT() { GLenum err = glGetError(); \
    ASSERT(err != GL_INVALID_ENUM && gl_Invalid_Enum) \
    ASSERT(err != GL_INVALID_VALUE && gl_Invalid_Value) \
    ASSERT(err != GL_INVALID_OPERATION && gl_Invalid_Operation) \
    ASSERT(err != GL_STACK_OVERFLOW && gl_Stack_Overflow) \
    ASSERT(err != GL_STACK_UNDERFLOW && gl_Stack_Underflow) \
    ASSERT(err != GL_OUT_OF_MEMORY && gl_Out_Of_Memory) \
    ASSERT(err != GL_TABLE_TOO_LARGE && gl_Table_Too_Large) }
#else
  #define ASSERT(x)
  #define GLASSERT()
#endif

#endif
