/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  UtilityFunctions.cpp

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:

-------------------------------------------------------------------------------|--------------------*/

#include "UtilityFunctions.h"


#include "shared\NV_StringFuncs.h"
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"
#include <vector>

using namespace std;
#define FOUT_66  TEXT("%6.6f")
#define FSPACE   TEXT("   ")

#define NV_USING_D3D9

struct UDEC3
{
	unsigned n1 : 10;
	unsigned n2 : 10;
	unsigned n3	: 10;
};

// CASE_STRINGIFY returns a string representing the value.
//  Use it to output strings for named values
// Exapmle use:
// swtich( state )
// {  
//    CASE_STRCPY_STRINGIFY( pBuf, sz, D3DRS_ZENABLE )
//    CASE_STRCPY_STRINGIFY( pBuf, sz, D3DRS_FILLMODE )
// }
#ifndef CASE_STRINGIFY
    #define CASE_STRINGIFY( n ) case n : return( TEXT(#n) ); break;
#endif

#ifndef CASE_BOOLSTRINGIFY
  #define CASE_BOOLSTRINGIFY( r, v )				\
  case r :											\
     switch( v )									\
     {  CASE_STRCPY_STRINGIFY( pBuf, sz, TRUE )		\
	    CASE_STRCPY_STRINGIFY( pBuf, sz, FALSE )	\
	 }												\
  break;
#endif

#ifndef CASE_STRCPY_STRINGIFY
#define CASE_STRCPY_STRINGIFY( buf, sz, var )		\
	case var :										\
	_tcsncpy( buf, TEXT(#var), sz );				\
	return;											\
	break;
#endif

#ifndef CASE_STRCPY_STRINGIFY_9
  #define CASE_STRCPY_STRINGIFY_9( buf, sz, var )		\
    case var :											\
    _tcsncpy( buf, TEXT(#var), sz );					\
    return;												\
    break;
#endif


// Use these to test if a parameter has a certain bit-flag set.
// If the bit is set, the variable name of the flag will be
//  appended to the supplied string, s.
// The string must have enough space to hold the new characters!
//
// if( v & f != 0 ) _tcscat( s, #f ); _tcscat( s, " | ");
//
#ifndef FLAG_STRINGIFY
  #define FLAG_STRINGIFY( v, f, s )								\
  if( ((v) & (f)) == f )										\
  {																\
		if( _tcslen( s ) > 0 )									\
			_tcscat( s, TEXT(" | ") );							\
		_tcscat( s, TEXT(#f) );									\
  }												
#endif

// Same as FLAG_STRINGIFY except it removes the flag bits from the input
//  DWORD.  This will leave any remainder bits that were not detected by
//  a combination of calls to this macro
#ifndef FLAG_STRINGIFY_REMOVE
  #define FLAG_STRINGIFY_REMOVE( v, f, s )						\
  if( ((v) & (f)) == f )										\
  {																\
	if( _tcslen( s ) > 0 )										\
		_tcscat( s, TEXT(" | ") );								\
	_tcscat( s, TEXT(#f) );										\
	v = v & (~(f));												\
  }
#endif

#ifndef FLAG_STRINGIFY_9
  #ifdef NV_USING_D3D9
	#define FLAG_STRINGIFY_9( v, f, s )	FLAG_STRINGIFY( v, f, s )
  #else
	#define FLAG_STRINGIFY_9( v, f, s)   //
  #endif
#endif

#ifndef FLAG_STRINGIFY_REMOVE_9
  #ifdef NV_USING_D3D9
    #define FLAG_STRINGIFY_REMOVE_9( v, f, s ) FLAG_STRINGIFY_REMOVE( v, f, s )
  #else
    #define FLAG_STRINGIFY_REMOVE_9( v, f, s ) //
  #endif
#endif

//-----------------------------------------------------------

void	ListMatrix( const D3DXMATRIX & matrix )
{
	FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE TEXT("\n"), matrix._11, matrix._21, matrix._31, matrix._41 );
	FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE TEXT("\n"), matrix._12, matrix._22, matrix._32, matrix._42 );
	FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE TEXT("\n"), matrix._13, matrix._23, matrix._33, matrix._43 );
	FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE TEXT("\n"), matrix._14, matrix._24, matrix._34, matrix._44 );
}

#define mfprec( x, y, dp, fp )  FDebug("_" #x #y ": %" #dp "." #fp "f\t  ", pMat->_##x##y );
#define outmat( dp, fp )  mfprec( 1,1,dp,fp ); mfprec( 2,1,dp,fp ); mfprec(3,1,dp,fp); mfprec(4,1,dp,fp); FDebug("\n"); \
						  mfprec( 1,2,dp,fp ); mfprec( 2,2,dp,fp ); mfprec(3,2,dp,fp); mfprec(4,2,dp,fp); FDebug("\n"); \
						  mfprec( 1,3,dp,fp ); mfprec( 2,3,dp,fp ); mfprec(3,3,dp,fp); mfprec(4,3,dp,fp); FDebug("\n"); \
						  mfprec( 1,4,dp,fp ); mfprec( 2,4,dp,fp ); mfprec(3,4,dp,fp); mfprec(4,4,dp,fp); FDebug("\n"); 

void	TextOutMatrix( D3DXMATRIX * pMat )
{
	RET_IF( pMat == NULL );
	// numbers determine the precision at which to output each floating point number.
	// you cannot supply variables to this macro
	outmat( 4, 8 );
}

// No linefeed after last element
void	ListVector( const D3DXVECTOR3 & vector )
{
	FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66, vector.x, vector.y, vector.z );
}

void	ListVector( const TCHAR * prefix, const D3DXVECTOR3 & vector, const TCHAR * postfix )
{
	if( prefix != NULL && postfix != NULL )
	{
		FMsg(TEXT("%s ") FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 TEXT(" %s"), prefix, vector.x, vector.y, vector.z, postfix );
	}
	else if( prefix != NULL )
	{
		FMsg(TEXT("%s ") FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66, prefix, vector.x, vector.y, vector.z );
	}
	else if( postfix != NULL )
	{
		FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 TEXT(" %s"), vector.x, vector.y, vector.z, postfix );
	}
}

void	ListVector( const D3DXVECTOR4 & vector )
{
	FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 TEXT("\n"), vector.x, vector.y, vector.z, vector.w );
}

void	ListVector( const TCHAR * prefix, const D3DXVECTOR4 & vector, const TCHAR * postfix )
{
	if( prefix != NULL && postfix != NULL )
	{
		FMsg(TEXT("%s ") FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 TEXT(" %s"), prefix, vector.x, vector.y, vector.z, vector.w, postfix );
	}
	else if( prefix != NULL )
	{
		FMsg(TEXT("%s ") FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66, prefix, vector.x, vector.y, vector.z, vector.w );
	}
	else if( postfix != NULL )
	{
		FMsg( FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 FSPACE FOUT_66 TEXT(" %s"), vector.x, vector.y, vector.z, vector.w, postfix );
	}
}

tstring GetStringOfBits( SHORT n )
{
	int i;
	tstring tstr = TEXT("");
	for( i = sizeof( short ) * 8 - 1; i >= 0; i-- )
	{
		tstr += ( n >> i ) == 1 ? TEXT("1") : TEXT("0");
	}
	return( tstr );
}

//------------------------------------------------------------------

// Returns a string based on the value of the format variable
// ie. GetStrD3DFORMAT( D3DFMT_R8G8B8 ) returns "D3DFMT_R8G8B8"
const TCHAR * GetStrD3DFORMAT( D3DFORMAT format )
{
#ifdef NV_USING_D3D8
	FMsg("*** GetStrD3DFORMAT not implemented for D3D8\n");
#endif

#ifdef NV_USING_D3D9
	switch( format )
	{
	// unsigned formats
	CASE_STRINGIFY(  D3DFMT_R8G8B8			)
	CASE_STRINGIFY(  D3DFMT_A8R8G8B8		)
	CASE_STRINGIFY(  D3DFMT_X8R8G8B8		)
	CASE_STRINGIFY(  D3DFMT_R5G6B5			)
	CASE_STRINGIFY(  D3DFMT_X1R5G5B5		)
	CASE_STRINGIFY(  D3DFMT_A1R5G5B5		)
	CASE_STRINGIFY(  D3DFMT_A4R4G4B4		)
	CASE_STRINGIFY(  D3DFMT_R3G3B2			)
	CASE_STRINGIFY(  D3DFMT_A8				)
	CASE_STRINGIFY(  D3DFMT_A8R3G3B2		)
	CASE_STRINGIFY(  D3DFMT_X4R4G4B4		)
	CASE_STRINGIFY(  D3DFMT_A2B10G10R10		)
	CASE_STRINGIFY(  D3DFMT_A8B8G8R8		)
	CASE_STRINGIFY(  D3DFMT_X8B8G8R8		)
	CASE_STRINGIFY(  D3DFMT_G16R16			)
	CASE_STRINGIFY(  D3DFMT_A2R10G10B10		)
	CASE_STRINGIFY(  D3DFMT_A16B16G16R16	)
	CASE_STRINGIFY(  D3DFMT_A8P8			)
	CASE_STRINGIFY(  D3DFMT_P8				)
	CASE_STRINGIFY(  D3DFMT_L8				)
	CASE_STRINGIFY(  D3DFMT_L16				)
	CASE_STRINGIFY(  D3DFMT_A8L8			)
	CASE_STRINGIFY(  D3DFMT_A4L4			)

	// signed formats
	CASE_STRINGIFY(  D3DFMT_V8U8			)
	CASE_STRINGIFY(  D3DFMT_Q8W8V8U8		)
	CASE_STRINGIFY(  D3DFMT_V16U16			)
	CASE_STRINGIFY(  D3DFMT_Q16W16V16U16	)
	CASE_STRINGIFY(  D3DFMT_CxV8U8 			)

	// mixed formats
	CASE_STRINGIFY(  D3DFMT_L6V5U5			)
	CASE_STRINGIFY(  D3DFMT_X8L8V8U8		)
	CASE_STRINGIFY(  D3DFMT_A2W10V10U10 	)

	// FOURCC formats
	CASE_STRINGIFY(  D3DFMT_MULTI2_ARGB8	)
	CASE_STRINGIFY(  D3DFMT_G8R8_G8B8		)
	CASE_STRINGIFY(  D3DFMT_R8G8_B8G8		)
	CASE_STRINGIFY(  D3DFMT_DXT1			)
	CASE_STRINGIFY(  D3DFMT_DXT2			)
	CASE_STRINGIFY(  D3DFMT_DXT3			)
	CASE_STRINGIFY(  D3DFMT_DXT4			)
	CASE_STRINGIFY(  D3DFMT_DXT5			)
	CASE_STRINGIFY(  D3DFMT_UYVY			)
	CASE_STRINGIFY(  D3DFMT_YUY2 			)

	// Buffer formats
	CASE_STRINGIFY(  D3DFMT_D16_LOCKABLE	)
	CASE_STRINGIFY(  D3DFMT_D32				)
	CASE_STRINGIFY(  D3DFMT_D15S1			)
	CASE_STRINGIFY(  D3DFMT_D24S8			)
	CASE_STRINGIFY(  D3DFMT_D24X8			)
	CASE_STRINGIFY(  D3DFMT_D24X4S4			)
	CASE_STRINGIFY(  D3DFMT_D32F_LOCKABLE	)
	CASE_STRINGIFY(  D3DFMT_D24FS8			)
	CASE_STRINGIFY(  D3DFMT_D16				)
	CASE_STRINGIFY(  D3DFMT_VERTEXDATA		)
	CASE_STRINGIFY(  D3DFMT_INDEX16			)
	CASE_STRINGIFY(  D3DFMT_INDEX32 		)

	// floating-point formats
	CASE_STRINGIFY(  D3DFMT_R16F			)
	CASE_STRINGIFY(  D3DFMT_G16R16F			)
	CASE_STRINGIFY(  D3DFMT_A16B16G16R16F 	)

	// IEEE formats
	CASE_STRINGIFY(  D3DFMT_R32F			)
	CASE_STRINGIFY(  D3DFMT_G32R32F			)
	CASE_STRINGIFY(  D3DFMT_A32B32G32R32F	) 

	CASE_STRINGIFY(  D3DFMT_UNKNOWN			)

	}
#endif

	// each of those cases returns a string, so if none were found, return our
	//  own 'unknown' string and report the format value via OutputDebugString(..)
	FMsg(TEXT("unknown D3DFORMAT: %u\n"), format );
	return( TEXT("(Unknown D3DFORMAT)") );
}

void GetStrD3DLOCK(	DWORD flags, TCHAR * pBuf, size_t sz )
{
	RET_IF( pBuf == NULL );
	RET_IF( sz == 0 );
	if( flags == 0 )
	{
		_tcscpy( pBuf, TEXT("0") );
		return;
	}
	pBuf[0] = '\0';

	// string big enough to hold all flag names
	int strsz = 50 * 6;
	TCHAR * pc = new TCHAR[strsz];
	RET_IF( pc == NULL );
	_tcscpy( pc, TEXT("") );

	DWORD f = flags;
	FLAG_STRINGIFY_REMOVE( f, D3DLOCK_DISCARD,			pc );	
	FLAG_STRINGIFY_REMOVE( f, D3DLOCK_DONOTWAIT,		pc );
	FLAG_STRINGIFY_REMOVE( f, D3DLOCK_NO_DIRTY_UPDATE,	pc );
	FLAG_STRINGIFY_REMOVE( f, D3DLOCK_NOOVERWRITE,		pc );
	FLAG_STRINGIFY_REMOVE( f, D3DLOCK_NOSYSLOCK,		pc );
	FLAG_STRINGIFY_REMOVE( f, D3DLOCK_READONLY,			pc );
	// report any leftover bits we didn't detect:
	if( f != 0 )
	{
		TCHAR tcsRemainder[20];
		_stprintf( tcsRemainder, TEXT("0x%X"), f );
		if( _tcslen( pc ) > 0 )
			_tcscat( pc, TEXT(" | ") );
		_tcscat( pc, tcsRemainder );
	}

	// Make sure string is terminated
	// There will be another terminator before this one, to 
	//  give the actual size of the string, unless the string size was exceeded
	pc[strsz-1] = '\0';
	if( _tcslen(pc) > sz )
		_tcsncpy( pBuf, TEXT("<STR TOO SMALL>"), sz );
	else
		_tcsncpy( pBuf, pc, sz );
	delete [] pc;
	pBuf[sz-1] = '\0';
}

const TCHAR * GetStrD3DRESOURCETYPE( D3DRESOURCETYPE restype )
{
	switch( restype )
	{
    CASE_STRINGIFY( D3DRTYPE_SURFACE )
    CASE_STRINGIFY( D3DRTYPE_VOLUME )
    CASE_STRINGIFY( D3DRTYPE_TEXTURE )
    CASE_STRINGIFY( D3DRTYPE_VOLUMETEXTURE )
	CASE_STRINGIFY( D3DRTYPE_CUBETEXTURE )
	CASE_STRINGIFY( D3DRTYPE_VERTEXBUFFER )
	CASE_STRINGIFY( D3DRTYPE_INDEXBUFFER )
	}

	FMsg(TEXT("unknown D3DRESOURCETYPE: %u\n"), restype );
	return( TEXT("(Unknown D3DRESOURCETYPE)") );
}

const TCHAR * GetStrD3DPOOL( D3DPOOL pool )
{
 	switch( pool )
	{
    CASE_STRINGIFY( D3DPOOL_DEFAULT )
    CASE_STRINGIFY( D3DPOOL_MANAGED )
    CASE_STRINGIFY( D3DPOOL_SYSTEMMEM )
    CASE_STRINGIFY( D3DPOOL_SCRATCH )
	}
	FMsg(TEXT("unknown D3DPPOOL: %u\n"), pool );
	return( TEXT("(Unknown D3DPOOL)") );
}

const TCHAR * GetStrD3DMULTISAMPLE_TYPE( D3DMULTISAMPLE_TYPE multisampletype )
{
	switch( multisampletype )
	{
    CASE_STRINGIFY( D3DMULTISAMPLE_NONE )
    CASE_STRINGIFY( D3DMULTISAMPLE_NONMASKABLE )	
    CASE_STRINGIFY( D3DMULTISAMPLE_2_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_3_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_4_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_5_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_6_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_7_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_8_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_9_SAMPLES )	
    CASE_STRINGIFY( D3DMULTISAMPLE_10_SAMPLES )
    CASE_STRINGIFY( D3DMULTISAMPLE_11_SAMPLES ) 
    CASE_STRINGIFY( D3DMULTISAMPLE_12_SAMPLES ) 
    CASE_STRINGIFY( D3DMULTISAMPLE_13_SAMPLES ) 
    CASE_STRINGIFY( D3DMULTISAMPLE_14_SAMPLES ) 
    CASE_STRINGIFY( D3DMULTISAMPLE_15_SAMPLES ) 
    CASE_STRINGIFY( D3DMULTISAMPLE_16_SAMPLES ) 
	}

	FMsg(TEXT("Unknown D3DMULTISAMPLE_TYPE: %u\n"), multisampletype );
	return( TEXT("(Unknown D3DMULTISAMPLE_TYPE)") );
}

void GetStrUsage( DWORD usage, TCHAR * pBuf, size_t sz )
{
	RET_IF( pBuf == NULL );
	RET_IF( sz == 0 );
	if( usage == 0 )
	{
		_tcscpy( pBuf, TEXT("0") );
		return;
	}
	pBuf[0] = '\0';

	// string big enough to hold all flag names
	int strsz = (30 + 10) * 16;
	TCHAR * pc = new TCHAR[strsz];
	RET_IF( pc == NULL );
	_tcscpy( pc, TEXT("") );

	DWORD u2 = usage;
	FLAG_STRINGIFY_REMOVE_9( u2, D3DUSAGE_AUTOGENMIPMAP, pc );	
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_DEPTHSTENCIL, pc );
	FLAG_STRINGIFY_REMOVE_9( u2, D3DUSAGE_DMAP, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_DONOTCLIP, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_DYNAMIC, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_NPATCHES, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_POINTS, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_RENDERTARGET, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_RTPATCHES, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_SOFTWAREPROCESSING, pc );
	FLAG_STRINGIFY_REMOVE( u2, D3DUSAGE_WRITEONLY, pc );
	// report any leftover bits we didn't detect:

	if( u2 != 0 )
	{
		TCHAR tcsRemainder[20];
		_stprintf( tcsRemainder, TEXT("0x%X"), u2 );
		if( _tcslen( pc ) > 0 )
			_tcscat( pc, TEXT(" | ") );
		_tcscat( pc, tcsRemainder );
	}

	// Make sure string is terminated
	// There will be another terminator before this one, to 
	//  give the actual size of the string, unless the string size was exceeded
	pc[strsz-1] = '\0';

	if( _tcslen(pc) > sz )
		_tcsncpy( pBuf, TEXT("<STR TOO SMALL>"), sz );
	else
		_tcsncpy( pBuf, pc, sz );
	delete [] pc;
	pBuf[sz-1] = '\0';
}


void	GetStrD3DFVF( DWORD fvf, TCHAR * out_pBuf, size_t sz )
{
	RET_IF( out_pBuf == NULL );
	RET_IF( sz < 4 );
	if( fvf == 0 )
	{
		_tcscpy( out_pBuf, TEXT("0"));
		return;
	}
	out_pBuf[0] = '\0';
	// string big enough to hold all FVF names
	int strsz = 4096;
	TCHAR * pc = new TCHAR[ strsz ];
	RET_IF( pc == NULL );
	_tcscpy( pc, TEXT("") );
	DWORD f2 = fvf;

	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZ,			pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZRHW,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZB1,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZB2,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZB3,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZB4,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZB5,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_XYZW,			pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_NORMAL,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_DIFFUSE,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_SPECULAR,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_PSIZE,		pc );

	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX0,		pc );	// D3DFVF_TEX0 = 0
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX1,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX2,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX3,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX4,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX5,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX6,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX7,		pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEX8,		pc );

	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE1(0),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE1(1),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE1(2),	pc );
//	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE1(3),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE2(0),	pc );	// each of the D3DFVF_TEXCOORDSIZE2 macros evaluate to 0
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE2(1),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE2(2),	pc );
//	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE2(3),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE3(0),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE3(1),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE3(2),	pc );
//	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE3(3),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE4(0),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE4(1),	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE4(2),	pc );
//	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOORDSIZE4(3),	pc );

//	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_POSITION_MASK,	pc );
//	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_TEXCOUNT_MASK,	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_LASTBETA_D3DCOLOR,	pc );
	FLAG_STRINGIFY_REMOVE( f2, D3DFVF_LASTBETA_UBYTE4,		pc );

	// If there are remaining bits that we didn't detect with the flags,
	//  add the value to the end of the string descriptor
	if( f2 != 0 )
	{
		TCHAR tcsRemainder[20];
		_stprintf( tcsRemainder, TEXT("0x%X"), f2 );
		if( _tcslen( pc ) > 0 )
			_tcscat( pc, TEXT(" | ") );
		_tcscat( pc, tcsRemainder );
	}
	// Make sure string is terminated
	// There will be another terminator before this one, to give the actual size
	// of the string, unless the buffer size was exceeded.
	pc[strsz-1] = '\0';

	if( _tcslen(pc) > sz )
		_tcsncpy( out_pBuf, TEXT("<STR TOO SMALL>"), sz );
	else
		_tcsncpy( out_pBuf, pc, sz );
	delete [] pc;
	out_pBuf[sz-1] = '\0';
}

const TCHAR * GetStrRenderState( D3DRENDERSTATETYPE renderstate )
{
	switch( renderstate )
	{
	CASE_STRINGIFY(    D3DRS_ZENABLE                   ) // 7
	CASE_STRINGIFY(    D3DRS_FILLMODE                  ) // 8
	CASE_STRINGIFY(    D3DRS_SHADEMODE                 ) // 9
	CASE_STRINGIFY(    D3DRS_ZWRITEENABLE              ) // 14
	CASE_STRINGIFY(    D3DRS_ALPHATESTENABLE           ) // 15
	CASE_STRINGIFY(    D3DRS_LASTPIXEL                 ) // 16
	CASE_STRINGIFY(    D3DRS_SRCBLEND                  ) // 19
	CASE_STRINGIFY(    D3DRS_DESTBLEND                 ) // 20
	CASE_STRINGIFY(    D3DRS_CULLMODE                  ) // 22
	CASE_STRINGIFY(    D3DRS_ZFUNC                     ) // 23
	CASE_STRINGIFY(    D3DRS_ALPHAREF                  ) // 24
	CASE_STRINGIFY(    D3DRS_ALPHAFUNC                 ) // 25
	CASE_STRINGIFY(    D3DRS_DITHERENABLE              ) // 26
	CASE_STRINGIFY(    D3DRS_ALPHABLENDENABLE          ) // 27
	CASE_STRINGIFY(    D3DRS_FOGENABLE                 ) // 28
	CASE_STRINGIFY(    D3DRS_SPECULARENABLE            ) // 29
	CASE_STRINGIFY(    D3DRS_FOGCOLOR                  ) 
	CASE_STRINGIFY(    D3DRS_FOGTABLEMODE              ) 
	CASE_STRINGIFY(    D3DRS_FOGSTART                  ) 
	CASE_STRINGIFY(    D3DRS_FOGEND                    ) 
	CASE_STRINGIFY(    D3DRS_FOGDENSITY                ) 
	CASE_STRINGIFY(    D3DRS_RANGEFOGENABLE            ) 
	CASE_STRINGIFY(    D3DRS_STENCILENABLE             ) 
	CASE_STRINGIFY(    D3DRS_STENCILFAIL               ) 
	CASE_STRINGIFY(    D3DRS_STENCILZFAIL              ) 
	CASE_STRINGIFY(    D3DRS_STENCILPASS               ) 
	CASE_STRINGIFY(    D3DRS_STENCILFUNC               ) 
	CASE_STRINGIFY(    D3DRS_STENCILREF                ) 
	CASE_STRINGIFY(    D3DRS_STENCILMASK               ) 
	CASE_STRINGIFY(    D3DRS_STENCILWRITEMASK          ) 
	CASE_STRINGIFY(    D3DRS_TEXTUREFACTOR             ) 
	CASE_STRINGIFY(    D3DRS_WRAP0                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP1                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP2                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP3                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP4                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP5                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP6                     ) 
	CASE_STRINGIFY(    D3DRS_WRAP7                     )
	CASE_STRINGIFY(    D3DRS_CLIPPING                  ) 
	CASE_STRINGIFY(    D3DRS_LIGHTING                  ) 
	CASE_STRINGIFY(    D3DRS_AMBIENT                   ) 
	CASE_STRINGIFY(    D3DRS_FOGVERTEXMODE             ) 
	CASE_STRINGIFY(    D3DRS_COLORVERTEX               ) // 141
	CASE_STRINGIFY(    D3DRS_LOCALVIEWER               ) 
	CASE_STRINGIFY(    D3DRS_NORMALIZENORMALS          ) 
	CASE_STRINGIFY(    D3DRS_DIFFUSEMATERIALSOURCE     ) 
	CASE_STRINGIFY(    D3DRS_SPECULARMATERIALSOURCE    ) 
	CASE_STRINGIFY(    D3DRS_AMBIENTMATERIALSOURCE     ) 
	CASE_STRINGIFY(    D3DRS_EMISSIVEMATERIALSOURCE    ) 
	CASE_STRINGIFY(    D3DRS_VERTEXBLEND               ) 
	CASE_STRINGIFY(    D3DRS_CLIPPLANEENABLE           ) 
	CASE_STRINGIFY(    D3DRS_POINTSIZE                 ) 
	CASE_STRINGIFY(    D3DRS_POINTSIZE_MIN             ) 
	CASE_STRINGIFY(    D3DRS_POINTSPRITEENABLE         ) 
	CASE_STRINGIFY(    D3DRS_POINTSCALEENABLE          ) // 157
	CASE_STRINGIFY(    D3DRS_POINTSCALE_A              ) 
	CASE_STRINGIFY(    D3DRS_POINTSCALE_B              ) 
	CASE_STRINGIFY(    D3DRS_POINTSCALE_C              ) 
	CASE_STRINGIFY(    D3DRS_MULTISAMPLEANTIALIAS      ) 
	CASE_STRINGIFY(    D3DRS_MULTISAMPLEMASK           ) 
	CASE_STRINGIFY(    D3DRS_PATCHEDGESTYLE            ) 
	CASE_STRINGIFY(    D3DRS_DEBUGMONITORTOKEN         ) 
	CASE_STRINGIFY(    D3DRS_POINTSIZE_MAX             ) // 166
	CASE_STRINGIFY(    D3DRS_INDEXEDVERTEXBLENDENABLE  ) // 167
	CASE_STRINGIFY(    D3DRS_COLORWRITEENABLE          ) // 168
	CASE_STRINGIFY(    D3DRS_TWEENFACTOR               ) // 170
	CASE_STRINGIFY(    D3DRS_BLENDOP                   ) // 171
	CASE_STRINGIFY(    D3DRS_POSITIONDEGREE            ) // 172
	CASE_STRINGIFY(    D3DRS_NORMALDEGREE				) // 173,
	CASE_STRINGIFY(    D3DRS_SCISSORTESTENABLE		) // 174,
	CASE_STRINGIFY(    D3DRS_SLOPESCALEDEPTHBIAS		) // 175,
	CASE_STRINGIFY(    D3DRS_ANTIALIASEDLINEENABLE	) // 176,
	CASE_STRINGIFY(    D3DRS_MINTESSELLATIONLEVEL		) // 178,
	CASE_STRINGIFY(    D3DRS_MAXTESSELLATIONLEVEL		) // 179,
	CASE_STRINGIFY(    D3DRS_ADAPTIVETESS_X			) // 180,
	CASE_STRINGIFY(    D3DRS_ADAPTIVETESS_Y			) // 181,
	CASE_STRINGIFY(    D3DRS_ADAPTIVETESS_Z			) // 182,
	CASE_STRINGIFY(    D3DRS_ADAPTIVETESS_W			) // 183,
	CASE_STRINGIFY(    D3DRS_TWOSIDEDSTENCILMODE		) // 185,
	CASE_STRINGIFY(    D3DRS_CCW_STENCILFAIL			) // 186,
	CASE_STRINGIFY(    D3DRS_CCW_STENCILZFAIL			) // 187,
	CASE_STRINGIFY(    D3DRS_CCW_STENCILPASS			) // 188,
	CASE_STRINGIFY(    D3DRS_CCW_STENCILFUNC			) // 189,
	CASE_STRINGIFY(    D3DRS_COLORWRITEENABLE1		) // 190,
	CASE_STRINGIFY(    D3DRS_COLORWRITEENABLE2		) // 191,
	CASE_STRINGIFY(    D3DRS_COLORWRITEENABLE3		) // 192,
	CASE_STRINGIFY(    D3DRS_BLENDFACTOR				) // 193,
	CASE_STRINGIFY(    D3DRS_SRGBWRITEENABLE			) // 194,
	CASE_STRINGIFY(    D3DRS_DEPTHBIAS				) // 195,
	CASE_STRINGIFY(    D3DRS_WRAP8					) // 198,
	CASE_STRINGIFY(    D3DRS_WRAP9					) // 199,
	CASE_STRINGIFY(    D3DRS_WRAP10					) // 200,
	CASE_STRINGIFY(    D3DRS_WRAP11					) // 201,
	CASE_STRINGIFY(    D3DRS_WRAP12					) // 202,
	CASE_STRINGIFY(    D3DRS_WRAP13					) // 203,
	CASE_STRINGIFY(    D3DRS_WRAP14					) // 204,
	CASE_STRINGIFY(    D3DRS_WRAP15					) // 205,
	CASE_STRINGIFY(    D3DRS_SEPARATEALPHABLENDENABLE ) // 206,
	CASE_STRINGIFY(    D3DRS_SRCBLENDALPHA			) // 207
	CASE_STRINGIFY(    D3DRS_DESTBLENDALPHA			) // 208
	CASE_STRINGIFY(    D3DRS_BLENDOPALPHA				) // 209
	}

//	FMsg(TEXT("Unknown render state: %u\n"), renderstate );
	return( TEXT("(Unknown renderstate)") );
}

// Return a string based on rsvalue.  Interpretation of the rsvalue depends
//  on the renderstate.
// pBuf must point to at least 128 chars.
void GetStrRenderStateValue( D3DRENDERSTATETYPE renderstate, DWORD rsvalue, TCHAR * pBuf, size_t sz )
{
	RET_IF( pBuf == NULL );
	RET_IF( sz == 0 );
	pBuf[0] = '\0';
	if( sz < 128 )
	{
		pBuf[0] = '\0';
		return;
	}
	size_t len;

	switch( renderstate )
	{
	case D3DRS_ZENABLE :                  // 7
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DZB_FALSE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DZB_TRUE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DZB_USEW )
		}
		break;
	case D3DRS_FILLMODE :                  // 8
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFILL_POINT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFILL_WIREFRAME )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFILL_SOLID )
		}
		break;
	case D3DRS_SHADEMODE :                 // 9
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSHADE_FLAT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSHADE_GOURAUD )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSHADE_PHONG )
		}
		break;

#ifdef NV_USING_D3D8
	case D3DRS_LINEPATTERN :               // 10
		_sntprintf( pBuf, sz, TEXT("0x%X"), rsvalue );
		return;
		break;
#endif

	CASE_BOOLSTRINGIFY( D3DRS_ZWRITEENABLE,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_ALPHATESTENABLE,		rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_LASTPIXEL,			rsvalue )

	case D3DRS_SRCBLEND :                   // 19
	case D3DRS_DESTBLEND :					// 20
#ifdef NV_USING_D3D9
	case D3DRS_SRCBLENDALPHA :			    // 207
	case D3DRS_DESTBLENDALPHA :			    // 208
#endif
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_ZERO )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_ONE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_SRCCOLOR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_INVSRCCOLOR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_SRCALPHA )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_INVSRCALPHA )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_DESTALPHA )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_INVDESTALPHA )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_DESTCOLOR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_INVDESTCOLOR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_SRCALPHASAT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_BOTHSRCALPHA )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLEND_BOTHINVSRCALPHA )
		CASE_STRCPY_STRINGIFY_9( pBuf, sz, D3DBLEND_BLENDFACTOR )
		CASE_STRCPY_STRINGIFY_9( pBuf, sz, D3DBLEND_INVBLENDFACTOR )
		}
		break;
	case D3DRS_CULLMODE :                  // 22
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCULL_NONE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCULL_CW )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCULL_CCW )
		}
		break;
	case D3DRS_ZFUNC :                     // 23
	case D3DRS_ALPHAFUNC :                 // 25
	case D3DRS_STENCILFUNC :
#ifdef NV_USING_D3D9
	case D3DRS_CCW_STENCILFUNC :		   // 189
#endif
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_NEVER )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_LESS )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_EQUAL )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_LESSEQUAL )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_GREATER )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_NOTEQUAL )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_GREATEREQUAL )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_ALWAYS )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DCMP_FORCE_DWORD )
		}
		break;

	case D3DRS_ALPHAREF :                  // 24
		_sntprintf( pBuf, sz, TEXT("%d"), rsvalue );
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_DITHERENABLE,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_ALPHABLENDENABLE,		rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_FOGENABLE,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_SPECULARENABLE,		rsvalue )

#ifdef NV_USING_D3D8
	case D3DRS_ZVISIBLE :					// 30 : NOT SUPPORTED in DX8 and doesn't exist in DX9
		_sntprintf( pBuf, sz, TEXT("%d"), 0 );
		return;
		break;
#endif

	case D3DRS_FOGCOLOR :
		_sntprintf( pBuf, sz, TEXT("%X"), rsvalue );
		return;
		break;
	case D3DRS_FOGTABLEMODE :
	case D3DRS_FOGVERTEXMODE :             
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFOG_NONE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFOG_EXP )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFOG_EXP2 )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DFOG_LINEAR )
		}
		break;

	case D3DRS_FOGSTART :
	case D3DRS_FOGEND :                    
	case D3DRS_FOGDENSITY :                
		_sntprintf( pBuf, sz, TEXT("%ff"), *((float*)&rsvalue) );
		return;
		break;

#ifdef NV_USING_D3D8
	CASE_BOOLSTRINGIFY( D3DRS_EDGEANTIALIAS,			rsvalue )
	case D3DRS_ZBIAS :								// integer value
		_sntprintf( pBuf, sz, TEXT("%u"), rsvalue );
		return;
		break;
#endif

	CASE_BOOLSTRINGIFY( D3DRS_RANGEFOGENABLE,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_STENCILENABLE,			rsvalue )

	case D3DRS_STENCILFAIL :
	case D3DRS_STENCILZFAIL :              
	case D3DRS_STENCILPASS :
#ifdef NV_USING_D3D9
	case D3DRS_CCW_STENCILFAIL :		  // 186
	case D3DRS_CCW_STENCILZFAIL :		  // 187
	case D3DRS_CCW_STENCILPASS :		  // 188
#endif
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_KEEP )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_ZERO )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_REPLACE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_INCRSAT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_DECRSAT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_INVERT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_INCR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DSTENCILOP_DECR )
		}
		break;

	case D3DRS_STENCILREF :                
	case D3DRS_STENCILMASK :               
	case D3DRS_STENCILWRITEMASK :          
	case D3DRS_TEXTUREFACTOR :
		_sntprintf( pBuf, sz, TEXT("0x%X"), rsvalue );
		return;
		break;

	case D3DRS_WRAP0 :                     
	case D3DRS_WRAP1 :                     
	case D3DRS_WRAP2 :                     
	case D3DRS_WRAP3 :                     
	case D3DRS_WRAP4 :                     
	case D3DRS_WRAP5 :                     
	case D3DRS_WRAP6 :                     
	case D3DRS_WRAP7 :
#ifdef NV_USING_D3D9
	case D3DRS_WRAP8 :					// 198
	case D3DRS_WRAP9 :					// 199
	case D3DRS_WRAP10 :					// 200
	case D3DRS_WRAP11 :					// 201
	case D3DRS_WRAP12 :					// 202
	case D3DRS_WRAP13 :					// 203
	case D3DRS_WRAP14 :					// 204
	case D3DRS_WRAP15 :					// 205
#endif
		_tcscpy( pBuf, TEXT("0 | "));
		if( (rsvalue & D3DWRAP_U) == D3DWRAP_U )
			_tcscat( pBuf, TEXT("D3DWRAP_U | ") );
		if( (rsvalue & D3DWRAP_V) == D3DWRAP_V )
			_tcscat( pBuf, TEXT("D3DWRAP_V | ") );
		if( (rsvalue & D3DWRAP_W) == D3DWRAP_W )
			_tcscat( pBuf, TEXT("D3DWRAP_W | ") );
		len = _tcslen( pBuf );
		if( len > 3 )
			pBuf[ len-3 ] = '\0';
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_CLIPPING,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_LIGHTING,			rsvalue )

	case D3DRS_AMBIENT :                   
		_sntprintf( pBuf, sz, TEXT("%X"), rsvalue );
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_COLORVERTEX,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_LOCALVIEWER,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_NORMALIZENORMALS,		rsvalue )

	case D3DRS_DIFFUSEMATERIALSOURCE :
	case D3DRS_SPECULARMATERIALSOURCE :    
	case D3DRS_AMBIENTMATERIALSOURCE :     
	case D3DRS_EMISSIVEMATERIALSOURCE :    
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DMCS_MATERIAL )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DMCS_COLOR1 )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DMCS_COLOR2 )
		}
		break;
	case D3DRS_VERTEXBLEND :               
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DVBF_DISABLE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DVBF_1WEIGHTS )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DVBF_2WEIGHTS )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DVBF_3WEIGHTS )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DVBF_TWEENING )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DVBF_0WEIGHTS )
		}
		break;

	case D3DRS_CLIPPLANEENABLE :
		_sntprintf( pBuf, sz, TEXT("%X"), rsvalue );
		return;
		break;

#ifdef NV_USING_D3D8
	CASE_BOOLSTRINGIFY( D3DRS_SOFTWAREVERTEXPROCESSING, rsvalue )
#endif

	case D3DRS_POINTSIZE :                 
	case D3DRS_POINTSIZE_MIN :
	case D3DRS_POINTSIZE_MAX :							// 166
	case D3DRS_POINTSCALE_A :        
	case D3DRS_POINTSCALE_B :              
	case D3DRS_POINTSCALE_C :
	case D3DRS_TWEENFACTOR :							// 170
		_sntprintf( pBuf, sz, TEXT("%ff"), *((float*)&rsvalue) );
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_POINTSPRITEENABLE,		rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_POINTSCALEENABLE,			rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_MULTISAMPLEANTIALIAS,		rsvalue )

	case D3DRS_MULTISAMPLEMASK :        
		_sntprintf( pBuf, sz, TEXT("0x%X"), rsvalue );
		return;
		break;

	case D3DRS_PATCHEDGESTYLE :            
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DPATCHEDGE_DISCRETE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DPATCHEDGE_CONTINUOUS )
		}
		break;

#ifdef NV_USING_D3D8
	case D3DRS_PATCHSEGMENTS : 
		_sntprintf( pBuf, sz, TEXT("%ff"), *((float*)&rsvalue) );
		return;
		break;
#endif

	case D3DRS_DEBUGMONITORTOKEN :         
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DDMT_ENABLE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DDMT_DISABLE )
		}
		break;

	CASE_BOOLSTRINGIFY( D3DRS_INDEXEDVERTEXBLENDENABLE,		rsvalue )

	case D3DRS_COLORWRITEENABLE :          // 168
#ifdef NV_USING_D3D9
	case D3DRS_COLORWRITEENABLE1 :		   // 190
	case D3DRS_COLORWRITEENABLE2 :		   // 191
	case D3DRS_COLORWRITEENABLE3 :		   // 192
#endif
		if( rsvalue == 0 )
		{
			_tcscpy( pBuf, TEXT("0") );
			return;
		}
		if( (rsvalue & D3DCOLORWRITEENABLE_RED) != 0 )
			_tcscat( pBuf, TEXT("D3DCOLORWRITEENABLE_RED | ") );
		if( (rsvalue & D3DCOLORWRITEENABLE_GREEN) != 0 )
			_tcscat( pBuf, TEXT("D3DCOLORWRITEENABLE_GREEN | ") );
		if( (rsvalue & D3DCOLORWRITEENABLE_BLUE) != 0 )
			_tcscat( pBuf, TEXT("D3DCOLORWRITEENABLE_BLUE | ") );
		if( (rsvalue & D3DCOLORWRITEENABLE_ALPHA) != 0 )
			_tcscat( pBuf, TEXT("D3DCOLORWRITEENABLE_ALPHA | ") );
		len = _tcslen( pBuf );
		if( len > 3 )
			pBuf[ len-3 ] = '\0';
		return;
		break;

	case D3DRS_BLENDOP :                    // 171
#ifdef NV_USING_D3D9
	case D3DRS_BLENDOPALPHA :				// 209
#endif
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLENDOP_ADD )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLENDOP_SUBTRACT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLENDOP_REVSUBTRACT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLENDOP_MIN )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DBLENDOP_MAX )
		}
		break;

#ifdef NV_USING_D3D8
	case D3DRS_POSITIONORDER :				// 172
	case D3DRS_NORMALORDER :				// 173
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DORDER_LINEAR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DORDER_CUBIC )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DORDER_QUADRATIC )
		}
		break;
#endif

#ifdef NV_USING_D3D9
	case D3DRS_POSITIONDEGREE :            // 172
	case D3DRS_NORMALDEGREE :			   // 173,
		switch( rsvalue )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DDEGREE_LINEAR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DDEGREE_QUADRATIC )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DDEGREE_CUBIC )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DDEGREE_QUINTIC )
		}
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_SCISSORTESTENABLE,		rsvalue )			// 174
	CASE_BOOLSTRINGIFY( D3DRS_ANTIALIASEDLINEENABLE,		rsvalue )		// 176

	case D3DRS_DEPTHBIAS :					// 195  float value
	case D3DRS_SLOPESCALEDEPTHBIAS :	  // 175	float value
	case D3DRS_MINTESSELLATIONLEVEL :	  // 178
	case D3DRS_MAXTESSELLATIONLEVEL :	  // 179
	case D3DRS_ADAPTIVETESS_X :			  // 180
	case D3DRS_ADAPTIVETESS_Y :			  // 181
	case D3DRS_ADAPTIVETESS_Z :			  // 182
	case D3DRS_ADAPTIVETESS_W :			  // 183
		_sntprintf( pBuf, sz, TEXT("%ff"), *((float*)&rsvalue) );
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_ENABLEADAPTIVETESSELLATION,		rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_TWOSIDEDSTENCILMODE,				rsvalue )

	case D3DRS_BLENDFACTOR :			  // 193
		_sntprintf( pBuf, sz, TEXT("0x%X"), rsvalue );
		return;
		break;

	CASE_BOOLSTRINGIFY( D3DRS_SRGBWRITEENABLE,		rsvalue )
	CASE_BOOLSTRINGIFY( D3DRS_SEPARATEALPHABLENDENABLE,		rsvalue )		// 206
#endif				// NV_USING_D3D9
	}

	if( _tcslen( pBuf ) == 0 )
		_sntprintf( pBuf, sz, TEXT("<D3DRS %u val = %u>"), renderstate, rsvalue ); 
//	FMsg(TEXT("Unknown render state value: %u\n"), rsvalue );
//	assert( false );
}

const TCHAR * GetStrTextureStageState( D3DTEXTURESTAGESTATETYPE type )
{
	switch( type )
	{
	CASE_STRINGIFY( D3DTSS_COLOROP					)
	CASE_STRINGIFY( D3DTSS_COLORARG1				)
	CASE_STRINGIFY( D3DTSS_COLORARG2				)
	CASE_STRINGIFY( D3DTSS_ALPHAOP					)
	CASE_STRINGIFY( D3DTSS_ALPHAARG1				)
	CASE_STRINGIFY( D3DTSS_ALPHAARG2				)
	CASE_STRINGIFY( D3DTSS_BUMPENVMAT00				)
	CASE_STRINGIFY( D3DTSS_BUMPENVMAT01				)
	CASE_STRINGIFY( D3DTSS_BUMPENVMAT10				)
	CASE_STRINGIFY( D3DTSS_BUMPENVMAT11				)
	CASE_STRINGIFY( D3DTSS_TEXCOORDINDEX 			)
	CASE_STRINGIFY( D3DTSS_BUMPENVLSCALE			)
	CASE_STRINGIFY( D3DTSS_BUMPENVLOFFSET 			)
	CASE_STRINGIFY( D3DTSS_TEXTURETRANSFORMFLAGS	)
	CASE_STRINGIFY( D3DTSS_COLORARG0 				)
	CASE_STRINGIFY( D3DTSS_ALPHAARG0 				)
	CASE_STRINGIFY( D3DTSS_RESULTARG 				)
	CASE_STRINGIFY( D3DTSS_CONSTANT				)
	}
	FMsg(TEXT("Unknown TextureStageState : %u\n"), type );
	return( TEXT("(Unknown TextureStageState)") );
}

void GetStrTextureStageStateValue( D3DTEXTURESTAGESTATETYPE type, DWORD value, TCHAR * pBuf, size_t sz )
{
	RET_IF( pBuf == NULL );
	RET_IF( sz == 0 );
	pBuf[0] = '\0';

	switch( type )
	{
	case D3DTSS_COLOROP					:
	case D3DTSS_ALPHAOP					:
		switch( value )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_DISABLE						)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_SELECTARG1					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_SELECTARG2 					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATE					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATE2X					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATE4X					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_ADD							)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_ADDSIGNED					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_ADDSIGNED2X					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_SUBTRACT					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_ADDSMOOTH 					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BLENDDIFFUSEALPHA			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BLENDTEXTUREALPHA 			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BLENDFACTORALPHA			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BLENDTEXTUREALPHAPM			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BLENDCURRENTALPHA			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_PREMODULATE					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATEALPHA_ADDCOLOR		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATECOLOR_ADDALPHA		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATEINVALPHA_ADDCOLOR	)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MODULATEINVCOLOR_ADDALPHA	)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BUMPENVMAP					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_BUMPENVMAPLUMINANCE			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_DOTPRODUCT3 				)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_MULTIPLYADD					)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTOP_LERP						)
		}

	//  These are flags, not strict values, so if combined with a modifier, the value could be unrecognized
	case D3DTSS_COLORARG1				:
	case D3DTSS_COLORARG2				:
	case D3DTSS_ALPHAARG1				:
	case D3DTSS_ALPHAARG2				:
	case D3DTSS_COLORARG0 				:
	case D3DTSS_ALPHAARG0 				:
	case D3DTSS_RESULTARG 				:
		switch( value )
		{
		CASE_STRCPY_STRINGIFY_9( pBuf, sz, D3DTA_CONSTANT		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_CURRENT		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_DIFFUSE		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SELECTMASK	)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SPECULAR		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEMP			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEXTURE		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TFACTOR		)

		CASE_STRCPY_STRINGIFY_9( pBuf, sz, D3DTA_CONSTANT	| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_CURRENT		| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_DIFFUSE		| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SELECTMASK	| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SPECULAR		| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEMP			| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEXTURE		| D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TFACTOR		| D3DTA_ALPHAREPLICATE )

		CASE_STRCPY_STRINGIFY_9( pBuf, sz, D3DTA_CONSTANT	| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_CURRENT		| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_DIFFUSE		| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SELECTMASK	| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SPECULAR		| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEMP			| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEXTURE		| D3DTA_COMPLEMENT )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TFACTOR		| D3DTA_COMPLEMENT )

		CASE_STRCPY_STRINGIFY_9( pBuf, sz, D3DTA_CONSTANT	| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_CURRENT		| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_DIFFUSE		| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SELECTMASK	| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_SPECULAR		| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEMP			| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TEXTURE		| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTA_TFACTOR		| D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE )
		}

	case D3DTSS_BUMPENVMAT00			:
	case D3DTSS_BUMPENVMAT01			:
	case D3DTSS_BUMPENVMAT10			:
	case D3DTSS_BUMPENVMAT11			:
	case D3DTSS_BUMPENVLSCALE			:
	case D3DTSS_BUMPENVLOFFSET 			:
		_sntprintf( pBuf, sz, TEXT("%ff"), *((float*)&value) );
		return;
		break;

	case D3DTSS_TEXCOORDINDEX 			:
#ifdef NV_USING_D3D9
	case D3DTSS_CONSTANT				:
#endif
		_sntprintf( pBuf, sz, TEXT("0x%X"), value );
		return;
		break;

	case D3DTSS_TEXTURETRANSFORMFLAGS	:
		switch( value )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTTFF_DISABLE	)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTTFF_COUNT1		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTTFF_COUNT2		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTTFF_COUNT3		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTTFF_COUNT4		)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTTFF_PROJECTED	)
		}
		break;
	}
	if( _tcslen( pBuf ) == 0 )
		_sntprintf( pBuf, sz, TEXT("<TSS %u val %u>"), type, value );

//	FMsg(TEXT("Unknown TextureStageState value: state= %u   value= %u\n"), type, value );
//	assert( false );
}

#ifdef NV_USING_D3D9
const TCHAR * GetStrSamplerState( D3DSAMPLERSTATETYPE type )
{
	switch( type )
	{
	CASE_STRINGIFY( D3DSAMP_ADDRESSU		)
	CASE_STRINGIFY( D3DSAMP_ADDRESSV		)
	CASE_STRINGIFY( D3DSAMP_ADDRESSW		)
	CASE_STRINGIFY( D3DSAMP_BORDERCOLOR		)
	CASE_STRINGIFY( D3DSAMP_MAGFILTER		)
	CASE_STRINGIFY( D3DSAMP_MINFILTER		)
	CASE_STRINGIFY( D3DSAMP_MIPFILTER		)
	CASE_STRINGIFY( D3DSAMP_MIPMAPLODBIAS	)
	CASE_STRINGIFY( D3DSAMP_MAXMIPLEVEL		)
	CASE_STRINGIFY( D3DSAMP_MAXANISOTROPY	)
	CASE_STRINGIFY( D3DSAMP_SRGBTEXTURE		)
	CASE_STRINGIFY( D3DSAMP_ELEMENTINDEX	)
	CASE_STRINGIFY( D3DSAMP_DMAPOFFSET		)
	}
	FMsg(TEXT("Unknown SamplerState : %u\n"), type );
	return( TEXT("(Unknown SamplerState)") );
}

void GetStrSamplerStateValue( D3DSAMPLERSTATETYPE type, DWORD value, TCHAR * pBuf, size_t sz  )
{
	RET_IF( pBuf == NULL );
	RET_IF( sz == 0 );
	pBuf[0] = '\0';

	switch( type )
	{
	case D3DSAMP_ADDRESSU		:
	case D3DSAMP_ADDRESSV		:
	case D3DSAMP_ADDRESSW		:
		switch( value )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTADDRESS_WRAP )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTADDRESS_MIRROR )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTADDRESS_CLAMP )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTADDRESS_BORDER )
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTADDRESS_MIRRORONCE )
		}
		break;

	case D3DSAMP_BORDERCOLOR	:
		_sntprintf( pBuf, sz, TEXT("0x%X"), value );
		return;
		break;

	case D3DSAMP_MAGFILTER		:
	case D3DSAMP_MINFILTER		:
	case D3DSAMP_MIPFILTER		:
		switch( value )
		{
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTEXF_NONE			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTEXF_POINT			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTEXF_LINEAR			)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTEXF_ANISOTROPIC	)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTEXF_PYRAMIDALQUAD	)
		CASE_STRCPY_STRINGIFY( pBuf, sz, D3DTEXF_GAUSSIANQUAD	)
		}    
		break;

	case D3DSAMP_MIPMAPLODBIAS	:
	case D3DSAMP_SRGBTEXTURE	:
		_sntprintf( pBuf, sz, TEXT("%ff"), *((float*)&value) );
		return;
		break;

	case D3DSAMP_MAXMIPLEVEL	:
	case D3DSAMP_MAXANISOTROPY	:
	case D3DSAMP_ELEMENTINDEX	:
	case D3DSAMP_DMAPOFFSET		:
		_sntprintf( pBuf, sz, TEXT("0x%X"), value );
		return;
		break;
	}
	FMsg(TEXT("Unknown SamplerState value: state= %u  value= %u\n"), type, value );
	_sntprintf( pBuf, sz, TEXT("<SS %u val %u>"), type, value );
//	assert( false );
}

const TCHAR * GetStrVERTEXELEMENT9Type( BYTE Type )
{
	switch( Type )
	{
	CASE_STRINGIFY( D3DDECLTYPE_FLOAT1		)
	CASE_STRINGIFY( D3DDECLTYPE_FLOAT2		)
	CASE_STRINGIFY( D3DDECLTYPE_FLOAT3		)
	CASE_STRINGIFY( D3DDECLTYPE_FLOAT4		)
	CASE_STRINGIFY( D3DDECLTYPE_D3DCOLOR	)
	CASE_STRINGIFY( D3DDECLTYPE_UBYTE4		)
	CASE_STRINGIFY( D3DDECLTYPE_SHORT2		)
	CASE_STRINGIFY( D3DDECLTYPE_SHORT4		)
	CASE_STRINGIFY( D3DDECLTYPE_UBYTE4N		)
	CASE_STRINGIFY( D3DDECLTYPE_SHORT2N		)
	CASE_STRINGIFY( D3DDECLTYPE_SHORT4N		)
	CASE_STRINGIFY( D3DDECLTYPE_USHORT2N	)
	CASE_STRINGIFY( D3DDECLTYPE_USHORT4N	)
	CASE_STRINGIFY( D3DDECLTYPE_UDEC3		)
	CASE_STRINGIFY( D3DDECLTYPE_DEC3N		)
	CASE_STRINGIFY( D3DDECLTYPE_FLOAT16_2	)
	CASE_STRINGIFY( D3DDECLTYPE_FLOAT16_4	)
	CASE_STRINGIFY( D3DDECLTYPE_UNUSED		)
	}
	FMsg(TEXT("Unknown VERTEXELEMENT9 Type : %u\n"), Type );
	assert( false );
	return( TEXT("<UNKNOWN>") );
}

const TCHAR * GetStrVERTEXELEMENT9Method( BYTE Method )
{
	switch( Method )
	{
	CASE_STRINGIFY( D3DDECLMETHOD_DEFAULT			)
	CASE_STRINGIFY( D3DDECLMETHOD_PARTIALU			)
	CASE_STRINGIFY( D3DDECLMETHOD_PARTIALV			)
	CASE_STRINGIFY( D3DDECLMETHOD_CROSSUV			)
	CASE_STRINGIFY( D3DDECLMETHOD_UV				)
	CASE_STRINGIFY( D3DDECLMETHOD_LOOKUP			)
	CASE_STRINGIFY( D3DDECLMETHOD_LOOKUPPRESAMPLED	)
	}
	FMsg(TEXT("Unknown VERTEXELEMENT9 Method : %u\n"), Method );
	assert( false );
	return( TEXT("<UNKNOWN>") );
}

const TCHAR * GetStrVERTEXELEMENT9Usage( BYTE Usage )
{
	switch( Usage )
	{
	CASE_STRINGIFY( D3DDECLUSAGE_POSITION		)
	CASE_STRINGIFY( D3DDECLUSAGE_BLENDWEIGHT	)
	CASE_STRINGIFY( D3DDECLUSAGE_BLENDINDICES	)
	CASE_STRINGIFY( D3DDECLUSAGE_NORMAL			)
	CASE_STRINGIFY( D3DDECLUSAGE_PSIZE			)
	CASE_STRINGIFY( D3DDECLUSAGE_TEXCOORD		)
	CASE_STRINGIFY( D3DDECLUSAGE_TANGENT		)
	CASE_STRINGIFY( D3DDECLUSAGE_BINORMAL		)
	CASE_STRINGIFY( D3DDECLUSAGE_TESSFACTOR		)
	CASE_STRINGIFY( D3DDECLUSAGE_POSITIONT		)
	CASE_STRINGIFY( D3DDECLUSAGE_COLOR			)
	CASE_STRINGIFY( D3DDECLUSAGE_FOG			)
	CASE_STRINGIFY( D3DDECLUSAGE_DEPTH			)
	CASE_STRINGIFY( D3DDECLUSAGE_SAMPLE			)
	}
	FMsg(TEXT("Unknown VERTEXELEMENT9 Usage : %u\n"), Usage );
	assert( false );
	return( TEXT("<UNKNOWN>") );
}

const TCHAR * GetStrD3DPRIMITIVETYPE( D3DPRIMITIVETYPE primt )
{
	switch( primt )
	{
	CASE_STRINGIFY( D3DPT_POINTLIST			)
	CASE_STRINGIFY( D3DPT_LINELIST			)
	CASE_STRINGIFY( D3DPT_LINESTRIP			)
	CASE_STRINGIFY( D3DPT_TRIANGLELIST		)
	CASE_STRINGIFY( D3DPT_TRIANGLESTRIP		)
	CASE_STRINGIFY( D3DPT_TRIANGLEFAN		)
	}
	FMsg( TEXT("Unknown D3DPRIMITIVETYPE : %u\n"), primt );
	return( TEXT("<D3DPRIMITIVETYPE>"));
}

const TCHAR * GetStrD3DQUERYTYPE( D3DQUERYTYPE qt )
{
	switch( qt )
	{
	CASE_STRINGIFY( D3DQUERYTYPE_VCACHE				)
	CASE_STRINGIFY( D3DQUERYTYPE_RESOURCEMANAGER	)
	CASE_STRINGIFY( D3DQUERYTYPE_VERTEXSTATS		)
	CASE_STRINGIFY( D3DQUERYTYPE_EVENT				)
	CASE_STRINGIFY( D3DQUERYTYPE_OCCLUSION			)
	CASE_STRINGIFY( D3DQUERYTYPE_TIMESTAMP			)
	CASE_STRINGIFY( D3DQUERYTYPE_TIMESTAMPDISJOINT	)
	CASE_STRINGIFY( D3DQUERYTYPE_TIMESTAMPFREQ		)
	CASE_STRINGIFY( D3DQUERYTYPE_PIPELINETIMINGS	)
	CASE_STRINGIFY( D3DQUERYTYPE_INTERFACETIMINGS	)
	CASE_STRINGIFY( D3DQUERYTYPE_VERTEXTIMINGS		)
	CASE_STRINGIFY( D3DQUERYTYPE_PIXELTIMINGS		)
	CASE_STRINGIFY( D3DQUERYTYPE_BANDWIDTHTIMINGS	)
	CASE_STRINGIFY( D3DQUERYTYPE_CACHEUTILIZATION	)
	}
	FMsg( TEXT("Unknown D3DQUERYTYPE : %u\n"), qt );
	return( TEXT("<D3DQUERYTYPE>"));
}

//-----------------------------------------------------------------------
// Returns a string based on what D3DCLEAR flags are set in the dword
tstring tstrD3DCLEAR( DWORD flags )
{
	// string big enough to hold all flag names
	int strsz = 20 * 4;
	TCHAR * pc = new TCHAR[strsz];
	RET_VAL_IF( pc == NULL, TEXT("") );
	_tcscpy( pc, TEXT("") );

	FLAG_STRINGIFY( flags, D3DCLEAR_TARGET, pc );
	FLAG_STRINGIFY( flags, D3DCLEAR_ZBUFFER, pc );
	FLAG_STRINGIFY( flags, D3DCLEAR_STENCIL, pc );

	tstring ret;
	ret = pc;
	delete [] pc;
	return( ret );
}

tstring tstrD3DFORMAT( D3DFORMAT format )
{
	tstring tstr = GetStrD3DFORMAT( format );
	return( tstr );
}
tstring tstrD3DLOCK( DWORD flags )
{
	TCHAR buf[ 512 ];
	GetStrD3DLOCK( flags, buf, 512 );
	return( buf );
}
tstring tstrD3DPOOL( D3DPOOL pool )
{
	tstring tstr = GetStrD3DPOOL( pool );
	return( tstr );
}
tstring tstrD3DUSAGE( DWORD usage )
{
	tstring tstr;
	TCHAR b[2048];
	GetStrUsage( usage, b, 2048 );
	tstr = b;
	return( tstr );
}
tstring tstrD3DFVF( DWORD fvf )
{
	tstring tstr;
	TCHAR b[2048];
	GetStrD3DFVF( fvf, b, 2048 );
	tstr = b;
	return( tstr );
}
tstring tstrRenderState( D3DRENDERSTATETYPE renderstate )
{
	return( GetStrRenderState( renderstate ));
}
tstring tstrRenderStateValue( D3DRENDERSTATETYPE renderstate, DWORD value )
{
	tstring tstr = TEXT("");
	TCHAR b[1024];
	GetStrRenderStateValue( renderstate, value, b, 1024 );
	tstr = b;
	return( tstr );
}
tstring tstrTextureStageStateValue( D3DTEXTURESTAGESTATETYPE type, DWORD value )
{
	tstring tstr = TEXT("");
	TCHAR b[1024];
	GetStrTextureStageStateValue( type, value, b, 1024 );
	tstr = b;
	return( tstr );
}
tstring tstrSamplerStateValue( D3DSAMPLERSTATETYPE type, DWORD value )
{
	tstring tstr = TEXT("");
	TCHAR b[1024];
	GetStrSamplerStateValue( type, value, b, 1024 );
	tstr = b;
	return( tstr );
}
tstring tstrD3DMULTISAMPLE_TYPE( D3DMULTISAMPLE_TYPE mst )
{
	tstring tstr = GetStrD3DMULTISAMPLE_TYPE( mst );
	return( tstr );
}
tstring tstrD3DSURFACE_DESC( D3DSURFACE_DESC & desc )
{
	tstring tstr = tstrPrintf( TEXT("W= %u  H= %u  Fmt= %s  Usage= %s  Pool= %s  Type= %s  mst= %s  msq= %u"), 
								desc.Width, desc.Height, 
								tstrD3DFORMAT(desc.Format).c_str(), tstrD3DUSAGE( desc.Usage ).c_str(), 
								tstrD3DPOOL( desc.Pool ).c_str(), 
								GetStrD3DRESOURCETYPE( desc.Type ), 
								tstrD3DMULTISAMPLE_TYPE( desc.MultiSampleType ).c_str(),
								desc.MultiSampleQuality );
	return( tstr );
}

tstring tstrD3DPRIMITIVETYPE( D3DPRIMITIVETYPE primt )
{
	tstring tstr = GetStrD3DPRIMITIVETYPE( primt );
	return( tstr );
}

tstring tstrD3DQUERYTYPE( D3DQUERYTYPE qt )
{
	tstring tstr = GetStrD3DQUERYTYPE( qt );
	return( tstr );
}

//=========================================================
tstring tstrBytes( BYTE * pBytes, size_t num_bytes )
{
	if( pBytes == 0 || num_bytes == 0 )
		return( TEXT(""));
	size_t i; 
	tstring tstr = TEXT("");
	for( i=0; i < num_bytes; i++ )
	{
		if( i%4==0 && i > 0 )
			tstr += TEXT("  ");		// space between DWORDs
		if( i%16==0 && i > 0 )
			tstr += TEXT("\n");		// linefeed after 4 DWORDS
		tstr += tstrPrintf( TEXT("%2.2X"), pBytes[i] );
	}
	return( tstr );
}

tstring tstrShader( IDirect3DVertexShader9 * pShader )
{
	if( pShader == NULL )
		return( TEXT(""));
	UINT uSize;
	pShader->GetFunction( NULL, &uSize );
	vector< DWORD > vDW;
	vDW.resize( uSize / 4, 0 );
	pShader->GetFunction( &(vDW.at(0)), &uSize );
	MSG_AND_RET_VAL_IF( vDW.size() == 0, TEXT("tstrShader() vertex - no bytes!\n"), TEXT("") );
	const DWORD * w = &vDW.at(0);
	tstring tstr = tstrShader(w );
	return( tstr );
}

tstring tstrShader( IDirect3DPixelShader9 * pShader )
{
	if( pShader == NULL )
		return( TEXT(""));
	UINT uSize;
	pShader->GetFunction( NULL, &uSize );
	vector< DWORD > vDW;
	vDW.resize( uSize / 4, 0 );
	pShader->GetFunction( &(vDW.at(0)), &uSize );
	tstring tstr = tstrShader( &(vDW.at(0)) );
	return( tstr );
}

void tstrShaderLines( const tstring & tstrShader, std::vector< tstring > * pvOutLines )
{
	RET_IF( pvOutLines == NULL );
	pvOutLines->clear();
	RET_IF( tstrShader.size() == 0 );
	//tstrTokenize( tstrShader, TEXT("\n\t\r\f"), pvOutLines );
}

tstring tstrShader( const DWORD * pdw )
{
	if( pdw == NULL )
		return( TEXT("") );
	ID3DXBuffer * pD3DXBuf = NULL;
	HRESULT hr;
	hr = D3DXDisassembleShader( pdw, FALSE, NULL, &pD3DXBuf );
	MSG_AND_RET_VAL_IF( FAILED(hr), TEXT("tstrShader( byte* ) disassembly failed!\n"), TEXT("") );
	tstring tstr = TEXT("");
	size_t i;
	char * pc = (char*)(pD3DXBuf->GetBufferPointer());
	wchar_t wc;
	MSG_AND_RET_VAL_IF( pc==NULL, TEXT("tstrShader(byte*) buffer ptr == NULL!\n"), TEXT("") );
	for( i=0; i < pD3DXBuf->GetBufferSize(); i++ )
	{
		mbtowc( &wc, pc+i, 1 );
		tstr += wc;
	}
	SAFE_RELEASE( pD3DXBuf );
	return( tstr );
}


#endif			//  NV_USING_D3D9

//------------------------------------------------------------------------
#ifdef NV_USING_D3D9

UINT GetSizeOfTypeInBytes( const D3DVERTEXELEMENT9 & elem )
{
	return( GetSizeOfTypeInBytes( (D3DDECLTYPE) elem.Type ) );
}

UINT GetSizeOfTypeInBytes( const D3DDECLTYPE & type )
{
	UINT uSize = 0;
	switch( type )
	{
	case D3DDECLTYPE_FLOAT1 :	uSize = 1*sizeof(float);	break;
	case D3DDECLTYPE_FLOAT2 :	uSize = 2*sizeof(float);	break;
	case D3DDECLTYPE_FLOAT3 :	uSize = 3*sizeof(float);	break;
	case D3DDECLTYPE_FLOAT4 :	uSize = 4*sizeof(float);	break;
	case D3DDECLTYPE_D3DCOLOR :	uSize = sizeof(D3DCOLOR);	break;
	case D3DDECLTYPE_UBYTE4 :	uSize = 4*sizeof(BYTE);		break;
	case D3DDECLTYPE_SHORT2 :	uSize = 2*sizeof(short);	break;
	case D3DDECLTYPE_SHORT4 :	uSize = 4*sizeof(short);	break;
	case D3DDECLTYPE_UBYTE4N :	uSize = 4*sizeof(BYTE);		break;
	case D3DDECLTYPE_SHORT2N :	uSize = 2*sizeof(short);	break;
	case D3DDECLTYPE_SHORT4N :	uSize = 4*sizeof(short);	break;
	case D3DDECLTYPE_USHORT2N :	uSize = 2*sizeof(short);	break;
	case D3DDECLTYPE_USHORT4N :	uSize = 4*sizeof(short);	break;
	case D3DDECLTYPE_UDEC3 :	uSize = 0;	break;		// 30 bits
	case D3DDECLTYPE_DEC3N :	uSize = 0;	break;		// 30 bits
	case D3DDECLTYPE_FLOAT16_2: uSize = 4;	break;
	case D3DDECLTYPE_FLOAT16_4: uSize = 8;	break;
	default:
		uSize = 0;
		break;
	}
	return( uSize );
}

UINT GetDimensionOfType( const D3DDECLTYPE & type )
{
	UINT dim;
	switch( type )
	{
	case D3DDECLTYPE_FLOAT1 :	dim = 1;	break;
	case D3DDECLTYPE_FLOAT2 :	dim = 2;	break;
	case D3DDECLTYPE_FLOAT3 :	dim = 3;	break;
	case D3DDECLTYPE_FLOAT4 :	dim = 4;	break;
	case D3DDECLTYPE_D3DCOLOR :	dim = 4;	break;
	case D3DDECLTYPE_UBYTE4 :	dim = 4;	break;
	case D3DDECLTYPE_SHORT2 :	dim = 2;	break;
	case D3DDECLTYPE_SHORT4 :	dim = 4;	break;
	case D3DDECLTYPE_UBYTE4N :	dim = 4;	break;
	case D3DDECLTYPE_SHORT2N :	dim = 2;	break;
	case D3DDECLTYPE_SHORT4N :	dim = 4;	break;
	case D3DDECLTYPE_USHORT2N :	dim = 2;	break;
	case D3DDECLTYPE_USHORT4N :	dim = 4;	break;
	case D3DDECLTYPE_UDEC3 :	dim = 3;	break;		// 30 bits
	case D3DDECLTYPE_DEC3N :	dim = 3;	break;		// 30 bits
	case D3DDECLTYPE_FLOAT16_2: dim = 2;	break;
	case D3DDECLTYPE_FLOAT16_4: dim = 4;	break;
	default:
		dim = 0;
		break;
	}
	return( dim );
}

UINT GetSizeOfFormatInBits( const D3DFORMAT & format )
{
	UINT uSize = 0;
	switch( format )
	{
	case D3DFMT_R8G8B8			:	uSize = 24;	break;
	case D3DFMT_A8R8G8B8		:	uSize = 32;	break;
	case D3DFMT_X8R8G8B8		:	uSize = 32;	break;
	case D3DFMT_R5G6B5			:	uSize = 16; break;
	case D3DFMT_X1R5G5B5		:	uSize = 16;	break;
	case D3DFMT_A1R5G5B5		:	uSize = 16;	break;
	case D3DFMT_A4R4G4B4		:	uSize = 16;	break;
	case D3DFMT_R3G3B2			:	uSize = 8;	break;
	case D3DFMT_A8				:	uSize = 8;	break;
	case D3DFMT_A8R3G3B2		:	uSize = 16;	break;
	case D3DFMT_X4R4G4B4		:	uSize = 16;	break;
	case D3DFMT_A2B10G10R10		:	uSize = 32;	break;
	case D3DFMT_A8B8G8R8		:	uSize = 32;	break;
	case D3DFMT_X8B8G8R8		:	uSize = 32;	break;
	case D3DFMT_G16R16			:	uSize = 32;	break;
	case D3DFMT_A2R10G10B10		:	uSize = 32;	break;
	case D3DFMT_A16B16G16R16	:	uSize = 64;	break;
	case D3DFMT_A8P8			:	uSize = 16;	break;
	case D3DFMT_P8				:	uSize = 8;	break;
	case D3DFMT_L8				:	uSize = 8;	break;
	case D3DFMT_L16				:	uSize = 16;	break;
	case D3DFMT_A8L8			:	uSize = 16;	break;
	case D3DFMT_A4L4			:	uSize = 16;	break;

	case D3DFMT_V8U8			:	uSize = 16;	break;
	case D3DFMT_Q8W8V8U8		:	uSize = 32;	break;
	case D3DFMT_V16U16			:	uSize = 32;	break;
	case D3DFMT_Q16W16V16U16	:	uSize = 64;	break;
	case D3DFMT_CxV8U8 			:	uSize = 16;	break;
	
	case D3DFMT_L6V5U5			:	uSize = 16;	break;
	case D3DFMT_X8L8V8U8		:	uSize = 32;	break;
	case D3DFMT_A2W10V10U10 	:	uSize = 32;	break;
	
	case D3DFMT_MULTI2_ARGB8	:	uSize = 0;	break;	// UNKNOWN
	case D3DFMT_G8R8_G8B8		:	uSize = 16;	break;
	case D3DFMT_R8G8_B8G8		:	uSize = 16;	break;
	case D3DFMT_DXT1			:	uSize = 4;	break;
	case D3DFMT_DXT2			:	uSize = 4;	break;
	case D3DFMT_DXT3			:	uSize = 8;	break;
	case D3DFMT_DXT4			:	uSize = 8;	break;
	case D3DFMT_DXT5			:	uSize = 8;	break;
	case D3DFMT_UYVY			:	uSize = 0;	break;	// UNKNOWN
	case D3DFMT_YUY2 			:	uSize = 0;	break;	// UNKNOWN
	
	case D3DFMT_D16_LOCKABLE	:	uSize = 16;	break;
	case D3DFMT_D32				:	uSize = 32;	break;
	case D3DFMT_D15S1			:	uSize = 16;	break;
	case D3DFMT_D24S8			:	uSize = 32;	break;
	case D3DFMT_D24X8			:	uSize = 32;	break;
	case D3DFMT_D24X4S4			:	uSize = 32;	break;
	case D3DFMT_D32F_LOCKABLE	:	uSize = 32;	break;
	case D3DFMT_D24FS8			:	uSize = 32;	break;
	case D3DFMT_D16				:	uSize = 16;	break;
	case D3DFMT_VERTEXDATA		:	uSize = 0;	break;	// UNKNOWN - vertex buffer surface
	case D3DFMT_INDEX16			:	uSize = 16;	break;
	case D3DFMT_INDEX32 		:	uSize = 32;	break;

	case D3DFMT_R16F			:	uSize = 16;	break;
	case D3DFMT_G16R16F			:	uSize = 32;	break;
	case D3DFMT_A16B16G16R16F 	:	uSize = 64;	break;

	case D3DFMT_R32F			:	uSize = 32;	break;
	case D3DFMT_G32R32F			:	uSize = 64;	break;
	case D3DFMT_A32B32G32R32F	: 	uSize = 128; break;

	default :
		FMsg("D3DFMT_ unrecognized! returning size = 0\n");
		uSize = 0;
	}
	return( uSize );
}

UINT GetShaderSizeInBytes( const DWORD * pFunction )
{
	return( D3DXGetShaderSize( pFunction ) );
}
// Estimates vid mem used for a texture.  Non-pow2 sizes are may or may not reflect the truth 
size_t	GetEstTextureSizeInBytes( UINT Width, UINT Height, UINT Levels, D3DFORMAT Format )
{
	size_t pixels =0;
	UINT lev = 0;
	do
	{
		pixels += Width * Height;
		Width = max( Width / 2, 1 );
		Height = max( Height / 2, 1 );
		lev++;
	}
	while( ((Width > 1) || (Height > 1)) &&
		   ( (Levels==0) || (lev < Levels) ) );

	size_t mems;
	mems = pixels * GetSizeOfFormatInBits( Format );
	mems = mems / 8;		// bits to bytes
	return( mems );
}

// *** This function is not well tested ***
HRESULT ConvertToFloat4( const void *pSrc, D3DDECLTYPE SrcFormat, D3DXVECTOR4 & out_Vec4 )
{
	HRESULT hr = S_OK;
	D3DXVECTOR3 * p3;
	D3DXVECTOR2 * p2;
	float * p1;
	D3DCOLOR * pCol;
	BYTE r,g,b,a;
	short s1, s2, s3, s4;
	unsigned short u1, u2, u3, u4;
	UDEC3 * pud;
	float udec3max = (float)0x03FF;		// 10 bits on
	D3DXVECTOR4 v4;

	switch( SrcFormat )
	{
	case D3DDECLTYPE_FLOAT1 :
		p1 = (float*) pSrc;
		out_Vec4 = D3DXVECTOR4( *p1, 0.0f, 0.0f, 1.0f );
		break;
	case D3DDECLTYPE_FLOAT2 :
		p2 = (D3DXVECTOR2*) pSrc;
		out_Vec4 = D3DXVECTOR4( p2->x, p2->y, 0.0f, 1.0f );
		break;
	case D3DDECLTYPE_FLOAT3 :
		p3 = (D3DXVECTOR3*) pSrc;
		out_Vec4 = D3DXVECTOR4( p3->x, p3->y, p3->z, 1.0f );
		break;
	case D3DDECLTYPE_FLOAT4 :
		out_Vec4 = *((D3DXVECTOR4*)pSrc);
		break;
	case D3DDECLTYPE_D3DCOLOR :
		pCol = (D3DCOLOR*)pSrc;
		// ARGB -> R, G, B, A
		a = (BYTE)(( (*pCol) & 0xFF000000 ) >> 24);
		r = (BYTE)(( (*pCol) & 0x00FF0000 ) >> 16);
		g = (BYTE)(( (*pCol) & 0x0000FF00 ) >> 8);
		b = (BYTE)(( (*pCol) & 0x000000FF ));
		out_Vec4 = D3DXVECTOR4( r/255.0f, g/255.0f, b/255.0f, a/255.0f );
		break;
	case D3DDECLTYPE_UBYTE4 :
		pCol = (D3DCOLOR*)pSrc;
		// ARGB -> B, G, R, A
		a = (BYTE)(( (*pCol) & 0xFF000000 ) >> 24);
		r = (BYTE)(( (*pCol) & 0x00FF0000 ) >> 16);
		g = (BYTE)(( (*pCol) & 0x0000FF00 ) >> 8);
		b = (BYTE)(( (*pCol) & 0x000000FF ));
		out_Vec4 = D3DXVECTOR4( b, g, r, a );
		break;
	case D3DDECLTYPE_UBYTE4N :
		pCol = (D3DCOLOR*)pSrc;
		// ARGB -> B, G, R, A
		a = (BYTE)(( (*pCol) & 0xFF000000 ) >> 24);
		r = (BYTE)(( (*pCol) & 0x00FF0000 ) >> 16);
		g = (BYTE)(( (*pCol) & 0x0000FF00 ) >> 8);
		b = (BYTE)(( (*pCol) & 0x000000FF ));
		out_Vec4 = D3DXVECTOR4( b/255.0f, g/255.0f, r/255.0f, a/255.0f );
		break;

	case D3DDECLTYPE_SHORT2 :
		s1 = ((short*)pSrc)[0];
		s2 = ((short*)pSrc)[1];
		out_Vec4 = D3DXVECTOR4( s1, s2, 0.0f, 1.0f );
		break;
	case D3DDECLTYPE_SHORT4 :
		s1 = ((short*)pSrc)[0];
		s2 = ((short*)pSrc)[1];
		s3 = ((short*)pSrc)[2];
		s4 = ((short*)pSrc)[3];
		out_Vec4 = D3DXVECTOR4( s1, s2, s3, s4 );
		break;
	case D3DDECLTYPE_SHORT2N :
		s1 = ((short*)pSrc)[0];
		s2 = ((short*)pSrc)[1];
		out_Vec4 = D3DXVECTOR4( s1/32767.0f, s2/32767.0f, 0.0f, 1.0f );
		break;
	case D3DDECLTYPE_SHORT4N :
		s1 = ((short*)pSrc)[0];
		s2 = ((short*)pSrc)[1];
		s3 = ((short*)pSrc)[2];
		s4 = ((short*)pSrc)[3];
		out_Vec4 = D3DXVECTOR4( s1/32767.0f, s2/32767.0f, s3/32767.0f, s4/32767.0f );
		break;
	case D3DDECLTYPE_USHORT2N :
		u1 = ((unsigned short*)pSrc)[0];
		u2 = ((unsigned short*)pSrc)[1];
		out_Vec4 = D3DXVECTOR4( u1/65535.0f, u2/65535.0f, 0.0, 1.0f );
		break;
	case D3DDECLTYPE_USHORT4N :
		u1 = ((unsigned short*)pSrc)[0];
		u2 = ((unsigned short*)pSrc)[1];
		u3 = ((unsigned short*)pSrc)[2];
		u4 = ((unsigned short*)pSrc)[3];
		out_Vec4 = D3DXVECTOR4( u1/65535.0f, u2/65535.0f, u3/65535.0f, u4/65535.0f );
		break;

	case D3DDECLTYPE_UDEC3 :
		// 3-D unsigned 10 10 10 format expanded to (value, value, value, 1)
		pud = (UDEC3*)pSrc;
		out_Vec4 = D3DXVECTOR4( (float)pud->n1, (float)pud->n2, (float)pud->n3, 1.0f );
		break;
	case D3DDECLTYPE_DEC3N :
		// 3-D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1). 
		pud = (UDEC3*)pSrc;
		out_Vec4 = D3DXVECTOR4( 2.0f*pud->n1/udec3max - 1.0f, 2.0f*pud->n2/udec3max - 1.0f, 2.0f*pud->n3/udec3max - 1.0f, 1.0f );
		break;

	case D3DDECLTYPE_FLOAT16_2 :
		D3DXFloat16To32Array( &(v4.x), (D3DXFLOAT16*)pSrc, 2 );
		out_Vec4 = D3DXVECTOR4( v4.x, v4.y, 0.0f, 1.0f );
		break;

	case D3DDECLTYPE_FLOAT16_4 :
		D3DXFloat16To32Array( &(v4.x), (D3DXFLOAT16*)pSrc, 4 );
		out_Vec4 = D3DXVECTOR4( v4.x, v4.y, v4.z, v4.w );
		break;

	default :
		FMsg(TEXT("ConvertToFloat4 Unkown or unsupported input type\n"));
		hr = E_FAIL;
		out_Vec4 = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );
		break;
	}
	return( hr );
}

// *** This function is not well tested ***
// pDest must be allocated to sufficient size to hold the output format
HRESULT ConvertFloat4To( const D3DXVECTOR4 & vec4, D3DDECLTYPE DestFormat, void * pDest )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pDest );
	BYTE a, r, g, b;
	float udec3max = (float)0x03FF;		// 10 bits on

	switch( DestFormat )
	{
	case D3DDECLTYPE_FLOAT1 :
		*((float*)pDest) = vec4.x;
		break;
	case D3DDECLTYPE_FLOAT2 :
		*((D3DXVECTOR2*)pDest) = D3DXVECTOR2( vec4.x, vec4.y );
		break;
	case D3DDECLTYPE_FLOAT3 :
		*((D3DXVECTOR3*)pDest) = D3DXVECTOR3( vec4.x, vec4.y, vec4.z );
		break;
	case D3DDECLTYPE_FLOAT4 :
		*((D3DXVECTOR4*)pDest) = vec4;
		break;
	case D3DDECLTYPE_D3DCOLOR :
		r = (BYTE)(vec4.x * 255.0f);
		g = (BYTE)(vec4.y * 255.0f);
		b = (BYTE)(vec4.z * 255.0f);
		a = (BYTE)(vec4.w * 255.0f);
		*((D3DCOLOR*)pDest) = D3DCOLOR_ARGB( a, r, g, b );
		break;
	case D3DDECLTYPE_UBYTE4 :
		r = (BYTE)(vec4.x);
		g = (BYTE)(vec4.y);
		b = (BYTE)(vec4.z);
		a = (BYTE)(vec4.w);
		*((D3DCOLOR*)pDest) = D3DCOLOR_ARGB( a, b, g, r );
		break;
	case D3DDECLTYPE_SHORT2 :
		((short*)pDest)[0] = (short)( vec4.x );
		((short*)pDest)[1] = (short)( vec4.y );
		break;
	case D3DDECLTYPE_SHORT4 :
		((short*)pDest)[0] = (short)( vec4.x );
		((short*)pDest)[1] = (short)( vec4.y );
		((short*)pDest)[2] = (short)( vec4.z );
		((short*)pDest)[3] = (short)( vec4.w );
		break;
	case D3DDECLTYPE_UBYTE4N :
		r = (BYTE)(vec4.x * 255.0f);
		g = (BYTE)(vec4.y * 255.0f);
		b = (BYTE)(vec4.z * 255.0f);
		a = (BYTE)(vec4.w * 255.0f);
		*((D3DCOLOR*)pDest) = D3DCOLOR_ARGB( a, b, g, r );
		break;
	case D3DDECLTYPE_SHORT2N :
		((short*)pDest)[0] = (short)( vec4.x * 32767.0f );
		((short*)pDest)[1] = (short)( vec4.y * 32767.0f );
		break;
	case D3DDECLTYPE_SHORT4N :
		((short*)pDest)[0] = (short)( vec4.x * 32767.0f );
		((short*)pDest)[1] = (short)( vec4.y * 32767.0f );
		((short*)pDest)[2] = (short)( vec4.z * 32767.0f );
		((short*)pDest)[3] = (short)( vec4.w * 32767.0f );
		break;
	case D3DDECLTYPE_USHORT2N :
		((short*)pDest)[0] = (short)( vec4.x * 65535.0f );
		((short*)pDest)[1] = (short)( vec4.y * 65535.0f );
		break;
	case D3DDECLTYPE_USHORT4N :
		((short*)pDest)[0] = (short)( vec4.x * 65535.0f );
		((short*)pDest)[1] = (short)( vec4.y * 65535.0f );
		((short*)pDest)[2] = (short)( vec4.z * 65535.0f );
		((short*)pDest)[3] = (short)( vec4.w * 65535.0f );
		break;

	case D3DDECLTYPE_UDEC3 :
		((UDEC3*)pDest)->n1 = (DWORD) vec4.x;
		((UDEC3*)pDest)->n2 = (DWORD) vec4.y;
		((UDEC3*)pDest)->n3 = (DWORD) vec4.z;
		break;
	case D3DDECLTYPE_DEC3N :
		// 3-D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1). 
		((UDEC3*)pDest)->n1 = (DWORD)(udec3max*(vec4.x + 1.0f)/2.0f);
		((UDEC3*)pDest)->n2 = (DWORD)(udec3max*(vec4.y + 1.0f)/2.0f);
		((UDEC3*)pDest)->n3 = (DWORD)(udec3max*(vec4.z + 1.0f)/2.0f);
		break;

	case D3DDECLTYPE_FLOAT16_2 :
		D3DXFloat32To16Array( (D3DXFLOAT16*)pDest, (float*)&vec4.x, 2 );
		break;
	case D3DDECLTYPE_FLOAT16_4 :
		D3DXFloat32To16Array( (D3DXFLOAT16*)pDest, (float*)&vec4.x, 4 );
		break;

	default :
		FMsg(TEXT("ConvertFloat4To Unkown or unsupported output type\n"));
		hr = E_FAIL;
		break;
	}
	return(hr);
}

HRESULT ConvertData( const void * pSrc, D3DDECLTYPE SrcFormat, void * pDest, D3DDECLTYPE DestFormat )
{
	FAIL_IF_NULL( pSrc );
	FAIL_IF_NULL( pDest );
	HRESULT hr = S_OK;
	D3DXVECTOR4 vec4;
	if( SrcFormat == DestFormat )
	{
		if( SrcFormat == D3DDECLTYPE_UDEC3 ||
			SrcFormat == D3DDECLTYPE_DEC3N )
		{
			*((UDEC3*)pDest) = *((UDEC3*)pSrc);
		}
		else
		{
			memcpy( pDest, pSrc, GetSizeOfTypeInBytes( DestFormat ) );
		}
	}
	else
	{
		hr = ConvertToFloat4( pSrc, SrcFormat, vec4 );
		hr = ConvertFloat4To( vec4, DestFormat, pDest );
	}
	return(hr);
}

#endif			//  NV_USING_D3D9



