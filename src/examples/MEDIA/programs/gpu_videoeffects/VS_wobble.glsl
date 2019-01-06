//
// Vertex shader for wobbling a texture
//
// Author: Eric Young
//
// Copyright (c) 2002-2004 3Dlabs Inc. Ltd. 
//
// See 3Dlabs-License.txt for license information
//

void main(void)
{
	vec4 v = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
	gl_Position = gl_ModelViewProjectionMatrix * v;

    gl_TexCoord[0]  = gl_MultiTexCoord0;
    gl_TexCoord[1]  = gl_MultiTexCoord1;
    gl_TexCoord[2]  = gl_MultiTexCoord2;
    gl_TexCoord[3]  = gl_MultiTexCoord3;
}
