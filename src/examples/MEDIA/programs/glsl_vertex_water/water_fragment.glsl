varying vec3 reflectVec;
varying vec3 colorCoeff1;
varying vec3 colorCoeff2;

uniform samplerCube environmentMap;
uniform samplerCube environmentMapDark;

void main()
{
    vec3 reflectColor = textureCube(environmentMap, reflectVec).rgb;
    vec3 reflectColorDark = textureCube(environmentMapDark, reflectVec).rgb;

    gl_FragColor.rgb = (reflectColor * colorCoeff1) + (reflectColorDark * colorCoeff2);
    gl_FragColor.a = 1.0;    
}
