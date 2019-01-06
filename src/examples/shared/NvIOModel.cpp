// Simple wrapper to load objects using nvIO and render them using GL

#include <nvIO/nvObjReader.h>
#include <nvIO/NVMeshMender.h>
#include <math.h>
#include <stdlib.h>

#if defined(WIN32)
#  include <windows.h>
#endif
#include <GL/gl.h>

#include <shared/NvIOModel.h>

bool
NvIOModel::ReadOBJ(char *filename)
{
  nvObjReader* reader = new nvObjReader();
  if (!reader->ReadFile(filename, 0))
    return false;

  std::vector<int> triIndices;
  if (!reader->GetTriangleIndices(triIndices))
	  return false;

  std::vector<float> vpos;
  if (!reader->GetAttribute(nvMeshAttribute::VERTEX_POSITION,vpos))
	  return false;

  std::vector<float> vnor;
  bool normals = false;
  if (reader->GetAttribute(nvMeshAttribute::VERTEX_NORMAL,vnor))
	  normals = true;

  std::vector<float> vtex;
  bool textures = false;
  if (reader->GetAttribute(nvMeshAttribute::TEX_COORD0,vtex))
	  textures = true;

  m_nverts   = vpos.size() / 3;
  m_nindices = triIndices.size();

  std::vector<NVMeshMender::VertexAttribute> inputAtts;
  std::vector<NVMeshMender::VertexAttribute> outputAtts;

  NVMeshMender::VertexAttribute posAtt;
  posAtt.Name_ = "position";
  posAtt.floatVector_ = vpos;

  NVMeshMender::VertexAttribute triIndAtt;
  triIndAtt.Name_ = "indices";
  triIndAtt.intVector_ = triIndices;

  NVMeshMender::VertexAttribute norAtt;
  norAtt.Name_ = "normal";
  norAtt.floatVector_ = vnor;

  NVMeshMender::VertexAttribute texCoordAtt;
  texCoordAtt.Name_ = "tex0";
  texCoordAtt.floatVector_ = vtex;

  std::vector<float> vtan;
  NVMeshMender::VertexAttribute tgtSpaceAtt;
  tgtSpaceAtt.Name_ = "tangent";
  tgtSpaceAtt.floatVector_ = vtan;

  std::vector<float> vbin;
  NVMeshMender::VertexAttribute binSpaceAtt;
  binSpaceAtt.Name_ = "binormal";
  binSpaceAtt.floatVector_ = vbin;
  
  inputAtts.push_back(posAtt);
  inputAtts.push_back(triIndAtt);
  if (normals)
	  inputAtts.push_back(norAtt);
  if (textures)
	  inputAtts.push_back(texCoordAtt);
  
  outputAtts.push_back(posAtt);
  outputAtts.push_back(triIndAtt);
  outputAtts.push_back(norAtt);
  outputAtts.push_back(texCoordAtt);
  outputAtts.push_back(tgtSpaceAtt);
  outputAtts.push_back(binSpaceAtt);
  
  NVMeshMender mger;
  mger.Munge(inputAtts,outputAtts, 3.141592654f / 2.0f, 0, NVMeshMender::FixTangents);

  
  m_nverts = outputAtts[ 0 ].floatVector_.size() / 3;

  m_indices   = new unsigned int[m_nindices];
  m_positions = new vec3f[m_nverts];
  m_normals   = new vec3f[m_nverts];
  m_tangents  = new vec3f[m_nverts];
  m_binormals = new vec3f[m_nverts];
  m_texcoords = new vec3f[m_nverts];

  if ( m_indices == NULL ||
       m_positions == NULL ||
       m_normals == NULL ||
       m_tangents == NULL ||
       m_binormals == NULL ||
       m_texcoords == NULL )
  {
      return false;
  }

  memcpy( m_positions, &(outputAtts[0].floatVector_[0]), m_nverts*3*sizeof(float) );
  memcpy( m_indices,   &(outputAtts[1].intVector_[0]),   m_nindices*sizeof(int) );
  memcpy( m_normals,   &(outputAtts[2].floatVector_[0]), m_nverts*3*sizeof(float) );
  memcpy( m_texcoords, &(outputAtts[3].floatVector_[0]), m_nverts*3*sizeof(float) );
  memcpy( m_tangents,  &(outputAtts[4].floatVector_[0]), m_nverts*3*sizeof(float) );
  memcpy( m_binormals, &(outputAtts[5].floatVector_[0]), m_nverts*3*sizeof(float) );

#if 0
  for (int i = 0; i < m_nverts; i++ )
  {
      m_normals[i].normalize();
      m_tangents[i].normalize();

      m_binormals[i] = m_tangents[i].cross(m_normals[i]);
      m_binormals[i].normalize();

//      m_tangents[i] = m_binormals[i].cross(m_normals[i]);
//      m_tangents[i].normalize();
  }
#endif

  m_dlist = 0;
  printf("Loaded '%s', %d vertices, %d triangles\n", filename, m_nverts, m_nindices/3);

  return true;
}

void
NvIOModel::Draw()
{
    glVertexPointer( 3, GL_FLOAT, 0, &m_positions[0]);
    glEnableClientState( GL_VERTEX_ARRAY );

    glNormalPointer( GL_FLOAT, 0, &m_normals[0] );
    glEnableClientState( GL_NORMAL_ARRAY );

    glTexCoordPointer( 3, GL_FLOAT, 0, &m_texcoords[0] );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glDrawElements( GL_TRIANGLES, m_nindices, GL_UNSIGNED_INT, &m_indices[0] );
    
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}


void
NvIOModel::DrawBasis(float length)
{
  glBegin(GL_LINES);
  for(unsigned int i=0; i<m_nverts; i++) {
    vec3f v = m_positions[i];
    vec3f n = v + m_normals[i]*length;
    vec3f t = v + m_tangents[i]*length;
    vec3f b = v + m_binormals[i]*length;
    glColor3f(1.0, 0.0, 0.0);
    glVertex3fv( (GLfloat *) &v);
    glVertex3fv( (GLfloat *) &n);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3fv( (GLfloat *) &v);
    glVertex3fv( (GLfloat *) &t);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3fv( (GLfloat *) &v);
    glVertex3fv( (GLfloat *) &b);
  }
  glEnd();
}

// render, encoding triangle ids as RGB color
void
NvIOModel::DrawIDs()
{
  glBegin(GL_TRIANGLES);
  for(unsigned int i=0; i<m_nindices; i+=3) {
    unsigned int i0 = m_indices[i];
    unsigned int i1 = m_indices[i+1];
    unsigned int i2 = m_indices[i+2];
    unsigned int tri = (i / 3) + 1;  // background is zero
    glColor3ub( tri & 255, (tri >> 8) & 255, (tri >> 16) & 255 );
    glVertex3fv( (GLfloat *) &m_positions[i0]);
    glVertex3fv( (GLfloat *) &m_positions[i1]);
    glVertex3fv( (GLfloat *) &m_positions[i2]);
  }
  glEnd();
}

void
NvIOModel::DrawTriangle(unsigned int tri)
{
  if (tri > m_nindices/3) return;

  glBegin(GL_TRIANGLES);
  unsigned int i0 = m_indices[tri*3];
  unsigned int i1 = m_indices[tri*3+1];
  unsigned int i2 = m_indices[tri*3+2];
  glVertex3fv( (GLfloat *) &m_positions[i0]);
  glVertex3fv( (GLfloat *) &m_positions[i1]);
  glVertex3fv( (GLfloat *) &m_positions[i2]);
  glEnd();
}

vec3f
NvIOModel::GetTriangleNormal(unsigned int tri)
{
  if (tri > m_nindices/3) return vec3f(0.0f);

  unsigned int i0 = m_indices[tri*3];
  unsigned int i1 = m_indices[tri*3+1];
  unsigned int i2 = m_indices[tri*3+2];

  vec3f n = m_normals[i0] + m_normals[i1] + m_normals[i2];
  n.normalize();
  return n;
//  return m_normals[i0];
}

void
NvIOModel::BuildDList()
{
    m_dlist = glGenLists(1);
    glNewList(m_dlist, GL_COMPILE);
    Draw();
    glEndList();
}

void
NvIOModel::Render()
{
  if (!m_dlist) {
    BuildDList();
  }
  glCallList(m_dlist);
}

void
NvIOModel::Rescale()
{
    unsigned int i;
    vec3f center(0.0, 0.0, 0.0);
    float radius = 1.0;

    // Determine model extents
    vec3f vmin(9999999.0f), vmax(-99999999.0f);
    for ( i = 0; i < m_nverts; i++ ) {
        vec3f &v = m_positions[i];
        if ( v[0] < vmin[0] ) vmin[0] = v[0];
        if ( v[0] > vmax[0] ) vmax[0] = v[0];
        if ( v[1] < vmin[1] ) vmin[1] = v[1];
        if ( v[1] > vmax[1] ) vmax[1] = v[1];
        if ( v[2] < vmin[2] ) vmin[2] = v[2];
        if ( v[2] > vmax[2] ) vmax[2] = v[2];
    }

    // Scale to shift to [-radius,radius]
    vec3f rad = 0.5f*(vmax-vmin);

    vec3f oldcenter = vmin + rad;

    float oldradius = rad.length();
    float scale = radius / oldradius;
    
    for ( i = 0; i < m_nverts; i++ ) {
        m_positions[i] = center + scale*(m_positions[i] - oldcenter);
    }
}
