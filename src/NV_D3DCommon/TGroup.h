/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DCommon\
File:  TGroup.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS* AND NVIDIA AND
AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER
INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THIS
SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
Templated class for making groups of things


-------------------------------------------------------------------------------|--------------------*/

#ifndef H_TGROUP_H
#define H_TGROUP_H

#include <tchar.h>
#include <vector>
#include <string>
using namespace std;
#include "shared\NV_Common.h"
#include "shared\NV_Error.h"

typedef std::basic_string<TCHAR> tstring; 

template < class T > class TGroup
{
	vector< T * >		m_vArray;
	tstring				m_tstrName;

public:
	void		FreeElements();
	tstring		GetName()							{ return( m_tstrName ); };
	void		SetName( const tstring & tstrName )	{ m_tstrName = tstrName; };

	size_t		GetNumElements()					{ return( m_vArray.size() ); };
	T *			GetElement( size_t i );
	T *			NewElement();
	T *			NewElementAfter( size_t stInsertAfter );
	T *			NewElement( const T & obj );
	T *			NewElementAfter( const T & obj, size_t stInsertAfter );
	T *			NewElementAt( const T & obj, size_t stInsertAt );

	vector< T* > * GetArray()						{ return( &m_vArray ); };

	TGroup();
	~TGroup();
};

template <class T> TGroup<T>::TGroup()
{
}

template <class T> TGroup<T>::~TGroup()
{
	FreeElements();
}

template <class T> void TGroup<T>::FreeElements()
{
	size_t n;
	for( n=0; n < m_vArray.size(); n++ )
	{
		SAFE_DELETE( m_vArray.at(n) );
	}
	m_vArray.clear();
}

template <class T> T* TGroup<T>::GetElement( size_t i )
{
	if( i >= m_vArray.size() || i < 0 )
		return( NULL );
	return( m_vArray[i] );
}

template <class T> T* TGroup<T>::NewElement()
{
	T * pT = new T;
	if( pT == NULL )
		return( NULL );
	m_vArray.push_back( pT );
	return( pT );
}

template <class T> T* TGroup<T>::NewElement( const T & obj )
{
	T * pT = new T( obj );
	if( pT == NULL )
		return( NULL );
	m_vArray.push_back( pT );
	return( pT );
}

template <class T> T* TGroup<T>::NewElementAfter( size_t stInsertAfter )
{
	T * pT = new T;
	if( pT == NULL )
		return( NULL );
	if( stInsertAt >= m_vArray.size() )
		m_vArray.push_back( pT );
	else
	{
		vector< T* >::iterator i = m_vArray.begin();
		i += stInsertAt;
		m_vArray.insert( i, pT );
	}
	return( pT );
}

template <class T> T* TGroup<T>::NewElementAfter( const T & obj, size_t stInsertAfter )
{
	return( NewElementAt( obj, stInsertAfter+1 ));
}

template <class T> T* TGroup<T>::NewElementAt( const T & obj, size_t stInsertAt )
{
	T * pT = new T( obj );
	if( pT == NULL )
		return( NULL );
	if( stInsertAt >= m_vArray.size() )
		m_vArray.push_back( pT );
	else
	{
		vector< T* >::iterator i = m_vArray.begin();
		i += stInsertAt;
		m_vArray.insert( i, pT );
	}
	return( pT );
}

#endif			// H_TGROUP_H
