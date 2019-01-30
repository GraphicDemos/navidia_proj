/*********************************************************************NVMH3****

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

******************************************************************************/

void main()
{
    // output position
    gl_Position = ftransform();

    // output texture coordinates
    gl_TexCoord[0] = gl_MultiTexCoord0;

    // material parameters
    vec3 ambientMaterial = gl_FrontMaterial.ambient.rgb;
    vec4 diffuseMaterial = gl_FrontMaterial.diffuse;
    vec3 specularMaterial = gl_FrontMaterial.specular.rgb;

    // light parameters
    vec3 lightVec = normalize(gl_LightSource[0].position.xyz);
    vec3 halfVec = normalize(gl_LightSource[0].halfVector.xyz);
    vec3 normalVec = normalize(gl_NormalMatrix * gl_Normal);
    
    // calculate diffuse lighting
    vec3 diffuse = vec3(max(dot(normalVec, lightVec), 0.0)) * diffuseMaterial.rgb;
    
    // calculate specular lighting
    vec3 specular = vec3(max(dot(normalVec, halfVec), 0.0));
    specular = pow(specular.x, 80.0) * specularMaterial;

    gl_FrontColor = vec4(diffuse + specular + ambientMaterial, diffuseMaterial.a);
}