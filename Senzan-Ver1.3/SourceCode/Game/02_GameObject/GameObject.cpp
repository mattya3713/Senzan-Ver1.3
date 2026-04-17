#include "GameObject.h"
#include "Engine/Utility/Transform/Transform.h"
#include "Engine/Utility/Math/Math.h"
#include "Game/00_GameLoop/Time/Time.h"

GameObject::GameObject()
	: m_spTransform		( std::make_shared<Transform>() )
	, m_Tag				( "Untagged" )
	, m_IsActive		( true )
	, m_IsRenderActive	( true )
	, m_TimeScale		( -1.0f )
{
}

GameObject::~GameObject()
{
}

// Transformを設定.
void GameObject::SetTransform(const Transform& TransformValue)
{
 // 既存のTransformがあれば上書き、なければ新規作成する.
	if (m_spTransform) {
        *m_spTransform = TransformValue;
	}
	else {
        m_spTransform = std::make_shared<Transform>(TransformValue);
	}
}

// ====================================================================================================.
// 位置設定 (SetPosition).
// ====================================================================================================.

void GameObject::SetPosition(const DirectX::XMFLOAT3& Position)
{
    m_spTransform->Position = Position;
}

void GameObject::SetPosition(float X, float Y, float Z)
{
    m_spTransform->Position = DirectX::XMFLOAT3(X, Y, Z);
}

void GameObject::SetPositionX(float X)
{
    m_spTransform->Position.x = X;
}

void GameObject::SetPositionY(float Y)
{
    m_spTransform->Position.y = Y;
}

void GameObject::SetPositionZ(float Z)
{
    m_spTransform->Position.z = Z;
}


// ====================================================================================================.
// 位置加算 (AddPosition).
// ====================================================================================================.

void GameObject::AddPosition(const DirectX::XMFLOAT3& Position)
{
    // SIMDで加算.
    DirectX::XMVECTOR v_pos = DirectX::XMLoadFloat3(&m_spTransform->Position);
    DirectX::XMVECTOR v_add = DirectX::XMLoadFloat3(&Position);
    DirectX::XMVECTOR v_new = DirectX::XMVectorAdd(v_pos, v_add);
    DirectX::XMStoreFloat3(&m_spTransform->Position, v_new);
}

void GameObject::AddPosition(float X, float Y, float Z)
{
    m_spTransform->Position.x += X;
    m_spTransform->Position.y += Y;
    m_spTransform->Position.z += Z;
}

void GameObject::AddPositionX(float X)
{
    m_spTransform->Position.x += X;
}

void GameObject::AddPositionY(float Y)
{
    m_spTransform->Position.y += Y;
}

void GameObject::AddPositionZ(float Z)
{
    m_spTransform->Position.z += Z;
}


// ====================================================================================================.
// 回転設定 (SetRotation).
// ====================================================================================================.

void GameObject::SetRotation(const DirectX::XMFLOAT3& Rotation)
{
    m_spTransform->Rotation = Rotation;
    m_spTransform->UpdateQuaternionFromRotation();
}

void GameObject::SetRotation(float X, float Y, float Z)
{
    m_spTransform->Rotation = DirectX::XMFLOAT3(X, Y, Z);
    m_spTransform->UpdateQuaternionFromRotation(); 
}

void GameObject::SetRotationX(float X)
{
    m_spTransform->Rotation.x = X;
    m_spTransform->UpdateQuaternionFromRotation(); 
}

void GameObject::SetRotationY(float Y)
{
    m_spTransform->Rotation.y = Y;
    m_spTransform->UpdateQuaternionFromRotation(); 
}

void GameObject::SetRotationZ(float Z)
{
    m_spTransform->Rotation.z = Z;
    m_spTransform->UpdateQuaternionFromRotation();
}

void GameObject::SetRotationAroundAxis(const DirectX::XMFLOAT3& Axis, float Angle)
{
    DirectX::XMVECTOR v_axis = DirectX::XMLoadFloat3(&Axis);
    DirectX::XMVECTOR q_axis = DirectX::XMQuaternionRotationAxis(v_axis, Angle);
    DirectX::XMVECTOR q_current = DirectX::XMLoadFloat4(&m_spTransform->Quaternion);

    // 現在クォータニオンに軸回転を乗算.
    DirectX::XMVECTOR q_new = DirectX::XMQuaternionMultiply(q_current, q_axis);
    DirectX::XMStoreFloat4(&m_spTransform->Quaternion, q_new);

    m_spTransform->UpdateRotationFromQuaternion();
}


// ====================================================================================================.
// 拡縮設定 (SetScale).
// ====================================================================================================.

void GameObject::SetScale(const DirectX::XMFLOAT3& Scale)
{
    m_spTransform->Scale = Scale;
}

void GameObject::SetScale(float XYZ)
{
    m_spTransform->Scale = DirectX::XMFLOAT3(XYZ, XYZ, XYZ);
}

void GameObject::SetScale(float X, float Y, float Z)
{
    m_spTransform->Scale = DirectX::XMFLOAT3(X, Y, Z);
}

void GameObject::SetScaleX(float X)
{
    m_spTransform->Scale.x = X;
}

void GameObject::SetScaleY(float Y)
{
    m_spTransform->Scale.y = Y;
}

void GameObject::SetScaleZ(float Z)
{
    m_spTransform->Scale.z = Z;
}


const std::string& GameObject::GetTag() const
{
	return m_Tag;
}


void GameObject::SetTag(const std::string& Tag)
{
    m_Tag = Tag;
}


bool GameObject::IsActive() const
{
	return m_IsActive;
}


void GameObject::SetIsActive(bool IsActive)
{
  m_IsActive = IsActive;
}


bool GameObject::IsRenderActive() const
{
	return m_IsRenderActive;
}


void GameObject::SetIsRenderActive(bool IsActive)
{
    m_IsRenderActive = IsActive;
}

void GameObject::SetTimeScale(float NewTimeScale)
{
	m_TimeScale = NewTimeScale;
}

float GameObject::GetTimeScale()
{
    if (MyMath::IsNearlyEqual(m_TimeScale, -1.f))
    {
        return Time::GetInstance().GetWorldTimeScale();
    }
    else
    {
        return m_TimeScale;
    }
}

// 最終的なDeltaTimeを取得.
float GameObject::GetDelta()
{
    float delta_time = Time::GetInstance().GetDeltaTime();

    // m_TimeScale が -1f の場合はワールド時間倍率を使用.
    if (MyMath::IsNearlyEqual(m_TimeScale, -1.f))
    {
        return delta_time;
    }
    else
    {
        return delta_time * m_TimeScale;
    }
}

// 目標角度へ回転補間.
void GameObject::RotetToTarget(float TargetRote, float RotetionSpeed)
{
    // クォータニオンSlerpでY軸方向にスムーズ回転する.
    float delta_time = GetDelta();

    // 目標角度を正規化する.
    TargetRote = MyMath::NormalizeAngleDegrees(TargetRote);
    DirectX::XMFLOAT3 current_rotation = m_spTransform->GetRotationDegrees();
    float current_rote = MyMath::NormalizeAngleDegrees(current_rotation.y);

    // 角度差分を計算(度数法、正規化済み).
    float angle_diff_deg = MyMath::NormalizeAngleDegrees(TargetRote - current_rote);
    float max_rotate_deg = RotetionSpeed * delta_time;

    // 1フレームで回せる角度を超えないよう補間係数を決定.
    float t = 0.0f;
    if (std::fabsf(angle_diff_deg) <= max_rotate_deg)
    {
        t = 1.0f;
    }
    else
    {
        t = max_rotate_deg / std::fabsf(angle_diff_deg);
        t = std::clamp(t, 0.0f, 1.0f);
    }

    // 現在のクォータニオン.
    DirectX::XMVECTOR q_current = DirectX::XMLoadFloat4(&m_spTransform->Quaternion);

    // 目標Y回転をラジアンへ変換.
    float targetYawRad = DirectX::XMConvertToRadians(TargetRote);
    DirectX::XMVECTOR q_target = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, targetYawRad, 0.0f);

    // Slerpで補間して適用.
    DirectX::XMVECTOR q_result = DirectX::XMQuaternionSlerp(q_current, q_target, t);
    q_result = DirectX::XMQuaternionNormalize(q_result);
    DirectX::XMFLOAT4 qf;
    DirectX::XMStoreFloat4(&qf, q_result);
    m_spTransform->SetQuaternion(qf);
}

