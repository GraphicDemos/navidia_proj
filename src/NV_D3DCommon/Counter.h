
#ifndef H_COUNTER_H
#define H_COUNTER_H

#include <vector>

class CountedThing
{
public:
	DWORD	m_dwIdentifier;
	DWORD	m_dwCount;		// number of things with m_dwIdentifier that have been sent to the counter
	double	m_fAcum1;		// acumulators for various things you might want to sum
	DWORD	m_dwAcum1;
	double	m_fAcum2;
	DWORD	m_dwAcum2;
	CountedThing();
};

class Counter
{
public:
	vector< CountedThing >	m_vThings;
	void	Clear();
	void	Add( DWORD dwIdentifier, DWORD dwCount = 1, double fVal1=0.0, DWORD dwVal1=0, double fVal2=0.0, DWORD dwVal2=0 );
	DWORD	GetCount( DWORD dwIdentifier );
	DWORD	GetTotalCounts( CountedThing * pOutTotals = NULL );	// sum all counts for all m_dwIdentifiers 
	CountedThing * GetBucket( DWORD dwIdentifier );				// returns pointer to element in m_vThings;
};

#endif
