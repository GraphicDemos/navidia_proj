varying vec3 normal;

void main()
{
    gl_Position = ftransform();
    normal = gl_NormalMatrix * gl_Normal;
}