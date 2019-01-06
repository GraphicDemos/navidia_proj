#include "TesselatedQuad.hpp"

#ifdef MACOS
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdio.h>

//.----------------------------------------------------------------------------.
//|   Function   : TesselatedQuad::TesselatedQuad                              |
//|   Description: ctor                                                        |
//.----------------------------------------------------------------------------.
TesselatedQuad::TesselatedQuad(int iNumDivisions)
: _iNumDivisions(iNumDivisions)
{
    _CreateData(iNumDivisions);
}


//.----------------------------------------------------------------------------.
//|   Function   : TesselatedQuad::~TesselatedQuad                             |
//|   Description: dtor                                                        |
//.----------------------------------------------------------------------------.
TesselatedQuad::~TesselatedQuad()
{
    delete [] _pVertexData;
    delete [] _pTexCoords;
    delete [] _pIndices;
}


//.----------------------------------------------------------------------------.
//|   Function   : TesselatedQuad::Display                                     |
//|   Description: Draw the quad                                               |
//.----------------------------------------------------------------------------.
void TesselatedQuad::Display()
{
    glVertexPointer(3, GL_FLOAT, 0, _pVertexData);
    glTexCoordPointer(2, GL_FLOAT, 0, _pTexCoords);
    glNormal3f(0, 1, 0);
    
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    // each row of triangles is a separate triangle strip.
    for (unsigned int i = 0; i < _iNumDivisions; i++)
    {
        unsigned int *p = &_pIndices[i * (2 * (_iNumDivisions + 1))];
        glDrawElements( GL_TRIANGLE_STRIP , 2 * (_iNumDivisions + 1), GL_UNSIGNED_INT, p );    
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}


//.----------------------------------------------------------------------------.
//|   Function   : TesselatedQuad::_CreateData                                 |
//|   Description: Creates vertices, indices (for tristrips) and texcoords     |
//.----------------------------------------------------------------------------.
void TesselatedQuad::_CreateData( unsigned int iNumDivisions)
{
    // quad lies in the XZ plane between (-0.5, 0, -0.5) and (0.5, 0, 0.5)
    
    _iNumVertices   = (iNumDivisions + 1) * (iNumDivisions + 1);
    _iNumIndices    = iNumDivisions * 2 * (_iNumDivisions + 1);

    _pVertexData    = new float[_iNumVertices * 3];
    _pTexCoords     = new float[_iNumVertices * 2];
    _pIndices       = new unsigned int[_iNumIndices];

    float pos[]     = {-0.5, 0, 0.5};
    float tex[]     = {0, 0};
    float increment = 1.0f / (float) iNumDivisions;

    int vindex = 0;
    int tindex = 0;
    unsigned int i;
    for (i = 0; i < iNumDivisions + 1; i++)
    {
        for (unsigned int j = 0; j < iNumDivisions + 1; j++)
        {
            _pVertexData[vindex++]  = pos[0];
            _pVertexData[vindex++]  = pos[1];
            _pVertexData[vindex++]  = pos[2];

            _pTexCoords[tindex++]   = tex[0];
            _pTexCoords[tindex++]   = tex[1];

            pos[0] += increment;
            tex[0] += increment;
        }
        pos[0] = -0.5;
        pos[2] -= increment;
        tex[1] += increment;
    }

    int k = 0;
    for (i = 0; i < iNumDivisions; i++)
    {
        int iFirstRowIndex  = i * (iNumDivisions + 1);
        int iSecondRowIndex = (i+1) * (iNumDivisions + 1);

        for (unsigned int j = 0; j < iNumDivisions + 1; j++)
        {
            _pIndices[k++] = iSecondRowIndex++;
            _pIndices[k++] = iFirstRowIndex++;
        }
    }
}
