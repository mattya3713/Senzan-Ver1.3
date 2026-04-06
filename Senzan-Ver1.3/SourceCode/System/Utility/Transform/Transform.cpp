#include "Transform.h"
#include "System/Utility/Math/Math.h"	

#include <DirectXMath.h>
#include <cmath>
//---------------------------------------------------------------------------------
// ヘルパーメソッドの実装.
//---------------------------------------------------------------------------------

void Transform::SetPosition(const DirectX::XMFLOAT3& newPosition)
{
	Position = newPosition;
}

void Transform::SetRotation(const DirectX::XMFLOAT3& eulerAngles)
{
	Rotation = eulerAngles;
	UpdateQuaternionFromRotation();
}

void Transform::SetRotationX(float X)
{
	Rotation.x = X;
	UpdateQuaternionFromRotation();
}

void Transform::SetRotationY(float Y)
{
    // ピッチとロールを保ちつつ、ヨーを目標値にスナップする.
    // 現在のピッチ(Rotation.x)、目標ヨー、現在のロール(Rotation.z)からクォータニオンを構築.
    float desiredYaw = Y;

    const float twoPI = 2.0f * DirectX::XM_PI;
    // 目標ヨーを[-PI, PI]の範囲に正規化する.
    desiredYaw = std::fmod(desiredYaw, twoPI);
    if (desiredYaw <= -DirectX::XM_PI) desiredYaw += twoPI;
    else if (desiredYaw > DirectX::XM_PI) desiredYaw -= twoPI;

    DirectX::XMVECTOR currentQuat = DirectX::XMLoadFloat4(&Quaternion);
    DirectX::XMVECTOR targetQuat = DirectX::XMQuaternionRotationRollPitchYaw(Rotation.x, desiredYaw, Rotation.z);

    // クォータニオンの半球を合わせて180度の長経路反転を防ぐ.
    float dot = DirectX::XMVectorGetX(DirectX::XMVector4Dot(currentQuat, targetQuat));
    if (dot < 0.0f)
    {
        targetQuat = DirectX::XMVectorNegate(targetQuat);
    }

    DirectX::XMStoreFloat4(&Quaternion, DirectX::XMQuaternionNormalize(targetQuat));
    // クォータニオンからオイラー角を同期する.
    UpdateRotationFromQuaternion();
}

void Transform::SetRotationZ(float Z)
{
	Rotation.z = Z;
	UpdateQuaternionFromRotation();
}

// 角度（度）で回転を設定.
void Transform::SetRotationDegrees(const DirectX::XMFLOAT3& eulerAnglesInDegrees)
{
	SetRotation(DirectX::XMFLOAT3(
		DirectX::XMConvertToRadians(eulerAnglesInDegrees.x),
		DirectX::XMConvertToRadians(eulerAnglesInDegrees.y),
		DirectX::XMConvertToRadians(eulerAnglesInDegrees.z)
	));
}

void Transform::SetQuaternion(const DirectX::XMFLOAT4& newQuaternion)
{
	Quaternion = newQuaternion;
	UpdateRotationFromQuaternion();
}

void Transform::SetScale(const DirectX::XMFLOAT3& newScale)
{
	Scale = newScale;
}

void Transform::Translate(const DirectX::XMFLOAT3& translation)
{
	DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&Position);
	DirectX::XMVECTOR trans = DirectX::XMLoadFloat3(&translation);
	XMStoreFloat3(&Position, DirectX::XMVectorAdd(pos, trans));
}

void Transform::Rotate(const DirectX::XMFLOAT3& eulerAngles)
{
	DirectX::XMVECTOR currentRot = DirectX::XMLoadFloat3(&Rotation);
	DirectX::XMVECTOR newRot = DirectX::XMLoadFloat3(&eulerAngles);
	XMStoreFloat3(&Rotation, DirectX::XMVectorAdd(currentRot, newRot));
	UpdateQuaternionFromRotation();
}

void Transform::RotateDegrees(const DirectX::XMFLOAT3& eulerAnglesInDegrees)
{
	Rotate(DirectX::XMFLOAT3(
        DirectX::XMConvertToRadians(eulerAnglesInDegrees.x),
        DirectX::XMConvertToRadians(eulerAnglesInDegrees.y),
        DirectX::XMConvertToRadians(eulerAnglesInDegrees.z)
	));
}

void Transform::Rotate(const DirectX::XMFLOAT4& quaternion)
{
	DirectX::XMVECTOR currentQuat = DirectX::XMLoadFloat4(&Quaternion);
	DirectX::XMVECTOR newQuat = DirectX::XMLoadFloat4(&quaternion);
	DirectX::XMStoreFloat4(&Quaternion, DirectX::XMQuaternionMultiply(currentQuat, newQuat));
	UpdateRotationFromQuaternion();
}

void Transform::RotateToDirection(const DirectX::XMFLOAT3& NormVecDirection)
{
	// 目標の方向ベクトルの取得.
	DirectX::XMVECTOR v_target_direction = DirectX::XMLoadFloat3(&NormVecDirection);
	v_target_direction = DirectX::XMVectorSetY(v_target_direction, 0.0f);

	// 正規化.
	if (DirectX::XMVector3NearEqual(
		v_target_direction,
		DirectX::XMVectorZero(),
		DirectX::XMVectorReplicate(1e-6f))) { return; }
	v_target_direction = DirectX::XMVector3Normalize(v_target_direction);

	// 現在の前方ベクトルを取得.
	DirectX::XMFLOAT3 curret_forward = Transform::GetForward();
	DirectX::XMVECTOR v_current_forward = DirectX::XMLoadFloat3(&curret_forward);
	v_current_forward = DirectX::XMVectorSetY(v_current_forward, 0.0f);
	v_current_forward = DirectX::XMVector3Normalize(v_current_forward);

	// 内積の取得.
	DirectX::XMVECTOR dot_product_vec = DirectX::XMVector3Dot(v_current_forward, v_target_direction);
	float dot = {};
	DirectX::XMStoreFloat(&dot, dot_product_vec);

	// 回転軸の取得.
	DirectX::XMVECTOR axis = DirectX::XMVector3Cross(v_current_forward, v_target_direction);

	DirectX::XMVECTOR rotation_quat;

	// 角度判定とクォータニオン生成.
	if (dot > 0.9999f)
	{
		// 回転角が0 (ベクトルがほぼ同じ).
		rotation_quat = DirectX::XMQuaternionIdentity();
	}
	else if (dot < -0.9999f)
	{
		// 回転角が180度 (ベクトルが反対).
		// Y軸周りの180度回転の軸として、現在の右ベクトル（Y軸と前方ベクトルの外積）を使用.
		DirectX::XMVECTOR v_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f);
		DirectX::XMVECTOR v_right = DirectX::XMVector3Cross(v_current_forward, v_up);

		// 回転軸と180度回転のクォータニオンを生成.
		rotation_quat = DirectX::XMQuaternionRotationAxis(v_right, DirectX::XM_PI);
	}
	else
	{
		// 通常の回転: 角度 = arccos(内積).
		DirectX::XMVECTOR v_angle_rad = DirectX::XMVectorACos(dot_product_vec);
		float angle_rad = {};
		DirectX::XMStoreFloat(&angle_rad, v_angle_rad);
		// 軸と角度を使用してクォータニオンを生成.
		rotation_quat = DirectX::XMQuaternionRotationAxis(axis, angle_rad);
	}

	// 現在のクォータニオンに乗算して回転を適用.
	DirectX::XMVECTOR v_current_quat = DirectX::XMLoadFloat4(&Quaternion);
	v_current_quat = DirectX::XMQuaternionMultiply(rotation_quat, v_current_quat);

	// 同期.
	v_current_quat = DirectX::XMQuaternionNormalize(v_current_quat);
	DirectX::XMStoreFloat4(&Quaternion, v_current_quat);
	UpdateRotationFromQuaternion();
}

// ローカル座標系の前方ベクトルを取得.
DirectX::XMFLOAT3 Transform::GetForward() const
{
	// ローカル前方ベクトル(0, 0, 1)をクォータニオンで回転.
	DirectX::XMVECTOR localForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR rotatedVector = DirectX::XMVector3Rotate(localForward, DirectX::XMLoadFloat4(&Quaternion));
	DirectX::XMFLOAT3 forward;
	DirectX::XMStoreFloat3(&forward, rotatedVector);
	return forward;
}

// ローカル座標系の上方ベクトルを取得.
DirectX::XMFLOAT3 Transform::GetUp() const
{
	// ローカル上方ベクトル(0, 1, 0)をクォータニオンで回転.
	DirectX::XMVECTOR localUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR rotatedVector = DirectX::XMVector3Rotate(localUp, DirectX::XMLoadFloat4(&Quaternion));
	DirectX::XMFLOAT3 up;
	DirectX::XMStoreFloat3(&up, rotatedVector);
	return up;
}

// ローカル座標系の右方ベクトルを取得.
DirectX::XMFLOAT3 Transform::GetRight() const
{
	// ローカル右方ベクトル(1, 0, 0)をクォータニオンで回転.
	DirectX::XMVECTOR localRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR rotatedVector = DirectX::XMVector3Rotate(localRight, DirectX::XMLoadFloat4(&Quaternion));
	DirectX::XMFLOAT3 right;
	DirectX::XMStoreFloat3(&right, rotatedVector);
	return right;
}

std::array<DirectX::XMFLOAT3, 3>Transform::GetAxisVector() const
{
	std::array<DirectX::XMFLOAT3, 3> axes;
	axes[0] = GetRight();
	axes[1] = GetUp();
	axes[2] = GetForward();
	return axes;
}

// 角度（度）でオイラー角を取得.
DirectX::XMFLOAT3 Transform::GetRotationDegrees() const
{
	return DirectX::XMFLOAT3(
		DirectX::XMConvertToDegrees(Rotation.x),
		DirectX::XMConvertToDegrees(Rotation.y),
		DirectX::XMConvertToDegrees(Rotation.z)
	);
}

// クォータニオンからオイラー角へ変換(ジンバルロックに注意).
void Transform::UpdateRotationFromQuaternion()
{
	DirectX::XMVECTOR quat = XMLoadFloat4(&Quaternion);

	// 正規化されていないクォータニオンの場合、0除算を防ぐ.
	// 浮動小数点誤差を防ぐためにクォータニオンを正規化.
	quat = DirectX::XMQuaternionNormalize(quat);

	DirectX::XMFLOAT4 qf;
	DirectX::XMStoreFloat4(&qf, quat);

	float roll, pitch, yaw;
	// 2* (qy*qz + qw*qx).
	float test = qf.y * qf.z + qf.w * qf.x;

	if (test > 0.499f) // 北極での特異点 (y=90°).
	{
		roll = 2.0f * atan2f(qf.y, qf.w); // roll (z) の計算.
		pitch = DirectX::XM_PIDIV2; // pitch (x) は π/2
		yaw = 0.0f; // yaw (y) は 0.
	}
	else if (test < -0.499f) // 南極での特異点 (y=-90°).
	{
		roll = -2.0f * atan2f(qf.y, qf.w); // roll (z) の計算.
		pitch = -DirectX::XM_PIDIV2; // pitch (x) は -π/2
		yaw = 0.0f; // yaw (y) は 0.
	}
	else
	{
		float sqx = qf.x * qf.x;
		float sqy = qf.y * qf.y;
		float sqz = qf.z * qf.z;

		// roll (z).
		roll = atan2f(2.0f * qf.w * qf.z - 2.0f * qf.x * qf.y, 1.0f - 2.0f * sqz - 2.0f * sqx);
		// ピッチ (x).
		pitch = asinf(2.0f * test);
		// ヨー (y).
		yaw = atan2f(2.0f * qf.w * qf.y - 2.0f * qf.x * qf.z, 1.0f - 2.0f * sqy - 2.0f * sqx);
	}

	// 注意: オイラー角の順序が PITCH(X) -> YAW(Y) -> ROLL(Z) の順であると仮定しています。.
	Rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
}

// オイラー角からクォータニオンへ変換.
void Transform::UpdateQuaternionFromRotation()
{
	DirectX::XMVECTOR rot = DirectX::XMLoadFloat3(&Rotation);
	DirectX::XMStoreFloat4(&Quaternion, DirectX::XMQuaternionRotationRollPitchYawFromVector(rot));
}

// デバッグ用 ToString.
#if _DEBUG
std::string Transform::ToString() const
{
	std::stringstream ss;
	ss << "Pos: (" << Position.x << ", " << Position.y << ", " << Position.z << ") "
		<< "Rot(Rad): (" << Rotation.x << ", " << Rotation.y << ", " << Rotation.z << ") "
		<< "Quat: (" << Quaternion.x << ", " << Quaternion.y << ", " << Quaternion.z << ", " << Quaternion.w << ") "
		<< "Scale: (" << Scale.x << ", " << Scale.y << ", " << Scale.z << ")";
	return ss.str();
}
#endif
