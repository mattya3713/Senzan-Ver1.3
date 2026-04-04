//=============================================================================
// Skinning.hlsl
// �X�^�e�B�b�N�E�X�L�����b�V�����p�V�F�[�_�[.
//=============================================================================

//-----------------------------------------------------------------------------
// �萔�o�b�t�@.
//-----------------------------------------------------------------------------

// ���[���h�E�r���[�E�v���W�F�N�V�����s��.
cbuffer cbWorld : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
};

// �X�P���g���i�{�[���s��j.
cbuffer cbSkeleton : register(b1)
{
    matrix BoneTransforms[128];
};

// �}�e���A��.
cbuffer cbMaterial : register(b0)
{
    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
    float Shininess;
    float3 Padding;
};

//-----------------------------------------------------------------------------
// �e�N�X�`���E�T���v���[.
//-----------------------------------------------------------------------------
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

//-----------------------------------------------------------------------------
// ���_�V�F�[�_�[���͍\����.
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
// ���_�V�F�[�_�[�o�͍\����.
//-----------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// ���_�V�F�[�_�[.
//-----------------------------------------------------------------------------
VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    // �X�L�j���O�ϊ��s����v�Z.
    // �X�^�e�B�b�N���b�V���̏ꍇ�� BoneTransforms[0] = Identity,
    // BoneIDs = {0,0,0,0}, Weights = {1,0,0,0} �ƂȂ邽�߁A
    // �P�ʍs��ł̕ϊ��ƂȂ�.
    matrix skinMatrix =
        BoneTransforms[input.BoneIDs.x] * input.Weights.x +
        BoneTransforms[input.BoneIDs.y] * input.Weights.y +
        BoneTransforms[input.BoneIDs.z] * input.Weights.z +
        BoneTransforms[input.BoneIDs.w] * input.Weights.w;

    // �X�L�j���O�K�p��̈ʒu.
    float4 skinnedPos = mul(float4(input.Pos, 1.0f), skinMatrix);

    // ���[���h�ϊ�.
    float4 worldPos = mul(skinnedPos, World);
    output.WorldPos = worldPos.xyz;

    // �r���[�E�v���W�F�N�V�����ϊ�.
    float4 viewPos = mul(worldPos, View);
    output.Pos = mul(viewPos, Projection);

    // �X�L�j���O�K�p��̖@��.
    float3 skinnedNormal = mul(input.Normal, (float3x3)skinMatrix);

    // ���[���h��Ԃł̖@��.
    output.Normal = normalize(mul(skinnedNormal, (float3x3)World));

    // UV���W.
    output.UV = input.UV;

    return output;
}

// �s�N�Z���V�F�[�_�[.
float4 PS_Main(VS_OUTPUT input) : SV_TARGET
{
    // �e�N�X�`���T���v�����O.
    float4 texColor = g_Texture.Sample(g_Sampler, input.UV);
    
    // �e�N�X�`�����o�C���h����Ă��Ȃ��ꍇ�͔����g�p.
    // TODO : �e�N�X�`���̗L�����`�F�b�N����萔.
    if (texColor.a < 0.01f && texColor.r < 0.01f && texColor.g < 0.01f && texColor.b < 0.01f)
    {
        texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // �ȈՃ��C�e�B���O�i�f�B���N�V���i�����C�g�j.
    float3 lightDir = normalize(float3(0.5f, -1.0f, 0.5f));
    float3 normal = normalize(input.Normal);
    
    // �f�B�t���[�Y���C�e�B���O.
    float NdotL = max(dot(normal, -lightDir), 0.0f);
    float3 diffuse = Diffuse.rgb * NdotL;
    
    // �A���r�G���g���C�e�B���O.
    float3 ambient = Ambient.rgb * 0.3f;

    // �ŏI�J���[.
    float3 finalColor = (ambient + diffuse) * texColor.rgb;
    
    return float4(finalColor, texColor.a * Diffuse.a);
}
