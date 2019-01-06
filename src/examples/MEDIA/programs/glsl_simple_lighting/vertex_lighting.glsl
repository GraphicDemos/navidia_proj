const vec3 diffuseMaterial = vec3(0.0, 0.0, 1.0);
const vec3 specularMaterial = vec3(1.0, 1.0, 1.0);

uniform vec3 lightVec;

void main()
{
    // transform vertex position into homogenous clip-space
    gl_Position = ftransform();

    // transform normal from model-space to view-space
    vec3 normalVec = normalize(gl_NormalMatrix * gl_Normal);

    // calculate half angle vector
    vec3 eyeVec = vec3(0.0, 0.0, 1.0);
    vec3 halfVec = normalize(lightVec + eyeVec);
    
    // calculate diffuse component
    vec3 diffuse = vec3(max(dot(normalVec, lightVec), 0.0)) * diffuseMaterial;

    // calculate specular component
    vec3 specular = vec3(max(dot(normalVec, halfVec), 0.0));
    specular = pow(specular.x, 32.0) * specularMaterial;
    
    // combine diffuse and specular contributions and output final vertex color
    gl_FrontColor.rgb = diffuse + specular;
    gl_FrontColor.a = 1.0;
}
