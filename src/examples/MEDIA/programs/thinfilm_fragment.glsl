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

uniform sampler2D diffuseMap;
uniform sampler2D fringeMap;

varying vec3 diffColor;
varying vec3 specColor;
varying vec2 viewDepth;
varying vec2 uv;

void main()
{
    // diffuse material color
    vec3 diffMaterial = texture2D(diffuseMap, uv).rgb;

    // lookup fringe value based on view depth
    vec3 fringeColor = texture2D(fringeMap, viewDepth).rgb;

    // modulate specular ligh ting by fringe color, combine with regular lighting
    gl_FragColor = vec4(diffColor*diffMaterial + fringeColor*specColor, 1.0);
}
