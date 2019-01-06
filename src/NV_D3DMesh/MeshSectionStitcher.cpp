/*--------------------------------------------------------------------- NVMH5 -|----------------------
Path:  Sdk\Libs\src\NV_D3DMesh\
File:  MeshSectionStitcher.cpp

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


#define NVD3DMESH_NOLIB
#include "NV_D3DMeshDX9PCH.h"

MeshSectionStitcher::MeshSectionStitcher()
{
}

MeshSectionStitcher::~MeshSectionStitcher()
{
}


HRESULT MeshSectionStitcher::AddToCrossSection( V_MeshVertex * pOut, const V_MeshVertex * pInputSections,
												UINT nvec_in_one_section, UINT n_sections,
												bool stitch_section_end,
												UINT subdiv_cross_section )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pOut );
	FAIL_IF_NULL( pInputSections );

	UINT nov = n_sections * nvec_in_one_section;
	UINT i, sec, n, ind;
	MeshVertex	base, d, first;
	float interp;
	MeshVertex  interpolated;

	pOut->clear();
	ind = 0;

	// Build the output array by interleaving new interpolated
	//   points with the original input points.
	for( sec=0; sec < n_sections; sec++ )
	{
		for( i=0; i < nvec_in_one_section - 1; i++ )
		{
			assert( ind < nov );
			base = pInputSections->at(ind);
			if( i==0 )
				first = base;	// save 1st point for later

			// add original point
			pOut->push_back( pInputSections->at(ind+1) );

			// calculate interpolation factor
			for( n=0; n < subdiv_cross_section; n++ )
			{
				interp = ((float)(n + 1.0f)) / ((float)(subdiv_cross_section + 1.0f));
				interpolated.Interpolate( &interpolated, & pInputSections->at(ind), 
											& pInputSections->at(ind+1), interp );
			}
			ind++;
		}

		if( stitch_section_end )
		{
			assert( ind < nov );
			pOut->push_back( pInputSections->at(ind) );

			// interpolate vertices between the last point of the cross section
			// and the first point of the cross section
			for( n=0; n < subdiv_cross_section; n++ )
			{
				interp = ((float)(n + 1.0f)) / ((float)(subdiv_cross_section + 1.0f));
				interpolated.Interpolate( &interpolated, & pInputSections->at(ind),
											&first, interp );
				pOut->push_back( interpolated );
			}
			ind++;
		}
		else
		{
			// add the last point of the cross section
			assert( ind < nov );
			pOut->push_back( pInputSections->at(ind++) );  // add original point
		}
	}
	return( hr );
}


HRESULT MeshSectionStitcher::AddCrossSections( V_MeshVertex * pOut, const V_MeshVertex * pInputSections,
												UINT nvec_in_section, UINT n_sections,
												bool stitch_extrusion_ends, UINT add_cross_sections )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pOut );
	FAIL_IF_NULL( pInputSections );

	UINT i, n, nadd;
	V_MeshVertex	new_sec;	// new cross section
	UINT sec0, sec1;			// base indices for cross section points we're interpolating between
	MeshVertex newvert;
	float interp;

	pOut->clear();

	for( i=0; i < n_sections; i++ )
	{
		new_sec.clear();
		// Add interpolated vectors to a new cross section
		sec0 = i * nvec_in_section;
		sec1 = (i+1) * nvec_in_section;

		if( i+1 == n_sections )
		{
			sec1 = 0;		// wrap back to start
		}
		// verts between end and start will be deleted later if
		//   the stitch_extrusion_ends is false

		for( nadd = 0; nadd < add_cross_sections; nadd++ )
		{
			// calculate the interpolation factor
			interp = (nadd + 1.0f) / ((float) add_cross_sections + 1.0f );

			// Interpolate and form new section
			// Sections are added one after the other
			for( n=0; n < nvec_in_section; n++ )
			{
				newvert.Interpolate( &newvert, & pInputSections->at(sec0+n),
										& pInputSections->at(sec1+n), interp );
				new_sec.push_back( newvert );
			}
		}

		// Add verts from the existing section before the new interpolated section
		for( n=0; n < nvec_in_section; n++ )
		{
			pOut->push_back( pInputSections->at( sec0 + n ) );
		}
		// Add verts from the new interpolated cross sections
		for( n=0; n < (int)new_sec.size(); n++ )
		{
			pOut->push_back( new_sec[n] );
		}
	}

	// If not wrapping back to start, delete the last added cross sections
	if( ! stitch_extrusion_ends )
	{
		for( n=0; n < (int)new_sec.size(); n++ )
		{
			pOut->pop_back();
		}
	}
	return( hr );
}


// pMesh         is completely overwritten
// pVertices     should already have all cross sections in it
// num_vertices                         is the number of vertices in pVertices array
// num_verts_in_each_cross_section      is the number of verts that make up one single cross section
// num_cross_sections                   is the total number of cross sections in pVertices
// stitch_cross_sections_closed			If true, each cross section is stitched closed into a loop
// stitch_first_cross_section_to_last	If true, the last cross section is stitched to the first
//
HRESULT MeshSectionStitcher::InitExtrusion( Mesh * pMesh,
										    MeshVertex * pVertices, UINT num_vertices,
											UINT num_verts_in_each_cross_section,
											UINT num_cross_sections,
											bool stitch_cross_sections_closed,
											bool stitch_first_cross_section_to_last )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pVertices );

	hr = pMesh->AllocateVertices( num_vertices );
	RET_VAL_IF_FAILED( hr );

	UINT i;
	for( i=0; i < num_vertices; i++ )
	{
		pMesh->m_pVertices[i] = pVertices[i];
	}

	hr = TesselateExtrusion( pMesh, num_verts_in_each_cross_section,
								num_cross_sections,
								stitch_cross_sections_closed,
								stitch_first_cross_section_to_last );

	return( hr );
}

// Vertex data is not effected
// Index data is overwritten
HRESULT MeshSectionStitcher::TesselateExtrusion( Mesh * pMesh,
												 UINT num_verts_in_each_cross_section,
												 UINT num_cross_sections,
												 bool stitch_cross_sections_closed,
												 bool stitch_first_cross_section_to_last )
{
	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	RET_VAL_IF( num_verts_in_each_cross_section < 2, E_FAIL );
	RET_VAL_IF( num_cross_sections < 2, E_FAIL );

	UINT nvert;
	nvert = num_verts_in_each_cross_section * num_cross_sections;
	if( pMesh->GetNumVertices() != nvert )
	{
		FMsg(TEXT("TesselateExtrusion(..) WARNING : Mesh does not have expected number of vertices\n"));
		FMsg(TEXT("    Mesh has %u vertices, tesselation pattern expecting %u vertices\n"), pMesh->GetNumVertices(), nvert );
	}

	UINT i, j, nind;
	UINT nquad_sec;			// number of quads per section
	UINT nsecs_to_stitch;	// number of sections to stitch

	// start filling in data, modifying as appropriate given the bools for closing each type of end
	nquad_sec =  num_verts_in_each_cross_section - 1;	
	// If stitching each cross section closed, you get 2 more tris in each section,
	//  which is one more quad.
	if( stitch_cross_sections_closed )
	{
		nquad_sec += 1;
	}

	nsecs_to_stitch = num_cross_sections - 1;
	// If stitching first cross section to the last, you have one more section of quads
	if( stitch_first_cross_section_to_last )
	{
		nsecs_to_stitch += 1;
	}

	// number of triangles is the number of quads * 2
	// number of indices is the number of tris * 3
	nind = nquad_sec * nsecs_to_stitch * 2 * 3;

	// allocate memory for indices
	pMesh->AllocateIndices( nind );
	pMesh->m_PrimType = D3DPT_TRIANGLELIST;		// Could optimize to create tri strips, but not today!
	
	// Stitch the tris
	UINT ind;			// index location to fill with a vertex index
	UINT v1, v2, v3;	// vertex indices of a triangle
	UINT jplus, iplus;	// j+1, i+1 wrapped back to previous vertex indices if the 
						// various stitch_ bools are true
	ind = 0;
	for( j=0; j < nsecs_to_stitch; j++ )
	{
		for( i=0; i < nquad_sec; i++ )
		{
			// Find the coords of the next vertices to form the tri
			iplus = i + 1;
			if( iplus == num_verts_in_each_cross_section )
				iplus = 0;
			jplus = j + 1;
			if( jplus == num_cross_sections )
				jplus = 0;

			// CCW winding order
			v1 = j		* num_verts_in_each_cross_section + i;
			v2 = jplus	* num_verts_in_each_cross_section + i;
			v3 = j		* num_verts_in_each_cross_section + iplus;

			assert( v1 < nvert );
			assert( v2 < nvert );
			assert( v3 < nvert );
			assert( ind < nind-2);
			pMesh->m_pIndices[ ind++ ] = v1;
			pMesh->m_pIndices[ ind++ ] = v2;
			pMesh->m_pIndices[ ind++ ] = v3;

			// second triangle of the pair
			v1 = v3;
			v2 = v2;
			v3 = jplus * num_verts_in_each_cross_section + iplus;
			assert( v3 < nvert );
			assert( ind < nind-2);
			pMesh->m_pIndices[ ind++ ] = v1;
			pMesh->m_pIndices[ ind++ ] = v2;
			pMesh->m_pIndices[ ind++ ] = v3;
		}
	}
	pMesh->m_bIsValid = true;
	return( hr );
}


HRESULT MeshSectionStitcher::InitExtrusion( Mesh * pMesh, 
										    D3DXVECTOR3 * pPos, D3DXVECTOR3 * pNrm, D3DCOLOR * pCol,
											D3DXVECTOR2 * pTexCoord,
											UINT num_verts_in_each_cross_section,
											UINT num_cross_sections,
											bool stitch_cross_sections_closed,
											bool stitch_first_cross_section_to_last )
{
	//
	// Would be better to create a tri strip instead of an indexed tri list =)
	//
	// stitch_section_end:		make tris to close each cross section into a tube
	//							This doesn't cap the ends - just closes the cross sections
	// stitch_extrusion_ends:	stitch the first cross section to the last to close
	//							  a tube or wrap a ribbon back to its start
	//
	// pPos			Required
	// pNrm			Not required
	// pCol			Not required
	// pTexCoord	Not required
	//
	// num_cross_sections is the number of cross sections in the input arrays
	// The size of the arrays should be num_verts_in_cross_section * num_cross_sections.
	//
	// For the vertex attributes, specify one cross section after another, so if
	//  num_verts_in_cross_section = 4, then pPos[0]..[3] are positions for first 
	//  cross section, pPos[4]..[7] are positions for 2nd cross section
	// All cross sections must have the same number of vertices!

	FAIL_IF_NULL( pMesh );
	HRESULT hr = S_OK;

	UINT nvert;
	nvert = num_verts_in_each_cross_section * num_cross_sections;

	pMesh->AllocateVertices( nvert );

	UINT i;
	for( i=0; i < nvert; i++ )
	{
		pMesh->m_pVertices[i].pos = pPos[i];
	}
	if( pNrm != NULL )
	{
		for( i=0; i < nvert; i++ )
		{
			pMesh->m_pVertices[i].nrm = pNrm[i];
		}
	}
	if( pCol != NULL )
	{
		for( i=0; i < nvert; i++ )
		{
			pMesh->m_pVertices[i].diffuse = pCol[i];
		}
	}
	if( pTexCoord != NULL )
	{
		for( i=0; i < nvert; i++ )
		{
			pMesh->m_pVertices[i].t0 = pTexCoord[i];
		}
	}

	hr = TesselateExtrusion( pMesh, num_verts_in_each_cross_section,
								num_cross_sections,
								stitch_cross_sections_closed,
								stitch_first_cross_section_to_last );
	return( hr );
}


// Interpolate the texture coordinate range along the vertex array
// that is input.  pInputVertices[0] will have texcoord start_coord.
// and pInputVertices[num_verts-1] will have texcoord end_coord.
HRESULT MeshSectionStitcher::InterpolateTexcoords( MeshVertex * pInputVertices,
												   UINT num_verts,
												   D3DXVECTOR2 & start_coord,
												   D3DXVECTOR2 & end_coord )
{
	FAIL_IF_NULL( pInputVertices );
	HRESULT hr = S_OK;
	if( num_verts == 1 )
	{
		pInputVertices[0].t0 = start_coord;
		return( hr );
	}
	UINT i;
	float interp;
	for( i=0; i < num_verts; i++ )
	{
		interp = ((float) i) / ((float)(num_verts - 1) );
		D3DXVec2Lerp( &(pInputVertices[i].t0), &start_coord, &end_coord, interp );
	}
	return( hr );
}


HRESULT MeshSectionStitcher::InitLathedObject( Mesh * pMesh,
											  MeshVertex * pInputVertices,  // single cross section to sweep
											  UINT num_verts_in_cross_section,
											  D3DXVECTOR3 & lathe_axis,
											  float lathe_angle_start,	// in degrees
											  float lathe_angle_end,	// in degrees
											  UINT num_cross_sections,	// number in final object
											  bool close_cross_section_ends,
											  bool stitch_last_cross_section_to_first,
											  bool generate_texcoordx_on_input_cross_section )
{
	// pInputVertices points to single cross section that will be swept around the axis
	// Other vertices are created for the lathe process
	// lathe center is always zero.
	// num_cross_sections is total number of cross sections.  Note that if 
	//  angle is 360 all the way around, then last cross section will overlap
	//  the first.  To avoid this, set stitch_last_cross_section_to_first = true
	//  and set angle difference to 360 * ( 1 - 1 / num_cross_sections )
	// Texture coordinates are generated in the .y axis for each lathed cross section created.
	// generate_texcoordx_on_input_cross_section determines if the texture coords on the 
	//  input pInputVertices will be overwritten in .x direction to range from [0,1]
	// If pTexCoord is not NULL, the .x coord will be used along each cross section,
	//  and the .y coord will be overridden to sweep from 0 to 1 around the shape.
	// If pTexCoord is NULL, tex coords will be created from 0 to 1 in .x along the 
	//  cross section, and from 0 to 1 in .y around the sweep axis from 1st to last cross section.

	HRESULT hr = S_OK;
	FAIL_IF_NULL( pMesh );
	FAIL_IF_NULL( pInputVertices );
	RET_VAL_IF( num_cross_sections < 2, E_FAIL );
	RET_VAL_IF( num_verts_in_cross_section < 2, E_FAIL );

	if( lathe_angle_end < lathe_angle_start )
	{
		float tmp = lathe_angle_end;
		lathe_angle_end = lathe_angle_start;
		lathe_angle_start = tmp;
	}

	if( generate_texcoordx_on_input_cross_section )
	{
		InterpolateTexcoords( pInputVertices, num_verts_in_cross_section,
								D3DXVECTOR2( 0.0f, 0.0f ), D3DXVECTOR2( 1.0f, 0.0f ) );
	}


	UINT i, vind;

	// copy the input cross section to a vector of vertices
	V_MeshVertex	vFirstCrossSection;
	for( i=0; i < num_verts_in_cross_section; i++ )
	{
		vFirstCrossSection.push_back( pInputVertices[i] );
	}

	V_MeshVertex	vVertices;	// vector of all vertices in the lathed object

	// Add all new cross sections to vVertices
	AddCrossSections( & vVertices, & vFirstCrossSection, num_verts_in_cross_section,
						1,		// 1 cross section
						true,
						num_cross_sections - 1 );	// add this number of new cross sections

	assert( vVertices.size() == num_verts_in_cross_section * num_cross_sections );

	// generate texcoord .y from [0,1] from first cross section to last.
	// each cross section has the same .y texcoord.  The last cross section has the 
	// .y coord = 1.0f
	float txy;
	for( i=0; i < vVertices.size(); i++ )
	{
		vind = i % num_verts_in_cross_section;
		txy = (float) floor((double)( (float)i / (float)num_verts_in_cross_section) ) / (num_cross_sections - 1.0f );
		vVertices.at(i).t0.y = txy;
	}

	// Rotate each cross section to form the lathed points
	D3DXMATRIX rot;
	UINT n;
	float ang_deg;
	float ang_radians;
	float interp;

	for( i = 0; i < num_cross_sections; i++ )
	{
		interp = (float) i / ((float)(num_cross_sections - 1));
		ang_deg = lathe_angle_start + ( lathe_angle_end - lathe_angle_start ) * interp;
		ang_radians = (float)(ang_deg * NVMESH_PI / 180.0f);
		D3DXMatrixRotationAxis( & rot, & lathe_axis, ang_radians );

		for( n=0; n < num_verts_in_cross_section; n++ )
		{
			vind = i * num_verts_in_cross_section + n;
			assert( vind < vVertices.size() );
			D3DXVec3TransformCoord( & (vVertices.at(vind).pos), &(vVertices.at(vind).pos), & rot );

			// Transform the vertex normal
			// We are doing pure rotation, so there is no need to use D3DXVec3TransformNormal or
			//  to use the inverse transpose matrix on the normal.
			D3DXVec3TransformCoord( & (vVertices.at(vind).nrm), &(vVertices.at(vind).nrm), & rot );
		}
	}

	// Stitch verts together as an extrusion object to make the lathed shape
	hr = InitExtrusion( pMesh, &(vVertices[0]), (UINT)(vVertices.size()),
						num_verts_in_cross_section,
						num_cross_sections,
						close_cross_section_ends,
						stitch_last_cross_section_to_first );

	pMesh->m_PrimType = D3DPT_TRIANGLELIST;
	pMesh->m_bIsValid = true;
	return( hr );
}





