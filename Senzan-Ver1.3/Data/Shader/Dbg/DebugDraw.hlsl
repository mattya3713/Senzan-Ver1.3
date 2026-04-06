cbuffer cbDebugViewProj : register(b0)
{
    matrix g_ViewProj;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

PS_INPUT VS_Main(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = mul(float4(input.Position, 1.0f), g_ViewProj);
    output.Color    = input.Color;
    return output;
}

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
    return input.Color;
}
