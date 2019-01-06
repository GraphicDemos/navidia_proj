// cloth shader vertex program
void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    gl_TexCoord[0] = gl_MultiTexCoord0 - 0.5;
    gl_TexCoord[1] = (gl_MultiTexCoord0 + 0.5) / 32.0 * vec4(1.0, -1.0, 0.0, 0.0);

    vec4 eyePos_obj = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 V = gl_Vertex.xyz - eyePos_obj.xyz;

    gl_TexCoord[2] = vec4(V, 0.0);
}