#ifndef __TESSELATEDQUAD_HPP__
#define __TESSELATEDQUAD_HPP__
//.----------------------------------------------------------------------------.
//| This class just creates and displays a quad that is tesselated into as     |
//| many divisions as you specify.  Renders using tri-strip vertex arrays      |
//.----------------------------------------------------------------------------.
class TesselatedQuad
{
public:
    TesselatedQuad(int iNumDivisions);
    ~TesselatedQuad();

    void Display();

private:
    unsigned int    _iNumDivisions;
    unsigned int    _iNumVertices;
    unsigned int    _iNumIndices;

    unsigned int    *_pIndices;
    float           *_pVertexData;
    float           *_pTexCoords;

    void _CreateData(unsigned int iNumDivisions);
};

#endif //__TESSELATEDQUAD_HPP__
