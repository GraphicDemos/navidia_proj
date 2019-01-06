// inputs from application
uniform float Time;
uniform vec4 WaveData[5];

// outputs from vertex shader
varying vec3 reflectVec;
varying vec3 colorCoeff1;
varying vec3 colorCoeff2;

void calcWave(out float disp,
              out vec2 normal,
              float dampening,
              vec3 viewPosition,
              float waveTime,
              float height,
              float frequency,
              vec2 waveDirection)
{
    float distance1 = dot(viewPosition.xy, waveDirection);

    distance1 = frequency * distance1 + waveTime;

    disp = height * sin(distance1) / dampening;
    normal = -cos(distance1) * height * frequency * waveDirection / (0.4 * dampening);
}

void main()
{
    vec4 position = vec4(gl_Vertex.x, 0.0, gl_Vertex.y, 1.0);
    vec3 normal = vec3(0.0, 1.0, 0.0);
    float dampening = (dot(position.xyz, position.xyz)/1000.0 + 1.0);

    float disp;
    vec2 norm;
    
    for (int i = 0; i < 5; i++)
    {
        float waveTime  = Time * WaveData[i].z;
        float frequency = WaveData[i].z;
        float height    = WaveData[i].w;
        vec2 waveDir  = WaveData[i].xy;

        calcWave(disp, norm, dampening, gl_Vertex.xyz, waveTime, height, frequency, waveDir);
        position.y = position.y + disp;
        normal.xz = normal.xz + norm;
    }

    gl_Position = gl_ModelViewProjectionMatrix * position;

    // transfom normal into eye-space
    normal = normalize(gl_NormalMatrix * normal);

    // get a vector from the vertex to the eye
    vec3 eyeToVert = (gl_ModelViewMatrix * position).xyz;
    eyeToVert = normalize(eyeToVert);

    // calculate the reflected vector for cubemap look-up
    reflectVec = mat3(gl_TextureMatrix[0]) * reflect(eyeToVert, normal);

    // Calculate a fresnel term (note that f0 = 0)
    float fres = 1.0 + dot(eyeToVert, normal);
    fres = pow(fres, 5.0);

    // set the two color coefficients (the magic constants are arbitrary), 
    // these two color coefficients are used to calculate the contribution from 
    // each of the two environment cubemaps (one bright, one dark)
    colorCoeff1 = vec3(fres * 1.4 + min(reflectVec.y, 0.0)) + vec3(0.2, 0.3, 0.3);
    colorCoeff2 = vec3(fres * 1.26);
}
