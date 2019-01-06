
/* -----------------------------------------------------------------------
    Shaded with several lights
-------------------------------------------------------------------------- */

float4x4 WorldViewProjection;
float4x4 ViewProjection;
float4x4 World;
float4x4 WorldIT;
float3 CameraPosition; // Camera position in world space

// Light positions in world space
float3 LightPosition[] = { 
    float3(-6, 0, 4),
    float3(6, 0, 2),
    float3(0, 0, -7)
};

int LightNum = 3;

// Material properties
float3 Color;
float DiffuseCoeff;
float SpecularCoeff;
float SpecularPower;

bool ColorOnly; // If true: Skip shading and use a single color per object

// Apply lighting
void Shade(float3 position, float3 normal, out float3 diffuse, out float3 specular)
{
    diffuse = 0;
    specular = 0;
    for (int i = 0; i < LightNum; ++i) {
	    float3 L = LightPosition[i] - position;
        float intensity = 1 / (1 + 0.01 * dot(L, L));
        L = normalize(L);
        diffuse += intensity * max(0, dot(normal, L));
	    float3 V = normalize(CameraPosition - position);
	    float3 H = normalize(L + V);
	    specular += intensity * pow(max(0, dot(normal, H)), SpecularPower);
    }
    diffuse *= DiffuseCoeff;
    specular *= SpecularCoeff;
}

struct ShadedPixel
{
    float4 Position : POSITION;
    float3 WorldPosition : TEXCOORD0;
    float3 WorldNormal : TEXCOORD1;
    float3 WorldTangent : TEXCOORD2;
    float2 TexCoord : TEXCOORD3;
};

ShadedPixel ShadedVS(float4 position : POSITION, float3 normal : NORMAL)
{
    ShadedPixel pixel;
    pixel.Position = mul(position, WorldViewProjection);
    pixel.WorldPosition = mul(position, World);
    pixel.WorldNormal = normalize(mul(normal, (float3x3)WorldIT));
    pixel.WorldTangent = 0;
    pixel.TexCoord = 0;
    return pixel;
}

float4 ShadedPS(ShadedPixel pixel) : COLOR
{
    if (ColorOnly)
        return float4(Color, 1);
    else {
        float3 normal = normalize(pixel.WorldNormal);
        float3 diffuse, specular;
        Shade(pixel.WorldPosition, normal, diffuse, specular);
        return float4(0.8 * Color * diffuse + specular, 1);
    }
}

technique Shaded
{
    pass
    {
        VertexShader = compile vs_3_0 ShadedVS();
        PixelShader = compile ps_3_0 ShadedPS();
    }
}

/* -----------------------------------------------------------------------
    Shaded and textured
-------------------------------------------------------------------------- */

texture ColorTexture;
sampler ColorTextureSampler = sampler_state { 
    Texture   = <ColorTexture>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    MaxAnisotropy = 8;
};

float TexCoordScaling;

ShadedPixel TexturedVS(float4 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD0)
{
    ShadedPixel pixel;
    pixel.Position = mul(position, WorldViewProjection);
    pixel.WorldPosition = mul(position, World);
    pixel.WorldNormal = normalize(mul(normal, (float3x3)WorldIT));
    pixel.WorldTangent = 0;
    pixel.TexCoord = texCoord;
    return pixel;
}

float4 TexturedPS(ShadedPixel pixel) : COLOR
{
    if (ColorOnly)
        return float4(Color, 1);
    else {
        float3 normal = normalize(pixel.WorldNormal);
        float3 diffuse, specular;
        Shade(pixel.WorldPosition, normal, diffuse, specular);
        return float4(Color * tex2D(ColorTextureSampler, TexCoordScaling * pixel.TexCoord) * diffuse + specular, 1);
    }
}

technique Textured
{
    pass
    {
        VertexShader = compile vs_3_0 TexturedVS();
        PixelShader = compile ps_3_0 TexturedPS();
    }
}

/* -----------------------------------------------------------------------
    Shaded, textured, and bump mapped
-------------------------------------------------------------------------- */

texture NormalMap;
sampler NormalMapSampler = sampler_state { 
    Texture   = <NormalMap>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    MaxAnisotropy = 8;
};

ShadedPixel TexturedBumpMappedVS(float4 position : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 texCoord : TEXCOORD0)
{
    ShadedPixel pixel;
    pixel.Position = mul(position, WorldViewProjection);
    pixel.WorldPosition = mul(position, World);
    pixel.WorldNormal = normalize(mul(normal, (float3x3)WorldIT));
    pixel.WorldTangent = normalize(mul(tangent, (float3x3)WorldIT));
    pixel.TexCoord = texCoord;
    return pixel;
}

float4 TexturedBumpMappedPS(ShadedPixel pixel) : COLOR
{
    if (ColorOnly)
        return float4(Color, 1);
    else {
        float3 normal = normalize(pixel.WorldNormal);
        float3 tangent = normalize(pixel.WorldTangent);
        float3 binormal = normalize(cross(normal, tangent));
        float3 bumpNormal = normalize(2 * tex2D(NormalMapSampler, TexCoordScaling * pixel.TexCoord) - 1);
        normal = bumpNormal.x * tangent + bumpNormal.y * binormal + bumpNormal.z * normal;
        float3 diffuse, specular;
        Shade(pixel.WorldPosition, normal, diffuse, specular);
        return float4(Color * tex2D(ColorTextureSampler, TexCoordScaling * pixel.TexCoord) * diffuse + specular, 1);
    }
}

technique TexturedBumpMapped
{
    pass
    {
        VertexShader = compile vs_3_0 TexturedBumpMappedVS();
        PixelShader = compile ps_3_0 TexturedBumpMappedPS();
    }
}

/* -----------------------------------------------------------------------
    Shaded textured skinned model
-------------------------------------------------------------------------- */

// Bone world matrices
static const int MAX_MATRICES = 26;
float4x3 BoneWorld[MAX_MATRICES] : WORLDMATRIXARRAY;
int NumBones;

ShadedPixel SkinVS(
	float4 position : POSITION,
    float4 blendWeights : BLENDWEIGHT,
    float4 blendIndices : BLENDINDICES,
    float3 normal : NORMAL,
    float3 texCoord : TEXCOORD0
)
{

    // Compensate for lack of UBYTE4
    int4 indexVector = D3DCOLORtoUBYTE4(blendIndices);

    // cast the vectors to arrays for use in the for loop below
    float blendWeightArray[4] = (float[4])blendWeights;
    int indexArray[4] = (int[4])indexVector;

    // calculate the pos/normal using the "normal" weights 
    // and accumulate the weights to calculate the last weight
    float lastWeight = 0;
    float3 skinPosition = 0;
    float3 skinNormal = 0;
    for (int bone = 0; bone < NumBones - 1; ++bone) {
        lastWeight += blendWeightArray[bone];
        skinPosition += mul(position, BoneWorld[indexArray[bone]]) * blendWeightArray[bone];
        skinNormal += mul(normal, BoneWorld[indexArray[bone]]) * blendWeightArray[bone];
    }
    lastWeight = 1 - lastWeight; 

    // Now that we have the calculated weight, add in the final influence
    skinPosition += (mul(position, BoneWorld[indexArray[NumBones - 1]]) * lastWeight);
    skinNormal += (mul(normal, BoneWorld[indexArray[NumBones - 1]]) * lastWeight);
    skinNormal = normalize(skinNormal);
    
    // transform position from world space into view and then projection space
    ShadedPixel pixel;
    pixel.Position = mul(float4(skinPosition.xyz, 1), ViewProjection);
    pixel.WorldPosition = skinPosition;
    pixel.WorldNormal = skinNormal;
    pixel.WorldTangent = 0;
    pixel.TexCoord = texCoord;

    return pixel;
}

technique Skinned
{
    pass
    {
        VertexShader = compile vs_3_0 SkinVS();
        PixelShader = compile ps_3_0 TexturedPS();
    }
}

/* -----------------------------------------------------------------------
    Shaded textured simulated model
-------------------------------------------------------------------------- */

int RTWidth;
int RTHeight;
float Dx; // Texture coordinate increment to access X-axis neighbor texels
float Dy; // Texture coordinate increment to access y-axis neighbor texels

texture PositionTexture;
sampler PositionSampler = sampler_state { 
    Texture   = <PositionTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

texture NormalTexture;
sampler NormalSampler = sampler_state { 
    Texture   = <NormalTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = POINT;
    MagFilter = POINT;
};

ShadedPixel SimulatedVS(float2 texCoord : TEXCOORD0)
{
    ShadedPixel pixel;
    float4 coord = float4((1 - Dx) * texCoord.x + 0.5 / RTWidth, (1 - Dy) * texCoord.y + 0.5 / RTHeight, 0, 0);
    float3 position = tex2Dlod(PositionSampler, coord);
    float3 normal = tex2Dlod(NormalSampler, coord);
    pixel.Position = mul(float4(position, 1), ViewProjection);
    pixel.WorldPosition = position;
    pixel.WorldNormal = normal;
    pixel.WorldTangent = 0;
    pixel.TexCoord = texCoord;
    return pixel;
}

float4 SimulatedPS(ShadedPixel pixel, float facing : VFACE) : COLOR
{
    pixel.WorldNormal = facing * pixel.WorldNormal;
    return TexturedPS(pixel);
}

technique Simulated
{
    pass
    {
        CullMode = None;
        DepthBias = -0.0001;
        VertexShader = compile vs_3_0 SimulatedVS();
        PixelShader = compile ps_3_0 SimulatedPS();
    }
}

/* -----------------------------------------------------------------------
    Shaded textured, bump mapped, simulated model
-------------------------------------------------------------------------- */

bool IsASeamParticle(float type)
{
	return (2 <= abs(type));
}

float2 GetNextTexCoord(float type)
{
 	float fracPart = frac(abs(type));
	float intPart = abs(type) - fracPart;   
	if (3 < intPart)
		return float2(fracPart, (intPart - 4) * (1 - 0.5 * Dy));
	else
		return float2((intPart - 2) * (1 - 0.5 * Dx), fracPart);
}

float3 ComputeTangent(float4 coord, float3 position, float3 normal)
{
    bool isABorderPixel = (coord.x >= 1 - Dx);
    float3 neighbor = tex2Dlod(PositionSampler, coord + float4(isABorderPixel ? - Dx : Dx, 0, 0, 0));
    float3 projected = neighbor - dot(neighbor - position, normal);
    float3 tangent = normalize(projected - position);
    if (isABorderPixel)
        tangent = - tangent;
    return tangent;
}

ShadedPixel SimulatedBumpMappedVS(float2 texCoord : TEXCOORD0)
{
    ShadedPixel pixel;
    
    // texCoord goes from 0 to 1
    float4 coord = float4((1 - Dx) * texCoord.x + 0.5 / RTWidth, (1 - Dy) * texCoord.y + 0.5 / RTHeight, 0, 0);
    float3 position = tex2Dlod(PositionSampler, coord).xyz;
    float type = tex2Dlod(PositionSampler, coord).w;
    float3 normal = tex2Dlod(NormalSampler, coord);
    pixel.Position = mul(float4(position, 1), ViewProjection);
    pixel.WorldPosition = position;
    pixel.WorldNormal = normal;
    
    // Compute tangent
    pixel.WorldTangent = ComputeTangent(coord, position, normal);
        
    // Seam
	if (IsASeamParticle(type)) {
		int stitchNum = 1, stitchMax = 4;
		float nextType = type;
		float4 nextCoord = float4(GetNextTexCoord(nextType), 0, 0);
		while ((nextCoord.x != coord.x || nextCoord.y != coord.y) && (stitchNum < stitchMax)) {
			float3 nextPosition = tex2Dlod(PositionSampler, nextCoord);
			pixel.WorldTangent += ComputeTangent(nextCoord, nextPosition, normal);
			nextType = tex2Dlod(PositionSampler, nextCoord).w;
		    nextCoord = float4(GetNextTexCoord(nextType), 0, 0);
			++stitchNum;
		}
	}

    pixel.TexCoord = texCoord;
    return pixel;
}

float4 SimulatedBumpMappedPS(ShadedPixel pixel, float facing : VFACE) : COLOR
{
    pixel.WorldNormal = facing * pixel.WorldNormal;
    return TexturedBumpMappedPS(pixel);
}

technique SimulatedBumpMapped
{
    pass
    {
        CullMode = None;
        DepthBias = -0.0001;
        VertexShader = compile vs_3_0 SimulatedBumpMappedVS();
        PixelShader = compile ps_3_0 SimulatedBumpMappedPS();
    }
}
