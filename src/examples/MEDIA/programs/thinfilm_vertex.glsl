/*********************************************************************NVMH2****
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
******************************************************************************/

uniform vec3 lightVector;
uniform vec3 eyeVector;
uniform float filmDepth;

varying vec3 diffColor;
varying vec3 specColor;
varying vec2 viewDepth;
varying vec2 uv;

void main()
{
    // transform position to clip space
    gl_Position = ftransform();

    // transform normal from model-space to view-space
    vec3 normalVec = gl_NormalMatrix * gl_Normal;
    normalVec = normalize(normalVec);

    // compute the eye->vertex vector
    vec3 eyeVec = eyeVector;   //infinite viewer (Best looking)

    // compute the view depth for the thin film
    viewDepth = vec2((1.0 / dot(normalVec, eyeVec)) * filmDepth);
    
    // store normalized light vector
    vec3 lightVec = normalize(lightVector);

    // calculate half angle vector
    vec3 halfAngleVec = normalize(lightVec + eyeVec);

    // calculate diffuse component
    float diffuse = max(dot(normalVec, lightVec), 0.0);

    // calculate specular component
    float specular = max(dot(normalVec, halfAngleVec), 0.0);
    specular = pow(specular, 32.0);

    // output final lighting results
    diffColor = vec3(diffuse);
    specColor = vec3(specular);

    // output texture coordinates for diffuse map
    uv = gl_MultiTexCoord0.xy;
}
