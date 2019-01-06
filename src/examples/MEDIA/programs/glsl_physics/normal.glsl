// calculate surface normal from positions

uniform sampler2DRect x_tex;

void main()
{
    vec2 uv = gl_TexCoord[0].st;
    
    vec3 x = texture2DRect(x_tex, uv).rgb;
    vec3 x2 = texture2DRect(x_tex, uv + vec2(1.0, 0.0)).rgb;
    vec3 x3 = texture2DRect(x_tex, uv + vec2(0.0, 1.0)).rgb;

    vec3 N = cross(x2 - x, x3 - x);
    N = normalize(N);

    gl_FragColor = vec4(N*0.5 + 0.5, 0.0);
}
