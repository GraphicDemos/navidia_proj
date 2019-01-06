/*********************************************************************NVMH4****
Path:  SDK\LIBS\src\NV_D3DCommon
File:  MatrixNode.h

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

MatrixV	:
A versioned matrix.  This is a D3D matrix with a version count for tracking
updates and dependencies.  Used in a tree of matrix nodes so that the total
matrices resulting from applying all the matrices in the tree can be calculated
and tracked.


MatrixNode :
An element that can form a tree of matrices.  Contains a handle to a MatrixV.
Several MatrixNodes may share the same MatrixV handle.  For example, several
nodes might share the same projection matrix or camera view matrix.

Create a tree of matrices by stringing nodes together.  Each node will check
the 'version' stamp of the nodes it depends on so that calculations of the 
total matrix for the tree can be avoided if the nodes are already up to date.

The classes are a quick implementation.  It would be good to include features
for computing inverse matrices and possibly transpose matrices using the same
lazy evaluation mechanism.

******************************************************************************/

#ifndef H_NVD3DCOMMON_MATRIXNODE_H
#define H_NVD3DCOMMON_MATRIXNODE_H

// These objects do not allocate or free some of the pointers they hold.
// Your own app or a scene factory manager should take care of tracking
//  what was allocated and what needs to be cleaned up.
// This is done because several nodes may share the same MatrixV

class MatrixV;
class MatrixNode;


/////////////////////////////////////////////

class MatrixV
{
public:
	D3DXMATRIX	m_Matrix;
	DWORD		m_dwUpdateCount;

	MatrixV()							{ D3DXMatrixIdentity(&m_Matrix); m_dwUpdateCount=0; };
	MatrixV( const D3DXMATRIX & mat )	{ m_Matrix = mat; m_dwUpdateCount=0; };
	~MatrixV()							{};

	void MarkAsUpdated();
};



class MatrixNode
{
protected:
									//@@ change to allocate only if required? 
	MatrixV			m_mvTotal;		// local transform on top of parent transform

		// m_dwGlobalUpdateCount is a simple way for all nodes to track updates.
		// Whenever a node changes, this value should be incremented so nodes can check their
		// own update count against this global.  For a tree of many nodes, this single
		// value on it's own will not be a good way to determine when recalculation is needed for a 
		// particular node.  A particular node can still walk it's parents looking at their
		// update counts to determine if and where a change took place.  If no change took place,
		// set the local update count to the global to avoid doing the same walk again in the future
		// until another update increments the global count.
	static DWORD	m_dwGlobalUpdateCount;		// A simple mechanism for tracking when updates are needed.

		// recursuve function:
		// calculates m_mvTotal if necessary
	const D3DXMATRIX	* GetTotalMatrixPtr(  bool * changed  );


public:

	MatrixV		** m_ppLocalMatrix;
	MatrixNode	** m_ppParent;

	MatrixNode( MatrixV ** ppLocalMatrix, MatrixNode ** ppParent );
	virtual ~MatrixNode();

	const D3DXMATRIX	* GetTotalMatrixPtr();		// calculates m_matTotal if necessary
	const D3DXMATRIX	* GetLocalMatrixPtr();		// return m_pLocalMatrix->m_Matrix

	void SetLocalMatrix( const D3DXMATRIX & matrix );		// Modifies matrix held in m_ppLocalMatrix
															// This may affect more than one node if several
															//  nodes shader the same MatrixV
	void SetLocalMatrix( MatrixV ** ppMatrixV );	// Sets node to use different m_ppLocalMatrix
													//  This will modify only this node.

	bool DependsOn( MatrixV * pMatrixToLookFor );

	virtual const char * GetName() { return( "NOT_NAMED" ); };

	friend class MatrixV;
};


class MatrixNodeNamed : public MatrixNode
{
public :
	char * m_pName;

	void	SetName( const char * pName );
	virtual const char * GetName() { return( m_pName ); };

	MatrixNodeNamed( MatrixV ** ppLocalMatrix, MatrixNode ** ppParent );
	MatrixNodeNamed( MatrixV ** ppLocalMatrix, const char * pName, MatrixNode ** ppParent );
	virtual ~MatrixNodeNamed();

};

#endif			// H_NVD3DCOMMON_MATRIXNODE_H
