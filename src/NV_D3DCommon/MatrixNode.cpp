/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\NV_D3DCommon
File:  MatrixNode.cpp

Copyright NVIDIA Corporation 2002
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
See the .h for comments


******************************************************************************/

#include "NV_D3DCommonDX9PCH.h"

// debug message diagnostics - Define as 1 to enable
#define GTMP_VERBOSE	0


// static members
DWORD	MatrixNode::m_dwGlobalUpdateCount = 0;

//---------------------------------------------

void MatrixV::MarkAsUpdated()
{
	m_dwUpdateCount++;
	MatrixNode::m_dwGlobalUpdateCount ++;
}


MatrixNode::MatrixNode( MatrixV ** ppLocalMatrix, MatrixNode ** ppParent )
{
	D3DXMatrixIdentity( & (m_mvTotal.m_Matrix));

	// force an update the next time matrices are querried
	m_mvTotal.m_dwUpdateCount = m_dwGlobalUpdateCount - 1;

	m_ppLocalMatrix	= ppLocalMatrix;
	m_ppParent		= ppParent;
}

MatrixNode::~MatrixNode()
{
	D3DXMatrixIdentity( & (m_mvTotal.m_Matrix));
	m_ppLocalMatrix = NULL;
	m_ppParent = NULL;
}



const D3DXMATRIX	* MatrixNode::GetTotalMatrixPtr()
{
	// Start traversal of the matrix tree using the private function.
	// This needs to know when nodes have changed.  It uses the
	//  update counts to only calculate things when needed.

	bool changed;
	return( GetTotalMatrixPtr( & changed ));
}



const D3DXMATRIX	* MatrixNode::GetTotalMatrixPtr( bool * pbChanged )
{	
	// This is the heart of the lazy evaluation of the tree of matrices
	//  to find the total matrix at this node.  The total matrix is
	//  this node's local matrix applied to the total matrix of the
	//  parent node.
	// Checks if m_mvTotal needs to be updated, updates it
	//  if required, and returns a pointer to it's matrix.
	//
	// pbChanged - Set the bool to true if the node was changed.

	if( pbChanged == NULL )
	{
		FMsg("MatrixNode::GetTotalMatrixPtr( bool * ) pointer is NULL.  Please supply a bool!\n");
		return( NULL );
	}

	
	#if GTMP_VERBOSE
		FDebug("MatrixNode::GetTotalMatrixPtr() for node:  %x\n", (DWORD)this );
	#endif


	bool locnull = false;
	if( m_ppLocalMatrix == NULL )
	{
		locnull = true;
	}
	else if( *m_ppLocalMatrix == NULL )
	{
		locnull = true;
	}

	if( locnull )
	{
		FDebug("Some MatrixNode has a NULL local matrix!\n");
		FDebug("  Trying to set it's total to that of its parent!\n");

		bool parentnull = false;
		if( m_ppParent == NULL )
		{
			parentnull = true;
		}
		else if( *m_ppParent == NULL )
		{
			parentnull = true;
		}

		if( parentnull )
		{
			FDebug("   Also no parent - setting to identity!\n");
			D3DXMATRIX tmat;
			D3DXMatrixIdentity( &tmat );

			// if the matrix is already identity, then it wasn't changed
			if( m_mvTotal.m_Matrix == tmat )
			{
				m_mvTotal.m_dwUpdateCount = m_dwGlobalUpdateCount;
				*pbChanged = false;
			}
			else
			{
				m_mvTotal.m_Matrix = tmat;
				m_mvTotal.m_dwUpdateCount = m_dwGlobalUpdateCount;
				*pbChanged = true;
			}
		}
		else
		{
			// If parent not null
			// Set local matrix to that of it's parent
			bool parents_matrix_changed;
			const D3DXMATRIX * pParM = (*m_ppParent)->GetTotalMatrixPtr( & parents_matrix_changed );

			if( pParM == NULL )
			{	
				FDebug("Parent's total matrix ptr == NULL, returning NULL\n");
				assert( false );	// shouldn't be null
				return( NULL );
			}

			// If the parent's matrix was changed or our update count doesn't match parent's
			//  change our total matrix to that of the parents (our local matrix is NULL) and
			//  return.
			if( parents_matrix_changed || 
				( m_mvTotal.m_dwUpdateCount != (*m_ppParent)->m_mvTotal.m_dwUpdateCount ) )
			{
				m_mvTotal.m_Matrix = *pParM;
				m_mvTotal.m_dwUpdateCount = (*m_ppParent)->m_mvTotal.m_dwUpdateCount;
				*pbChanged = true;
			}
			else
			{
				*pbChanged = false;
			}
		}
		return( &(m_mvTotal.m_Matrix) );
	}


	*pbChanged = false;


	if( ( m_mvTotal.m_dwUpdateCount == m_dwGlobalUpdateCount ) &&
		( (*m_ppLocalMatrix)->m_dwUpdateCount == m_mvTotal.m_dwUpdateCount ) )	
	{
		// if we're current and parent is current, return the m_mvTotal as it is, 
		//  knowing that it's already been updated

		#if GTMP_VERBOSE
			FDebug("  Node has already been calculated.  Update count = %u\n", m_mvTotal.m_dwUpdateCount );
		#endif
		*pbChanged = false;
		return( & (m_mvTotal.m_Matrix) );
	}

	bool parentnull = false;
	if( m_ppParent == NULL )
	{
		parentnull = true;
	}
	else if( *m_ppParent == NULL )
	{
		parentnull = true;
	}

	if( parentnull )
	{
		// We have no parent, so (*m_ppLocalMatrix)->m_Matrix holds
		//  the current updated matrix
		// *m_ppLocalMatrix is safe as we've handled the NULL case above

		if( (*m_ppLocalMatrix)->m_dwUpdateCount != m_mvTotal.m_dwUpdateCount )
		{
			// copy the matrix and the m_dwUpdateCount
			m_mvTotal = ** m_ppLocalMatrix;
			*pbChanged = true;

			#if GTMP_VERBOSE
				FDebug("  No parent - UPDATED total matrix: %u\n", m_dwGlobalUpdateCount );
			#endif
		}
		else
		{
			#if GTMP_VERBOSE
				FDebug("  No parent - total matrix is current: %u\n", m_mvTotal.m_dwUpdateCount );
			#endif
		}

		// We're up to date, so mark us with current global count so we don't 
		//  have to do so much work next time.
		(*m_ppLocalMatrix)->m_dwUpdateCount = m_dwGlobalUpdateCount;
		m_mvTotal.m_dwUpdateCount			= m_dwGlobalUpdateCount;
	}
	else
	{
		// We need to check the parent node and make sure
		//  it is up to date.

		bool parent_changed = false;
		const D3DXMATRIX * pParM = (*m_ppParent)->GetTotalMatrixPtr( & parent_changed );
		assert( pParM != NULL );	// shouldn't be null

		if( parent_changed || ((*m_ppLocalMatrix)->m_dwUpdateCount != m_mvTotal.m_dwUpdateCount) )
		{
			// parent node was changed, or our own local
			//  matrix was changed, so update our total 
			//  matrix by multiplying the local matrix with
			//  the parent's matrix.
	
			// multiply our local matrix by the parent matrix 
			// multiply our matrix on the right, as it is applied
			//  after the parent's matrix.

			D3DXMatrixMultiply( &(m_mvTotal.m_Matrix), pParM, &((*m_ppLocalMatrix)->m_Matrix) );

			(*m_ppLocalMatrix)->m_dwUpdateCount = m_dwGlobalUpdateCount;
			m_mvTotal.m_dwUpdateCount			= m_dwGlobalUpdateCount;
			*pbChanged = true;

			#if GTMP_VERBOSE
				FDebug("  Has parent - Parent updated: %d   New matrix calculated: %u\n", loc_ch, m_mvTotal.m_dwUpdateCount );
			#endif
		}
		else
		{
			// parent didn't change and we're up to date, so don't need to do anything
			#if GTMP_VERBOSE
				FDebug("  Has parent - No update required: %u\n", m_mvTotal.m_dwUpdateCount );
			#endif
			// No need to set *pbChanged = false.  That was done above
		}
	}

	return( & (m_mvTotal.m_Matrix) );
}


bool MatrixNode::DependsOn( MatrixV * pMatrixToLookFor )
{
	if( (*m_ppLocalMatrix) == pMatrixToLookFor )
	{
		return( true );
	}
	else if( m_ppParent != NULL )
	{
		if( *m_ppParent != NULL )
		{
			return( (*m_ppParent)->DependsOn( pMatrixToLookFor ) );
		}
	}

	return( false );
}




const D3DXMATRIX	* MatrixNode::GetLocalMatrixPtr()
{
	// Return the matrix held in the MatrixV this node may point to, 
	// or NULL if the handle/pointer isn't there.

	D3DXMATRIX * pm = NULL;

	if( m_ppLocalMatrix != NULL )
	{
		if( *m_ppLocalMatrix != NULL )
		{
			pm = & ((*m_ppLocalMatrix)->m_Matrix);
		}
	}

	return( pm );
}


void MatrixNode::SetLocalMatrix( const D3DXMATRIX & matrix )
{
	// Modifies matrix held in m_pLocalMatrix
	// This may affect more than one node if several
	//  nodes share the same MatrixV
	//
	// This does not recompute the matrix chain.  The computation
	//  of total matrices is done only when a specific node is
	//  querried with Get..().

	if( m_ppLocalMatrix == NULL )
	{
		FDebug("Some MatrixNode has a NULL local matrix handle!\n");
		FDebug("  Setting the node's local total matrix to identity!\n");
		assert( false );
		D3DXMatrixIdentity( &(m_mvTotal.m_Matrix) );
		m_mvTotal.m_dwUpdateCount = m_dwGlobalUpdateCount;
		return;
	}
	if( *m_ppLocalMatrix == NULL )
	{
		FDebug("Some MatrixNode has a NULL local matrix handle!\n");
		FDebug("  Setting the node's local total matrix to identity!\n");
		assert( false );
		D3DXMatrixIdentity( &(m_mvTotal.m_Matrix) );
		m_mvTotal.m_dwUpdateCount = m_dwGlobalUpdateCount;
		return;
	}

	(*m_ppLocalMatrix)->m_Matrix = matrix;
	(*m_ppLocalMatrix)->m_dwUpdateCount++;

	// Increment global count so nodes know they have to check
	//  their parents
	m_dwGlobalUpdateCount++;
}


void MatrixNode::SetLocalMatrix( MatrixV ** ppMatrixV )
{
	// Sets node to use different m_pLocalMatrix
	//  This will modify only this node.

	m_ppLocalMatrix = ppMatrixV;

	// mark the local matrix as having been updated.
	if( m_ppLocalMatrix != NULL )
	{
		if( *m_ppLocalMatrix != NULL )
		{
			(*m_ppLocalMatrix)->m_dwUpdateCount++;
		}
	}
	m_dwGlobalUpdateCount++;
};

/////////////////////////////////////////////


MatrixNodeNamed::MatrixNodeNamed( MatrixV ** ppLocalMatrix, MatrixNode ** ppParent )
 : MatrixNode( ppLocalMatrix, ppParent )
{
	m_pName = NULL;
	SetName("UNNAMED");
}


MatrixNodeNamed::MatrixNodeNamed( MatrixV ** ppLocalMatrix,
									const char * pName, 
									MatrixNode ** ppParent )
 : MatrixNode( ppLocalMatrix, ppParent )
{
	m_pName = NULL;
	SetName( pName );
}

MatrixNodeNamed::~MatrixNodeNamed()
{
	SAFE_ARRAY_DELETE( m_pName );
}


void MatrixNodeNamed::SetName( const char * pName )
{
	// Delete name if it's not NULL
	SAFE_ARRAY_DELETE( m_pName );

	size_t len = strlen( pName );
	if( len > 0 && len < 10000 )
	{
		m_pName = new char[ len + 3 ];	// +3 for terminating chars.  3 to be safe
		RET_IF_NULL( m_pName );
		strcpy( m_pName, pName );
	}
	else
	{
		m_pName = new char[ 4 ];
		RET_IF_NULL( m_pName );
		strcpy( m_pName, " " );
	}
}








