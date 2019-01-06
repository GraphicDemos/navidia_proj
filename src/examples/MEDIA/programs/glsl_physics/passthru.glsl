uniform vec4 scale;
uniform vec4 bias;
uniform sampler2DRect tex;

void main()
{
    vec2 uv = gl_TexCoord[0].st;
    gl_FragColor = texture2DRect(tex, uv) * scale + bias;
}
