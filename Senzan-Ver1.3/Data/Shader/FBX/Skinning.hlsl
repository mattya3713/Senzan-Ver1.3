//=============================================================================
// Skinning.hlsl
// スキニング対応メッシュ用シェーダー。
//=============================================================================

//-----------------------------------------------------------------------------
// 定数バッファ。
//-----------------------------------------------------------------------------

// ワールド・ビュー・プロジェクション行列。
cbuffer cbWorld : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

// スケルトン（ボーン行列）。
cbuffer cbSkeleton : register(b1)
{
    matrix BoneTransforms[128];
};

// マテリアル。
cbuffer cbMaterial : register(b2)
{
    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
    float Shininess;
    float3 Padding;
};

//-----------------------------------------------------------------------------
// テクスチャ・サンプラー。
//-----------------------------------------------------------------------------
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

//-----------------------------------------------------------------------------
// 頂点シェーダー入力構造体。
//-----------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos      : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
    uint4  BoneIDs  : BONEIDS;
    float4 Weights  : WEIGHTS;
};

//-----------------------------------------------------------------------------
// 頂点シェーダー出力構造体。
//-----------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// 頂点シェーダー。
//-----------------------------------------------------------------------------
VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    // スキニング変換行列を計算。
    // スタティックメッシュの場合は BoneTransforms[0] = Identity,
    // BoneIDs = {0,0,0,0}, Weights = {1,0,0,0} となるため、
    // 単位行列での変換となる。
    matrix skinMatrix =
        BoneTransforms[input.BoneIDs.x] * input.Weights.x +
        BoneTransforms[input.BoneIDs.y] * input.Weights.y +
        BoneTransforms[input.BoneIDs.z] * input.Weights.z +
        BoneTransforms[input.BoneIDs.w] * input.Weights.w;

    // スキニング適用後の位置。
    float4 skinnedPos = mul(float4(input.Pos, 1.0f), skinMatrix);

    // ワールド変換。
    float4 worldPos = mul(skinnedPos, World);
    output.WorldPos = worldPos.xyz;

    // ビュー・プロジェクション変換。
    float4 viewPos = mul(worldPos, View);
    output.Pos = mul(viewPos, Projection);

    // スキニング適用後の法線。
    float3 skinnedNormal = mul(input.Normal, (float3x3)skinMatrix);

    // ワールド空間での法線。
    output.Normal = normalize(mul(skinnedNormal, (float3x3)World));

    // UV座標。
    output.UV = input.UV;

    return output;
}

// ピクセルシェーダー。
float4 PS_Main(VS_OUTPUT input) : SV_TARGET
{
    // テクスチャサンプリング。
    float4 texColor = g_Texture.Sample(g_Sampler, input.UV);

    // テクスチャがバインドされていない場合は白色を使用。
    // TODO : テクスチャ有無を示す定数で分岐。
    if (texColor.a < 0.01f && texColor.r < 0.01f && texColor.g < 0.01f && texColor.b < 0.01f)
    {
        texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // 簡易ライティング（ディレクショナルライト）。
    float3 lightDir = normalize(float3(0.5f, -1.0f, 0.5f));
    float3 normal = normalize(input.Normal);

    // ディフューズライティング。
    float NdotL = max(dot(normal, -lightDir), 0.0f);
    float3 diffuse = Diffuse.rgb * NdotL;

    // アンビエントライティング。
    float3 ambient = Ambient.rgb * 0.3f;

    // 最終カラー。
    float3 finalColor = (ambient + diffuse) * texColor.rgb;

    return float4(finalColor, texColor.a * Diffuse.a);
}
