#define FILTER_IN_SHADER 1

#if FILTER_IN_SHADER
#define texRECT(A, B) texRECT_bilinear(A, B)
#else
#define texRECT(A, B) h4texRECT(A, B)
#endif

// texture lookup with bilinear filtering
vec4 texRECT_bilinear(uniform samplerRECT tex, vec2 s)
{
	vec2 f = frac(s);
	vec4 s2 = s.xyxy + vec4(0, 0, 1, 1);
	vec4 t0 = h4texRECT(tex, s2.xy);
	vec4 t1 = h4texRECT(tex, s2.zy);
	vec4 t2  = lerp(t0, t1, f[0]);
	t0 = h4texRECT(tex, s2.xw);
	t1 = h4texRECT(tex, s2.zw);
	t0 = lerp(t0, t1, f[0]);
	t0 = lerp(t2, t0, f[1]);
	return t0;
}

// texture lookup with 3x3 blur
vec4 texRECT_blur(samplerRECT tex, vec2 s)
{
  vec4 a;
  a  =   h4texRECT(tex, s + vec2(-1, -1)); a += 2*h4texRECT(tex, s + vec2(0, -1)); a +=   h4texRECT(tex, s + vec2(1, -1));
  a += 2*h4texRECT(tex, s + vec2(-1, 0));  a += 4*h4texRECT(tex, s);                 a += 2*h4texRECT(tex, s + vec2(1, 0));
  a +=   h4texRECT(tex, s + vec2(-1, 1));  a += 2*h4texRECT(tex, s + vec2(0, 1));  a +=   h4texRECT(tex, s + vec2(1, 1));
  a *= 1.0 / 16.0;
  return a;
}

// texture lookup with 3x3 sharpen
vec4 texRECT_sharpen(samplerRECT tex, vec2 s)
{
  vec4 a;
  a  =   -h4texRECT(tex, s + vec2(-1, -1)); a += -2*h4texRECT(tex, s + vec2(0, -1)); a +=   -h4texRECT(tex, s + vec2(1, -1));
  a += -2*h4texRECT(tex, s + vec2(-1, 0));  a += 24*h4texRECT(tex, s);                 a += -2*h4texRECT(tex, s + vec2(1, 0));
  a +=   -h4texRECT(tex, s + vec2(-1, 1));  a += -2*h4texRECT(tex, s + vec2(0, 1));  a +=   -h4texRECT(tex, s + vec2(1, 1));
  a *= 1.0 / 12.0;
  return a;
}
