//-----------------------------------------------------------------------------

texture ShadowMap;
texture SpotLight;
texture Jitter;
texture BaseTexture;

//-----------------------------------------------------------------------------

float4x4 World : World;
float4x4 WorldViewProj : WorldViewProj;
float4x4 WorldIT : WorldIT;
float4x4 TexTransform;
float3   LightPos;
float4   FilterSize, JitterScale;
float4	 Resolution;
float    Shade;

//-----------------------------------------------------------------------------

struct LS2V {
    float4 Position : POSITION; //in object space
};

struct V2F {
    float4 Position : POSITION; //in projection space
    float4 Color : COLOR0;
};

V2F SimpleTransform(LS2V IN)
{
    V2F OUT;
    OUT.Color = 1.0f;
    OUT.Position = mul(IN.Position, WorldViewProj);
    return OUT;
}

//-----------------------------------------------------------------------------

struct VS_INPUT {
    float4 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 Position  : POSITION;
    float4 ProjectedCoord : TEXCOORD0;
    float2 BaseTextureCoord : TEXCOORD2;
    float3 WSPos : TEXCOORD3;
    float3 Normal : TEXCOORD4;
};

struct VS2_OUTPUT {
    float4 Position  : POSITION;
    float4 ProjectedCoord : TEXCOORD0;
    float4 ScreenPosition : TEXCOORD1;
    float2 BaseTextureCoord : TEXCOORD2;
    float3 WSPos : TEXCOORD3;
    float3 Normal : TEXCOORD4;
};

//-----------------------------------------------------------------------------

sampler ShadowMapSampler = sampler_state
{
    Texture = <ShadowMap>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler SpotLightSampler = sampler_state
{
    Texture = <SpotLight>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

sampler JitterSampler = sampler_state
{
    Texture = <Jitter>;
    MinFilter = Point;  
    MagFilter = Point;
    MipFilter = None;
    AddressU  = Wrap;
    AddressV  = Wrap;
};

sampler BaseTextureSampler = sampler_state
{
    Texture = <BaseTexture>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Wrap;
    AddressV  = Wrap;
};

//-----------------------------------------------------------------------------

VS_OUTPUT MainVS(VS_INPUT IN)
{
    VS_OUTPUT OUT;

    // transform normal into world space, then dot with world space light vector 
    // to determine how much light is falling on the surface:
    
    OUT.WSPos = mul(IN.Position, World);
    OUT.Normal = normalize(mul(IN.Normal, (float3x3)WorldIT));

    // transform model-space vertex position to light-space:
    
    OUT.ProjectedCoord = mul(IN.Position, TexTransform);
    
    OUT.BaseTextureCoord = IN.TexCoord;
    
    // transform model-space vertex position to screen space:
    
    OUT.Position = mul(IN.Position, WorldViewProj);

    return OUT;
}

VS2_OUTPUT MainSoftVS2(VS_INPUT IN)
{
    VS2_OUTPUT OUT;

    // transform normal into world space, then dot with world space light vector 
    // to determine how much light is falling on the surface:
    
    OUT.WSPos = mul(IN.Position, World);
    OUT.Normal = normalize(mul(IN.Normal, (float3x3)WorldIT));

    // transform model-space vertex position to light-space:
    
    OUT.ProjectedCoord = mul(IN.Position, TexTransform);
    
    OUT.BaseTextureCoord = IN.TexCoord;
    
    // transform model-space vertex position to screen space:
    
    OUT.ScreenPosition = mul(IN.Position, WorldViewProj) * Resolution;
    OUT.Position = mul(IN.Position, WorldViewProj);

    return OUT;
}

//-----------------------------------------------------------------------------

float4 MainPS(VS_OUTPUT IN) : COLOR 
{
    float3 shadow    = tex2Dproj(ShadowMapSampler, IN.ProjectedCoord).rgb;
    float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
    float3 color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
    color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);
    
    return float4(color, 1.0);
}

//-----------------------------------------------------------------------------

float4 MainPenumbraPS(VS_OUTPUT IN, const in float2 vPos : VPOS) : COLOR 
{
	float fsize = IN.ProjectedCoord.w * FilterSize.w;
	float4 smcoord = {0, 0, IN.ProjectedCoord.zw};
	float4 jcoord = {vPos * JitterScale, 0, 0};
	float shadow = 0, shadowsample;
	float4 jitter;
	float3 color;
	
	// Perform 8 test samples
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	if( (shadow > 0) && (shadow < 8) ) {		
		color = float3(1.0f, 0.0f, 0.0f);
	} else {
	  shadow *= 0.125f;
    float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
    color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
    color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);
	}
  
  return float4(color, 1.0);
}

float4 MainSoftPS3(VS_OUTPUT IN, float2 vPos : VPOS, uniform int loopIterations) : COLOR 
{
	float fsize = IN.ProjectedCoord.w * FilterSize.w;
	float4 smcoord = {0, 0, IN.ProjectedCoord.zw};
	float4 jcoord = {vPos * JitterScale, 0, 0};
	float shadow = 0, shadowsample;
	float4 jitter;
	
	// Perform 8 test samples
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
	jcoord.z += 0.03125f;
	smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
	
	if( (shadow > 0) && (shadow < 8) ) {		
		for(int i = 0; i < loopIterations; i++) {
			jitter = (2 * tex3Dlod(JitterSampler, jcoord) - 1.0);
			jcoord.z += 0.03125f;
			smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
			shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
			smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
			shadow += tex2Dlod(ShadowMapSampler, float4(smcoord.xyz / smcoord.w, 0));
		}	

  	shadow /= (2 * (loopIterations + 4));
	} else {
	  shadow /= 8;
	}
	
  float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
  float3 color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
  color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);
  
  return float4(color, 1.0);
}

float4 MainSoftPS2_16(VS2_OUTPUT IN) : COLOR 
{
	float fsize = IN.ProjectedCoord.w * FilterSize.w;
	float4 smcoord = {0, 0, IN.ProjectedCoord.zw};
	float4 jcoord = {IN.ScreenPosition.xy * JitterScale.xy, 0, 0};
	float shadow = 0, shadowsample;
	float4 jitter;
	
	// Perform 16 samples
	for(int i = 0; i <= 7; i++) {
	  jitter = (2 * tex3D(JitterSampler, jcoord) - 1.0);
	  jcoord.z += 0.03125f;
	  smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	  smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	}
	
	shadow /= 16;
	
  float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
  float3 color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
  color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);

  return float4(color, 1.0);
}

float4 MainSoftPS2_32(VS2_OUTPUT IN) : COLOR 
{
	float fsize = IN.ProjectedCoord.w * FilterSize.w;
	float4 smcoord = {0, 0, IN.ProjectedCoord.zw};
	float4 jcoord = {IN.ScreenPosition.xy * JitterScale.xy, 0, 0};
	float shadow = 0, shadowsample;
	float4 jitter;
	
	// Perform 32 samples
	for(int i = 0; i <= 15; i++) {
	  jitter = (2 * tex3D(JitterSampler, jcoord) - 1.0);
	  jcoord.z += 0.03125f;
	  smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	  smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	}
	
	shadow /= 32;
	
  float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
  float3 color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
  color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);

  return float4(color, 1.0);
}

float4 MainSoftPS2_48(VS2_OUTPUT IN) : COLOR 
{
	float fsize = IN.ProjectedCoord.w * FilterSize.w;
	float4 smcoord = {0, 0, IN.ProjectedCoord.zw};
	float4 jcoord = {IN.ScreenPosition.xy * JitterScale.xy, 0, 0};
	float shadow = 0, shadowsample;
	float4 jitter;
	
	// Perform 48 samples
	for(int i = 0; i <= 23; i++) {
	  jitter = (2 * tex3D(JitterSampler, jcoord) - 1.0);
	  jcoord.z += 0.03125f;
	  smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	  smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	}
	
	shadow /= 48;
	
  float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
  float3 color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
  color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);

  return float4(color, 1.0);
}

float4 MainSoftPS2_64(VS2_OUTPUT IN) : COLOR 
{
	float fsize = IN.ProjectedCoord.w * FilterSize.w;
	float4 smcoord = {0, 0, IN.ProjectedCoord.zw};
	float4 jcoord = {IN.ScreenPosition.xy * JitterScale.xy, 0, 0};
	float shadow = 0, shadowsample;
	float4 jitter;
	
	// Perform 64 samples
	for(int i = 0; i <= 31; i++) {
	  jitter = (2 * tex3D(JitterSampler, jcoord) - 1.0);
	  jcoord.z += 0.03125f;
	  smcoord.xy = jitter.xy * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	  smcoord.xy = jitter.zw * fsize + IN.ProjectedCoord.xy;
	  shadow += tex2Dproj(ShadowMapSampler, smcoord);
	}
	
	shadow /= 64;
	
  float3 spotlight = tex2Dproj(SpotLightSampler, IN.ProjectedCoord).rgb;
  float3 color = max(dot(normalize(LightPos - IN.WSPos), IN.Normal), 0.0);
  color = spotlight * shadow * color * tex2D(BaseTextureSampler, IN.BaseTextureCoord);

  return float4(color, 1.0);
}

//-----------------------------------------------------------------------------

float4 WhitePS(VS_OUTPUT IN) : COLOR 
{
    return float4(0, 1.0f, 1.0f, 1.0f);
}

//-----------------------------------------------------------------------------

technique UseHardwareShadowMap
{
    pass P0
    {
        VertexShader = compile vs_2_0 MainVS();
        PixelShader = compile ps_2_0 MainPS();
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap2_16
{
    pass P0
    {
        VertexShader = compile vs_2_a MainSoftVS2();
        PixelShader = compile ps_2_a MainSoftPS2_16();
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap3_16
{
    pass P0
    {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader = compile ps_3_0 MainSoftPS3(4);
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap2_32
{
    pass P0
    {
        VertexShader = compile vs_2_a MainSoftVS2();
        PixelShader = compile ps_2_a MainSoftPS2_32();
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap3_32
{
    pass P0
    {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader = compile ps_3_0 MainSoftPS3(12);
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap2_48
{
    pass P0
    {
        VertexShader = compile vs_2_a MainSoftVS2();
        PixelShader = compile ps_2_a MainSoftPS2_48();
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap3_48
{
    pass P0
    {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader = compile ps_3_0 MainSoftPS3(20);
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap2_64
{
    pass P0
    {
        VertexShader = compile vs_2_a MainSoftVS2();
        PixelShader = compile ps_2_a MainSoftPS2_64();
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique UseHardwareSoftShadowMap3_64
{
    pass P0
    {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader = compile ps_3_0 MainSoftPS3(28);
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique ShowPenumbra
{
    pass P0
    {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader = compile ps_3_0 MainPenumbraPS();
        
        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = CW;
    }
}

technique GenHardwareShadowMap
{
    pass P0
    {
        VertexShader = compile vs_1_1 MainVS();
        PixelShader = compile ps_1_1 WhitePS();

        ZEnable          = True;
        AlphaBlendEnable = False;
        Lighting         = False;
        CullMode         = None;  // note: not quite optimal
        
        ColorWriteEnable = 0;     // no need to render to color, we only need z
    }
}

Technique DrawLightSource
{
    Pass P0
    {
        VertexShader = compile vs_1_1 SimpleTransform();
        PixelShader = NULL;
    }
}

//-----------------------------------------------------------------------------

