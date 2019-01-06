
#include "NV_D3DCommonDX9PCH.h"
#include "NV_D3DCommon\Counter.h"

CountedThing::CountedThing()
{
	m_dwIdentifier = 0;
	m_dwCount =0;
	m_fAcum1 = 0.0;
	m_dwAcum1 = 0;
	m_fAcum2 = 0.0;
	m_dwAcum2 = 0;
}
//--------------------
void Counter::Clear()
{
	m_vThings.clear();
}
void Counter::Add( DWORD dwIdentifier, DWORD dwCount, double fVal1, DWORD dwVal1, double fVal2, DWORD dwVal2 )
{
	CountedThing * pt = GetBucket( dwIdentifier );
	if( pt != NULL )
	{
		// if found, increment the counts
		pt->m_dwCount += dwCount;
		pt->m_dwAcum1 += dwVal1;
		pt->m_dwAcum2 += dwVal2;
		pt->m_fAcum1 += fVal1;
		pt->m_fAcum2 += fVal2;
	}
	else
	{
		// not found, so add it!]
		CountedThing ct;
		ct.m_dwIdentifier = dwIdentifier;
		ct.m_dwAcum1 = dwVal1;
		ct.m_dwAcum2 = dwVal2;
		ct.m_dwCount = dwCount;
		ct.m_fAcum1 = fVal1;
		ct.m_fAcum2 = fVal2;
		m_vThings.push_back( ct );
	}	
}
DWORD Counter::GetCount( DWORD dwIdentifier )
{
	CountedThing * pt = GetBucket( dwIdentifier );
	if( pt != NULL )
		return( pt->m_dwCount );
	return( 0 );
}
// sum all counts for all m_dwIdentifiers 
DWORD Counter::GetTotalCounts( CountedThing * pOutTotals )
{
	CountedThing ct;
	size_t i;
	for( i=0; i < m_vThings.size(); i++ )
	{
		ct.m_dwCount	+= m_vThings.at(i).m_dwCount;
		ct.m_dwAcum1	+= m_vThings.at(i).m_dwAcum1;
		ct.m_dwAcum2	+= m_vThings.at(i).m_dwAcum2;
		ct.m_fAcum1		+= m_vThings.at(i).m_fAcum1;
		ct.m_fAcum2		+= m_vThings.at(i).m_fAcum2;
	}

	if( pOutTotals != NULL )
	{
		*pOutTotals = ct;
	}
	return( ct.m_dwCount );
}
// Returns pointer to element in m_vThings or NULL if not found
CountedThing * Counter::GetBucket( DWORD dwIdentifier )
{
	size_t i;
	for( i=0; i < m_vThings.size(); i++ )
	{
		if( m_vThings.at(i).m_dwIdentifier == dwIdentifier )
			return( &(m_vThings.at(i)) );
	}
	return( NULL );
}
