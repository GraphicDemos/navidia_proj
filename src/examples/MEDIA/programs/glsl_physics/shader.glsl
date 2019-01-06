// cloth shader
// object-space Blinn lighting model

uniform sampler2DRect normal_tex;
uniform sampler2D cloth_tex;

void main()
{
    // these should really be shader parameters:
    const float shininess = 20.0;
    const vec3 ambientColor = vec3(0.3, 0.3, 0.3);
    const vec3 diffuseColor = vec3(0.7, 0.7, 0.7);
    const vec3 specularColor = vec3(1.0, 1.0, 1.0);  
    const vec3 L = vec3(-0.577, -0.577, -0.577);     // light direction

    // get interpolated normal
    vec3 N = texture2DRect(normal_tex, gl_TexCoord[0].xy).rgb * 2.0 - 1.0;
    N = normalize(N);

    vec3 V = normalize(gl_TexCoord[2].xyz);

#if 1
    // Phong
    vec3 R = reflect(-V, N);
    vec4 lighting = lit(dot(N, L), dot(L, R), shininess);
#else
    // Blinn
    vec3 H = normalize(V + L);
    vec4 lighting = lit(dot(N, L), dot(N, H), shininess);
#endif

    vec3 colorMap = tex2D(cloth_tex, gl_TexCoord[1].xy).rgb;

    vec3 color = ambientColor*colorMap + lighting.y*diffuseColor*colorMap + lighting.z*specularColor;
    
    gl_FragColor = vec4(color, 0.0);
}
