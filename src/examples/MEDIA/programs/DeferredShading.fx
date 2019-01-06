float4x4 WorldViewProj : WorldViewProj;
float4x3 WorldView;
float4x3 WorldViewIT;
float4x4 InvProj;
float4x4 InvViewProj;
float4x4 InvView;
float4x4 ShadowMat;
float3 LightVector;
float3 LightPosition;
float3 EyeVector;
float SpecularExponent;
float3 LightColor;
float LightRange;
float TonemapScale;
float EmissiveFactor;
float NotShadowed;
float2 BlurOffset[8];

float Ambient;

texture DiffuseMap;
texture NormalMap;
texture MRT0;
texture MRT1; 
texture MRT2;
texture MRT3;
texture LightTransport;
texture Luminance;
texture AvgLuminance;
texture ShadowMap;
texture BlurTexture;
texture CurrentLum;
texture PreviousLum;

sampler CurrentLumSampler = sampler_state
{
    texture = <CurrentLum>;
    AddressU  = CLAMP;      
    AddressV  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler PrevLumSampler = sampler_state
{
    texture = <PreviousLum>;
    AddressU  = CLAMP;      
    AddressV  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler LuminanceSampler = sampler_state
{
    texture = <Luminance>;
    AddressU  = CLAMP;      
    AddressV  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler BlurSampler = sampler_state
{
    texture = <BlurTexture>;
    AddressU  = CLAMP;      
    AddressV  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler ShadowMapSampler = sampler_state
{
    texture = <ShadowMap>;
    AddressU  = CLAMP;      
    AddressV  = CLAMP;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler AvgLuminanceSampler = sampler_state
{
    texture = <AvgLuminance>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler DiffuseMapSampler = sampler_state 
{
    texture = <DiffuseMap>;
    AddressU  = WRAP;        
    AddressV  = WRAP;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
    
    SRGBTEXTURE = FALSE;
};

sampler NormalMapSampler = sampler_state 
{
    Texture   = <NormalMap>;
    AddressU  = WRAP;
    AddressV  = WRAP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    SRGBTEXTURE = FALSE;
};

sampler MRT0Sampler = sampler_state 
{
    texture = <MRT0>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler MRT1Sampler = sampler_state 
{
    Texture   = <MRT1>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;
    SRGBTEXTURE = FALSE;
};

sampler MRT2Sampler = sampler_state 
{
    texture = <MRT2>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    MINFILTER = POINT;
    MAGFILTER = POINT;
    MIPFILTER = NONE;
    SRGBTEXTURE = FALSE;
};

sampler MRT3Sampler = sampler_state 
{
    Texture   = <MRT3>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;
    SRGBTEXTURE = FALSE;
};

sampler LightTransportSampler = sampler_state 
{
    Texture   = <LightTransport>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;
    SRGBTEXTURE = FALSE;
};

sampler LightTransportSampler_Filtered = sampler_state 
{
    Texture   = <LightTransport>;
    AddressU  = CLAMP;
    AddressV  = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;
    SRGBTEXTURE = FALSE;
};

struct VS_INPUT0 {
    float4 Position : POSITION; //in object space
    float3 Normal   : NORMAL;   //in object space
    float2 TexCoord : TEXCOORD0;
    float3 T        : TEXCOORD1; //in object space
    float3 B        : TEXCOORD2; //in object space
    float3 N        : TEXCOORD3; //in object space
};

struct VS_INPUT1 {
    float4 Position : POSITION; //in object space
    float2 TexCoord : TEXCOORD0;
};

struct VS_INPUT2 {
    float4 Position : POSITION; //in object space
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VS_INPUT_LIGHTMESH {
    float4 Position : POSITION; //in object space
    float3 Normal   : NORMAL;
};

struct VS_OUTPUT0 {
    float4 Position       : POSITION; //in projection space
    float2 TexCoord0      : TEXCOORD0;
    float2 TexCoord1      : TEXCOORD1;
    float3 TangentToView0 : TEXCOORD2;   //in tangent space
    float3 TangentToView1 : TEXCOORD3;   //in tangent space
    float3 TangentToView2 : TEXCOORD4;   //in tangent space
    float4 Position2      : TEXCOORD5;
};

struct VS_OUTPUT1 {
    float4 Position   : POSITION;
    float2 TexCoord   : TEXCOORD0;
    float4 Position2  : TEXCOORD1;
};

struct VS_OUTPUT2 {
    float4 Position  : POSITION;
    float3 Normal    : TEXCOORD0;
    float2 TexCoord  : TEXCOORD1;
    float4 Position2 : TEXCOORD2;
};

struct VS_OUTPUT_LIGHTMESH {
    float4 Position  : POSITION;
    float3 Normal    : TEXCOORD0;
    float4 Position2 : TEXCOORD2;
};

struct PS_MRT_OUTPUT {
	float4 Color0 : COLOR0;
	float4 Color1 : COLOR1;
	float4 Color2 : COLOR2;
};


VS_OUTPUT0 SimpleVS(VS_INPUT0 IN)
{
    VS_OUTPUT0 Out;

    // pass texture coordinates for fetching the diffuse map
    Out.TexCoord0 = IN.TexCoord.xy;

    // pass texture coordinates for fetching the normal map
    Out.TexCoord1 = IN.TexCoord.xy;

    // compute the 3x3 tranform from tangent space to object space; we will 
    //   use it "backwards" (vector = mul(matrix, vector) to go from object 
    //   space to tangent space, though.
    float3x3 TangentToObject;
    TangentToObject[0] = IN.T;
    TangentToObject[1] = IN.B;
    TangentToObject[2] = IN.N;
    
    float3x3 TangentToView = mul(TangentToObject, WorldView);
    
    Out.TangentToView0 = TangentToView[0];
    Out.TangentToView1 = TangentToView[1];
    Out.TangentToView2 = TangentToView[2];

    // transform position to projection space
    Out.Position = mul(IN.Position, WorldViewProj);
    
    Out.Position2 = mul(IN.Position, WorldViewProj);

	return Out;
}

VS_OUTPUT2 SimpleVSInterpolatedNormal(VS_INPUT2 In)
{
    VS_OUTPUT2 Out;

    //normal in view space
    float3 NormalView = mul(In.Normal, WorldViewIT);
    
    float3 ViewPos = mul(In.Position, WorldView);
    
    Out.Normal = normalize(NormalView);
    Out.Position = mul(In.Position, WorldViewProj);
    Out.Position2 = mul(In.Position, WorldViewProj);
    Out.TexCoord = In.TexCoord;

	return Out;
}

VS_OUTPUT_LIGHTMESH SimpleVSLightMesh(VS_INPUT_LIGHTMESH In)
{
    VS_OUTPUT_LIGHTMESH Out;

    //normal in view space
    float3 NormalView = mul(In.Normal, WorldViewIT);
    
    Out.Normal = normalize(NormalView);
    Out.Position = mul(In.Position, WorldViewProj);
    Out.Position2 = mul(In.Position, WorldViewProj);

	return Out;
}

PS_MRT_OUTPUT SimplePS(VS_OUTPUT0 In)
{
	PS_MRT_OUTPUT Out;
	
	half4 diffuseTex = tex2D( DiffuseMapSampler, In.TexCoord0);
	half3 normalTex = tex2D( NormalMapSampler, In.TexCoord1);
	
	diffuseTex.xyz = diffuseTex.w;
	
	//put normal into view space
	float3x3 TangentToView;
	TangentToView[0] = In.TangentToView0;
	TangentToView[1] = In.TangentToView1;
	TangentToView[2] = In.TangentToView2;
	half3 normal = normalize(mul(normalTex, TangentToView));
	
	//pack
	normal = normal * 0.5f + 0.5f;
	
	Out.Color0 = float4(diffuseTex.xyz, NotShadowed);
	Out.Color1 = float4(normal, 0.0);  
	Out.Color2 = float4(In.Position2.z / In.Position2.w, 0.0f, 0.0f, 0.0f);
	
	return Out;
}

PS_MRT_OUTPUT EmissivePS(VS_OUTPUT0 In)
{
	PS_MRT_OUTPUT Out;
	
	half3 diffuseTex = tex2D( DiffuseMapSampler, In.TexCoord0);
	
	Out.Color0 = float4(diffuseTex.xyz, 1.0);
	Out.Color1 = float4(0.5, 0.5, 0.5, EmissiveFactor / 10.0f);
	Out.Color2 = float4(0.0, 0.0, 0.0, 0.0);
	
	return Out;
}

PS_MRT_OUTPUT SimplePSInterpolatedNormal(VS_OUTPUT2 In)
{
	PS_MRT_OUTPUT Out;
	
	half4 diffuseTex = tex2D( DiffuseMapSampler, In.TexCoord);
	half3 normal = normalize(In.Normal);

	diffuseTex.w = diffuseTex.w - 0.01;
	clip(diffuseTex.www);
	
	//pack
	normal = normal * 0.5f + 0.5f;
	
	Out.Color0 = float4(diffuseTex.xyz, NotShadowed);
	Out.Color1 = float4(normal, 0.0);
	Out.Color2 = float4(In.Position2.z / In.Position2.w, 0.0f, 0.0f, 0.0f);

	return Out;
}

PS_MRT_OUTPUT SimplePSLightMesh(VS_OUTPUT_LIGHTMESH In)
{
	PS_MRT_OUTPUT Out;
	
	half3 normal = normalize(In.Normal);
	
	//pack
	normal = normal * 0.5f + 0.5f;
	
	Out.Color0 = float4(1.0, 1.0, 1.0, NotShadowed);
	Out.Color1 = float4(normal, 0.0);
	Out.Color2 = float4(In.Position2.z / In.Position2.w, 0.0f, 0.0f, 0.0f);
	
	return Out;
}


VS_OUTPUT1 MRT_VS(VS_INPUT1 IN)
{
    VS_OUTPUT1 Out;

    // transform position to projection space
    Out.Position = mul(IN.Position, WorldViewProj);

    Out.Position2 = mul(IN.Position, WorldViewProj);
    
    // pass texture coordinates for fetching the diffuse map
    Out.TexCoord = IN.TexCoord.xy;

	return Out;
}

float4 DirLightingPS(VS_OUTPUT1 In) : COLOR
{
	half4 diffuseTex  = tex2D(MRT0Sampler, In.TexCoord);
	half4 normalTex   = tex2D(MRT1Sampler, In.TexCoord);
	half  z           = tex2D(MRT2Sampler, In.TexCoord);

	half3 finalLighting = half3(0.0, 0.0, 0.0);

	//unpack
	half isNotShadowed = diffuseTex.w;
	
	//////////////////////////
	//reconstruct original view-space position
	float x = In.Position2.x;
	float y = In.Position2.y;
	float4 position = mul(float4(x, y, z, 1.0), InvProj);
	position.xyz = position.xyz / position.www;
	position.w = 1.0f;
	
	//compute position in light space
	float4 posLight = mul(position, ShadowMat);
	half shadow = tex2Dproj(ShadowMapSampler, posLight).x;
	shadow += isNotShadowed;  //if not shadowed, push shadow to 1 (no shadow)
	shadow = saturate(shadow);
	//////////////////////////
	
	half3 normal = normalTex.xyz * 2 - 1;
	half3 albedo = diffuseTex.xyz;
	half3 emissive = albedo * normalTex.w * 10.0f;
	
	normal = normalize(normal);
	
	//diffuse lighting
	half NdotL = dot(normal, LightVector);//float3(0.0, 0.0, -1.0));
	half selfShadow = (NdotL > 0) ? 1 : 0;
	half3 diffuse = albedo * NdotL * selfShadow * LightColor;

	finalLighting = shadow * diffuse + Ambient + emissive;

	return float4(finalLighting, 1.0f);
}

float4 PointLightingPS(VS_OUTPUT1 In) : COLOR
{
	half4 diffuseTex = tex2D(MRT0Sampler, In.TexCoord);
	half4 normalTex  = tex2D(MRT1Sampler, In.TexCoord);
	half  z          = tex2D(MRT2Sampler, In.TexCoord);

	//unpack
	half kS = normalTex.w;
	half3 normal = normalTex.xyz * 2 - 1;
    half3 albedo = diffuseTex.xyz;

	//reconstruct original view-space position
	float x = In.Position2.x;
	float y = In.Position2.y;
	float4 position = mul(float4(x, y, z, 1.0/*w*/), InvViewProj);
	position.xyz = position.xyz / position.www;
	position.w = 1.0f;
	float3 lightPos = mul(float4(LightPosition, 1.0), InvView);
	
	float3 lightVec = (lightPos - position.xyz);
	float length_ = length(lightVec);
	
	//normalize
	lightVec = lightVec / length_;

	half atten = max(1.0f - (length_ / LightRange), 0.0f);
	
	normal = normalize(mul(float4(normal, 0.0), InvView));
	
	//diffuse lighting
	half NdotL = dot(normal, lightVec);
	half selfShadow = (NdotL > 0) ? 1 : 0;
	half3 diffuse = albedo * NdotL * selfShadow * LightColor;

	//
	//specular currently disabled, need better art	
	float3 eyeVec = normalize(-position.xyz);
	
	//compute half angle
	half3 halfAngle = normalize(lightVec + eyeVec);
	
	half3 specular = max(pow(dot(halfAngle, normal), SpecularExponent), 0) * selfShadow;
	//
	
	half3 finalLighting = albedo * NdotL * selfShadow * LightColor * atten;
	
	return float4(finalLighting, 1.0f);
}

float4 ConvertToLumPS(VS_OUTPUT1 In) : COLOR
{
	half3 lightTransport = tex2D(LightTransportSampler, In.TexCoord);
	
	//convert to luminance
	float lum = dot(lightTransport, float3(0.3086, 0.6094, 0.0820));
	float logLum = log(0.001 + lum);  //used to prevent numerical underflow of log(0)
	
	return float4(lum, logLum, 0.0, 0.0);
}

float4 AddBlurPS(VS_OUTPUT1 In) : COLOR
{
	half3 lightTransport = tex2D(LightTransportSampler, In.TexCoord);
	half3 blur			 = tex2D(BlurSampler, In.TexCoord);
	
	return float4(lerp(blur, lightTransport, 0.5), 1.0f);
}

const float weights[8] = {
	0.05,
	0.08,
	0.1,
	0.15,
	0.15,
	0.1,
	0.08,
	0.05,
};

float4 BlurPS(VS_OUTPUT1 In) : COLOR
{
	half3 sum = tex2D(BlurSampler, float2(In.TexCoord.x, In.TexCoord.y)) * 0.25;

	for (int i = 0; i < 8; ++i)
	{
		sum += tex2D(BlurSampler, In.TexCoord + BlurOffset[i]) * weights[i];
	}
	
	return float4(sum, 1.0f);
}

float4 DebugScissorPS(VS_OUTPUT1 In) : COLOR
{
	half4 diffuseTex = tex2D(MRT0Sampler, In.TexCoord);
	half4 normalTex  = tex2D(MRT1Sampler, In.TexCoord);
	half  z          = tex2D(MRT2Sampler, In.TexCoord);
	half  w          = tex2D(MRT3Sampler, In.TexCoord);

	return float4(diffuseTex.xyz, 1.0f);
}

float4 OutputChannel0(VS_OUTPUT1 In) : COLOR
{
	half4 diffuseTex = tex2D(MRT0Sampler, In.TexCoord);
	half4 normalTex  = tex2D(MRT1Sampler, In.TexCoord);
	half  z          = tex2D(MRT2Sampler, In.TexCoord);

	return float4(diffuseTex.xyz, 1.0f);
}

float4 OutputChannel1(VS_OUTPUT1 In) : COLOR
{
	half4 diffuseTex = tex2D(MRT0Sampler, In.TexCoord);
	half4 normalTex  = tex2D(MRT1Sampler, In.TexCoord);
	half  z          = tex2D(MRT2Sampler, In.TexCoord);
	
	return float4(normalTex.xyz, 1.0f);
}

float4 OutputChannel2(VS_OUTPUT1 In) : COLOR
{
	half4 diffuseTex = tex2D(MRT0Sampler, In.TexCoord);
	half4 normalTex  = tex2D(MRT1Sampler, In.TexCoord);
	half  z          = tex2D(MRT2Sampler, In.TexCoord);

	return float4(z.xxx, 1.0f);
}

float4 PassThroughPS(VS_OUTPUT1 In) : COLOR
{
	half4 inTex = tex2D(LightTransportSampler, In.TexCoord);

	return float4(inTex.x, inTex.y, inTex.z, inTex.w);
}

float4 FilterPS(VS_OUTPUT1 In) : COLOR
{
	half4 inTex = tex2D(LightTransportSampler_Filtered, In.TexCoord);
	
	return float4(inTex.x, inTex.y, inTex.z, inTex.w);
}

//fade in current frame average log luminance value
float4 AdaptationPS(VS_OUTPUT1 In) : COLOR
{
	half4 currFrame = tex2D(CurrentLumSampler, In.TexCoord);
	half4 prevFrame = tex2D(PrevLumSampler, In.TexCoord);
	
	half4 finalCol = lerp(currFrame, prevFrame, 0.99);
	
	return finalCol;
}

float4 FilteredThresholdPS(VS_OUTPUT1 In) : COLOR
{
	half4 inTex = tex2D(LightTransportSampler_Filtered, In.TexCoord);
		
	if ( (inTex.x < 3.0) &&
		 (inTex.y < 3.0) &&
		 (inTex.z < 3.0) )
		inTex = 0.0f;
	
	return float4(inTex.x, inTex.y, inTex.z, inTex.w);
}

float4 RenderToShadowMapPS(VS_OUTPUT1 In) : COLOR
{
	return float4(0.0, 0.0, 0.0, 0.0);
}

float4 TonemapPS(VS_OUTPUT1 In) : COLOR
{
	half lum = tex2D(LuminanceSampler, In.TexCoord).x;
	half3 blur = tex2D(BlurSampler, In.TexCoord);
	half avg_loglum = exp(tex2D(AvgLuminanceSampler, half2(0.5, 0.5)).y);  //grab the first texel
	half3 lightTransport = tex2D(LightTransportSampler, In.TexCoord).xyz;

	lightTransport += blur;
	
	half lum_temp = (TonemapScale / (avg_loglum + 0.001)) * lum;
	half ld = lum_temp / (lum_temp + 1);
	half3 finalColor = (lightTransport / lum) * ld;

	return float4(finalColor, 1.0f);
}

technique CreateMRTPerPixelNormal
{
	pass P0
	{
		VertexShader = compile vs_1_1 SimpleVS();
		PixelShader = compile ps_2_0 SimplePS();
		
		AlphaBlendEnable	= false;
		AlphaTestEnable		= false;
		FogEnable			= false;
		
		CullMode			= CW;
	}
}

technique CreateMRTEmissive
{
	pass P0
	{
		VertexShader = compile vs_1_1 SimpleVS();
		PixelShader = compile ps_2_0 EmissivePS();
		
		AlphaBlendEnable	= false;
		AlphaTestEnable		= false;
		FogEnable			= false;
		
		CullMode			= CW;
	}
}

technique CreateMRTInterpolatedNormal
{
	pass P0
	{
		VertexShader = compile vs_1_1 SimpleVSInterpolatedNormal();
		PixelShader = compile ps_2_0 SimplePSInterpolatedNormal();
		
		AlphaBlendEnable	= false;
		AlphaTestEnable		= false;
		FogEnable			= false;
		
		CullMode			= CW;
	}
}

technique CreateMRTLightMesh
{
	pass P0
	{
		VertexShader = compile vs_1_1 SimpleVSLightMesh();
		PixelShader = compile ps_2_0 SimplePSLightMesh();
		
		AlphaBlendEnable	= false;
		AlphaTestEnable		= false;
		FogEnable			= false;
		
		CullMode			= CW;
	}
}

technique DirLighting
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 DirLightingPS();
		
		AlphaTestEnable = false;

		CullMode		= None;
		
		FillMode = Solid;
	}
}

technique PointLighting
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 PointLightingPS();
		
		AlphaTestEnable = false;

		CullMode		= None;
		
		FillMode = Solid;
	}
}

technique DebugScissor
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 DebugScissorPS();
		
		AlphaTestEnable = false;

		CullMode		= None;
		
		FillMode = Solid;
	}
}

technique ShowChannel0
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 OutputChannel0();
		
		AlphaTestEnable = false;

		CullMode		= None;
		
		FillMode = Solid;
	}
}

technique ShowChannel1
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 OutputChannel1();
		
		AlphaTestEnable = false;

		CullMode		= None;
		
		FillMode = Solid;
	}
}

technique ShowChannel2
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 OutputChannel2();
		
		AlphaTestEnable = false;

		CullMode		= None;
		
		FillMode = Solid;
	}
}

technique Adaptation
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 AdaptationPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique ConvertToLuminance
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 ConvertToLumPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique Blur
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 BlurPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique AddBlur
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 AddBlurPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique PassThrough
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 PassThroughPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		//pixel blending state
		Sampler[0]   = (LightTransportSampler);
		ColorArg1[0] = Texture;
		ColorOp[0]   = Selectarg1;
		ColorArg2[0] = Current;
		
		FillMode = Solid;
	}
}

technique Tonemap
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 TonemapPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique PassThroughFiltered
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 FilterPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique FilteredThreshold
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 FilteredThresholdPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}

technique RenderToShadowMap
{
	pass P0
	{
		VertexShader = compile vs_1_1 MRT_VS();
		PixelShader = compile ps_2_0 RenderToShadowMapPS();
		
		AlphaTestEnable  = false;
		AlphaBlendEnable = false;

		CullMode		 = None;
		
		FillMode = Solid;
	}
}
