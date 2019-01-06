attribute vec4 position;
attribute vec3 normal;
attribute vec4 weight;
attribute vec4 index;
attribute float numBones;

uniform mat4 boneMatrices[30];
uniform vec4 color;
uniform vec4 lightPos;

void main()
{
    vec4 transformedPosition = vec4(0.0);
    vec3 transformedNormal = vec3(0.0);

    vec4 curIndex = index;
    vec4 curWeight = weight;

    for (int i = 0; i < int(numBones); i++)
    {
        mat4 m44 = boneMatrices[int(curIndex.x)];
        
        // transform the offset by bone i
        transformedPosition += m44 * position * curWeight.x;

        mat3 m33 = mat3(m44[0].xyz,
                        m44[1].xyz,
                        m44[2].xyz);

        // transform normal by bone i
        transformedNormal += m33 * normal * curWeight.x;

        // shift over the index/weight variables, this moves the index and 
        // weight for the current bone into the .x component of the index 
        // and weight variables
        curIndex = curIndex.yzwx;
        curWeight = curWeight.yzwx;
    }

    gl_Position = gl_ModelViewProjectionMatrix * transformedPosition;

    transformedNormal = normalize(transformedNormal);
    gl_FrontColor = dot(transformedNormal, lightPos.xyz) * color;
}
