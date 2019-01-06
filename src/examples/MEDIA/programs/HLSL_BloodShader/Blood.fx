//HLSL BloodShader FX File
//This Sample illustrates per pixel animation by defining gravity at every point across a texture

//Define the Min height the Blood can have and still move
#define MinBloodHeight 0.4f

//Blood Color initialized to red 
float3 BloodColor = float3(.928, .156, .156);

//Point size 1/512 for 512x512 RT
#define pSize 0.001953125

//Define World Gravity as pointing in the negative Z direction
#define GRAVITY float3(0.0f, 0.0f, -1.0f)

float4x4 World : World;  
float4x4 WorldViewProjection : WorldViewProjection;

//SurfaceMap: Texture that represents the Normal Map for the mesh
texture SurfaceMap;
sampler2D SurfaceMapSampler = sampler_state 
{
    Texture = <SurfaceMap>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

//SurfaceTexture: Texture that represents the Texture Map for the mesh
texture SurfaceTexture;
sampler2D SurfaceTextureSampler = sampler_state 
{
    Texture = <SurfaceTexture>;
    AddressU = CLAMP;
    AddressV = CLAMP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

//GravityMap: A texture representing gravity at each point in Texture Space
texture GravityMap;
sampler2D GravityMapSampler = sampler_state 
{
    Texture = <GravityMap>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
};
//FinalBlend: Also samples the GravityMap but does so with a linear filter(for the final display)
sampler2D FinalBlend = sampler_state 
{
    Texture = <GravityMap>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};
texture RawMap;
sampler2D RawMapSampler = sampler_state 
{
    Texture = <RawMap>;
    AddressU = WRAP;
    AddressV = WRAP;
    MinFilter = POINT;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


//--------------------Shaders-----------------------------------
struct VS_GravityIn 
{
    float3 Pos : POSITION;
    float3 Norm : NORMAL; 
    float2 Tex : TEXCOORD;
    float3 Tan : TANGENT; 
    float3 Bin : BINORMAL;
};

struct VS_GravityOut
{
    float4 Pos : POSITION;
    float2 Tex: TEXCOORD0;
    float2 TexAbove : TEXCOORD1;
    float2 TexBelow : TEXCOORD2;
    float2 TexLeft : TEXCOORD3;
    float2 TexRight : TEXCOORD4;
    float2 Grav : TEXCOORD5;
};

VS_GravityOut VS_GravityMap(VS_GravityIn In)
{
    VS_GravityOut Out = (VS_GravityOut)0;
    Out.Pos = float4(2*(In.Tex.x-.5) - pSize, -2*(In.Tex.y -.5) + pSize, 0, 1);

    //Center Texel
    Out.Tex.x = In.Tex.x;
    Out.Tex.y = In.Tex.y;
    
    //Texel Above
    Out.TexAbove.x = In.Tex.x;
    Out.TexAbove.y = In.Tex.y - pSize;
    
    //Texel Below
    Out.TexBelow.x = In.Tex.x;
    Out.TexBelow.y = In.Tex.y + pSize;
    

    //Texel to the Left
    Out.TexLeft.x = In.Tex.x - pSize;
    Out.TexLeft.y = In.Tex.y;
    
    //Texel to the Right
    Out.TexRight.x = In.Tex.x + pSize;
    Out.TexRight.y = In.Tex.y;

    float3 Gravity = mul(GRAVITY, transpose(World));
    //Gravity is now in object space
    float3x3 ObjectToTan ={In.Tan, In.Bin, In.Norm};
    Gravity = mul(Gravity, ObjectToTan);
    Gravity = normalize(Gravity);
    //Gravity is now in tangent space
    Out.Grav.xy = Gravity.xy;
    
    return Out;
}

float4 PS_GravityMap (VS_GravityOut In) : COLOR
{
    float4 Out = (float4)0;
 
    float Blood = 0;
    float4 Grab = (float4)0;
    float MyGrav; 
    //What's up with my blood
    Grab = tex2D(GravityMapSampler, In.Tex);
    MyGrav = abs(2*(Grab.x -.5)) + abs(2*(Grab.y - .5));
    Blood = Grab.b - MyGrav*Grab.b;
   
    //Above
    Grab = tex2D(GravityMapSampler, In.TexAbove);
    MyGrav = -2*(Grab.y - .5);
    if(MyGrav > 0) Blood += MyGrav*Grab.b;
  
    //Below
    Grab = tex2D(GravityMapSampler, In.TexBelow);
    MyGrav = 2*(Grab.y - .5);
    if(MyGrav> 0)Blood += MyGrav*Grab.b;

    //Left
    Grab = tex2D(GravityMapSampler, In.TexLeft);
    MyGrav = -2*(Grab.x - .5);
    if(MyGrav> 0) Blood += MyGrav*Grab.b;
    
    //Right
    Grab = tex2D(GravityMapSampler, In.TexRight);
    MyGrav = 2*(Grab.x - .5);
    if(MyGrav> 0) Blood += MyGrav*Grab.b;

    float2 BloodDir = (float2)0;

    if(Blood > MinBloodHeight)
    {
    float3 Normal = tex2D(SurfaceMapSampler, In.Tex).rgb;
    Normal = 2*(Normal - .5);
    BloodDir = Normal.xy+In.Grav.xy;
    BloodDir = clamp(BloodDir, -1, 1);
    BloodDir = pow(BloodDir, 3);
    }
    Out.xy = (.5*BloodDir) +.5;
    Out.b = Blood;
    return Out;
}


struct VS_FinalOut
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Light : TEXCOORD2;
    float2 TexAbove : TEXCOORD3;
    float2 TexBelow : TEXCOORD4;
    float2 TexLeft : TEXCOORD5;
    float2 TexRight : TEXCOORD6;
    float3 View : TEXCOORD7;
};

VS_FinalOut VS_FinalBlend(VS_GravityIn In)
{
    VS_FinalOut Out = (VS_FinalOut)0;
    Out.Tex = In.Tex;
    Out.Pos = mul(float4(In.Pos,1), WorldViewProjection);

    //Texel Above
    Out.TexAbove.x = In.Tex.x;
    Out.TexAbove.y = In.Tex.y - pSize;
    
    //Texel Below
    Out.TexBelow.x = In.Tex.x;
    Out.TexBelow.y = In.Tex.y + pSize;
    

    //Texel to the Left
    Out.TexLeft.x = In.Tex.x - pSize;
    Out.TexLeft.y = In.Tex.y;
    
    //Texel to the Right
    Out.TexRight.x = In.Tex.x + pSize;
    Out.TexRight.y = In.Tex.y;
    
    float3x3 ObjectToTan ={In.Tan, In.Bin, In.Norm};
    float3 Light = mul(float3(0.0f, 1.0f, 0.0f), transpose(World));
    //Light vector is now in object space
    Out.Light = mul(Light, ObjectToTan);
    //Light vector is now in tangent space
    
    //Move view vector into tangent space
    float3 View = float3(0,1,0);
    View = mul(View, transpose(World));
    Out.View = mul(View, ObjectToTan);
    
    return Out;
}

float4 PS_FinalBlend(VS_FinalOut In) : COLOR
{
    float4 Color = (float4)0;
    float3 Light = -In.Light;
    float3 View = -In.View;
    Light = normalize(Light);
    
    
    //Get the height of the Blood in this texel
    float Blood = tex2D(FinalBlend, In.Tex).b;
    //Raising Blood to a power of .33 to increase it's intensity
    Blood = pow(Blood,.33);
    //Use this value to determine amount of Blood to be placed at this texel
    float3 BloodOut = Blood*float3(BloodColor);

    //Compute deltas to determine blood normal
    float dx = tex2D(FinalBlend, In.TexRight).b - tex2D(FinalBlend, In.TexLeft).b;
    float dy = tex2D(FinalBlend, In.TexBelow).b - tex2D(FinalBlend, In.TexAbove).b;

    //Get the color for this texel from texture map
    Color = tex2D(SurfaceTextureSampler, In.Tex);
    //Add Blood Color
    Color.rgb = Color.rgb*(1-Blood) + BloodOut;

    //Dot3 Specular Lighting calculation
    float3 Normal = tex2D(SurfaceMapSampler, In.Tex);
    Normal = 2*(Normal-.5);

    //Add the blood's contribution to this normal
    Normal = normalize(Normal + float3(dx, dy, 0));
    //Compute final color
    Color.rgb = Color*.3 + Color*max(0, dot(Light, Normal))*.7 + pow(dot(reflect(Light, Normal), View),30)*.3;
    return Color;
}


technique ComputeGravity
{
    pass Gravity
    {
        VertexShader = compile vs_1_1 VS_GravityMap();
        PixelShader = compile ps_2_0 PS_GravityMap();
    }
}


technique FinalBlend_Tech
{
    pass Blend
    {
        VertexShader = compile vs_1_1 VS_FinalBlend();
        PixelShader = compile ps_2_0 PS_FinalBlend();
    }
}
