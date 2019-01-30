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

uniform sampler2D decalMap;
uniform samplerCube envMap;
uniform sampler3D noiseMap;

uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 ambientColor;
uniform vec3 lightVector;

varying vec3 oPosition;
varying vec3 ePosition;
varying vec3 normal;
varying vec2 uv;

float fresnel(vec3 i, vec3 n, float eta);

float saturate(float a)
{
    return clamp(a, 0.0, 1.0);
}

void main()
{
    // PART 1 : Diffuse Lighting of base coat

    // Constants for computations

    // light vector in eye space...
    vec3 diffuse_material_color = diffuseColor * 2.0;
    vec3 decal = texture2D(decalMap, uv).rgb;

    // 1a) Normalize light and normal vectors...
    vec3 L = normalize(lightVector);
    vec3 N = normalize(normal);

    // 1b) Compute diffuse component with dot product between 
    //     light and normal vectors
    float n_dot_l = dot(N, L);

    // 1c) Compute self shadowing term
    float self_shadow = (n_dot_l > 0.0) ? 1.0 : 0.0;

    // 1d) Compute  all the above terms together to arrive at final diffuse color
    vec3 diffuse_color = self_shadow * n_dot_l * (diffuse_material_color + ambientColor) * decal;

    // PART 2: Add specular gloss of clear coat

    // Constants for computations in part 2
    vec3 specular_material_color = specularColor;
    float clearcoat_shininess = 80.0;

    //      2a) Calculate view vector from eye to fragment position in eye coordinates
    //          Hint: the eye position in eye space is constant
    vec3 V = normalize(ePosition);

    //      2b) Calculate reflection vector of eye around normal
    //          Hint: use the standard library "reflect" function
    vec3 R = reflect(V, N);

    //      2c) Calculate specular component with dot product between reflection and light vectors
    //          Don't forget to factor in your self shadowing term
    //          Don't raise it to the shininess until the next step (you'll be using r_dot_l later)
    float r_dot_l = self_shadow * saturate(dot(R, L));

    //      2d) Okay, go ahead and raise it to the power and modulate with specular material color
    vec3 specular_color = pow(r_dot_l, clearcoat_shininess) * specular_material_color;

    // PART 3: Add reflected environment with fresnel term
    // Constants for computations in part 3
    float refraction_index_ratio = 1.0/1.5;

    //      3a) You've already calculated the reflection vector, so simply look up into the env_map
    //          Hint: use vec3 texCUBE(texobj tex, vec3 texcoords);
    vec3 reflected_color = textureCube(envMap, R).rgb;

    //      3b) Calculate the fresnel term (hint: use the stdlib again)
    float f = fresnel(V, N, refraction_index_ratio);

    // PART 4: Add metallic flecks between the base coat and clear coat

    // Constants used for computing fleck intensity and color
    float freq = 2.1;

    // saturate the specular color
    vec3 fleck_material_color = specularColor * 4.0;
    float fleck_locality = 15.0;
    float fleck_shininess = 5.0;

    // Scale the obj_coords to make things more sparkly
    vec3 noise_coords = oPosition * 4.0;

    //      4a) Look up 4 noise values from the noise_map, sampling at different frequencies (powers of freq)
    //          Hint: use vec3 f3tex3D(texobj tex, vec3 texcoords);
    vec3 fleck_normal0 = texture3D(noiseMap, noise_coords).rgb;
    vec3 fleck_normal1 = texture3D(noiseMap, noise_coords * freq).rgb;
    vec3 fleck_normal2 = texture3D(noiseMap, noise_coords * pow(freq, 2.0)).rgb;
    vec3 fleck_normal3 = texture3D(noiseMap, noise_coords * pow(freq, 3.0)).rgb;

    //      4b) Hack a specular component calculation - instead of (R dot L)^s, use (N dot E)^s
    //          This doesn't take the light position into account, but the normals are random so nobody
    //          will notice!  You will want to reverse the normal if it's pointing away from the eye.
    float  fleck_intensity = pow(saturate(abs(fleck_normal0.z)), fleck_shininess) + 
                             pow(saturate(abs(fleck_normal1.z)), fleck_shininess) + 
                             pow(saturate(abs(fleck_normal2.z)), fleck_shininess) + 
                             pow(saturate(abs(fleck_normal3.z)), fleck_shininess);

    //      4c) Calculate specular component using this normal, just like you did above, with ONE EXCEPTION
    //          Raise r_dot_l from above by the fleck_locality constant and modulate into the computation
    //          This will restrict the sparkles to the vicinity of the specular highlights.
    float locality_term = pow(r_dot_l, fleck_locality);
    fleck_intensity = fleck_intensity * locality_term * self_shadow;

    //      4e) Finally, modulate with fleck's material color
    vec3 fleck_color = fleck_intensity * fleck_material_color;

    // PART 5 (BONUS ROUND): Add noise octaves together to form smooth value noise, use to slightly perturb
    //                       reflection vector to simulate slight irregularities on car surface
    //                       Hint: use the fleck_normal lookups from above

    // Constant for computation in part 5
    float clearcoat_smoothness = 25.0;

    //      5a) Synthesize a multi-frequency noise function by appropriately weighting each of the noise
    //           freqencies with diminishing weights (1/2, 1/4, 1/8, 1/16...)
    vec3 noise_vector = 0.5 * fleck_normal0 + 0.25 * fleck_normal1 + 
        0.125 * fleck_normal2 + 0.0625 * fleck_normal3;

    //      5b) Use this vector to perturb N very slightly
    N = normalize(noise_vector + clearcoat_smoothness * N);

    //      5c) Replace specular calculation in Part 2 with this one
    R = reflect(V, N);
    r_dot_l = saturate(dot(R, L));
    specular_color = self_shadow * pow(r_dot_l, clearcoat_shininess) * specular_material_color;

    // You won't have to change anything from here to the end...
    // Final compositing
    vec3 sum_color = mix(diffuse_color + fleck_color, reflected_color, f) + specular_color;

    gl_FragColor = vec4(sum_color, 1.0);
}

//
// Fresnel function
// references:
//
// fresnel reflectance: www.graphics.cornell.edu/~phil/GI/TotalCompendium.pdf (p.26)
// refraction vector: http://research.microsoft.com/~hollasch/cgindex/render/refraction.txt
//
// Assumes:
//      eta = n1/n2
//      i is normalized and points towards the surface like in RenderMan
//
// Optionally computes:
//      kt = refraction coefficient = (1-kr)/(eta*eta)  SIGGRAPH 2001 Advanced RenderMan notes
//      R  = reflection ray
//      T  = refraction ray
//
// If "total internal reflectance" occurs:
//      kt = 0;
//      T = (0,0,0);
//      returns 1;
//
float fresnel(vec3 i, vec3 n, float eta)
{
    float c1;
    float cs2;
    float tflag;
    vec3 t;
    
    // Refraction vector courtesy Paul Heckbert.
    c1 = dot(-i,n);
    cs2 = 1.0 - eta * eta * (1.0 - c1 * c1);
    tflag = (cs2 >= 0.0) ? 1.0 : 0.0;
    t = tflag * (((eta * c1 - sqrt(cs2)) * n) + eta * i);
    // t is already unit length or (0,0,0)

    // From Global Illumination Compendium.
    float ndott;
    float cosr_div_cosi;
    float cosi_div_cosr;
    float fs;
    float fp;
    float kr;

    ndott = dot(-n,t);
    cosr_div_cosi = ndott / c1;
    cosi_div_cosr = c1 / ndott;
    fs = (cosr_div_cosi - eta) / (cosr_div_cosi + eta);
    fs = fs * fs;
    fp = (cosi_div_cosr - eta) / (cosi_div_cosr + eta);
    fp = fp * fp;
    kr = 0.5 * (fs + fp);
    
    return tflag*kr + (1.0 - tflag);
}
