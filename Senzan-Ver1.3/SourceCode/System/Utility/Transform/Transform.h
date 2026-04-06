#pragma once

#include <DirectXMath.h>
#include <string>
#include <sstream>

struct Transform
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Rotation;		// オイラー角(ラジアン).
	DirectX::XMFLOAT4 Quaternion;	// クォータニオン.
	DirectX::XMFLOAT3 Scale;

	Transform()
		: Position(0.0f, 0.0f, 0.0f)
		, Rotation(0.0f, 0.0f, 0.0f)
		, Quaternion(0.0f, 0.0f, 0.0f, 1.0f)
		, Scale(1.0f, 1.0f, 1.0f)
	{
	}

	Transform(
		const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& rotation,
		const DirectX::XMFLOAT4& quaternion,
		const DirectX::XMFLOAT3& scale)
		: Position(position)
		, Rotation(rotation)
		, Quaternion(quaternion)
		, Scale(scale)
	{
	}

	// ゲッター.
	DirectX::XMFLOAT3 GetForward() const;
	DirectX::XMFLOAT3 GetUp() const;
	DirectX::XMFLOAT3 GetRight() const;
	std::array<DirectX::XMFLOAT3, 3> GetAxisVector() const;
	DirectX::XMFLOAT3 GetRotationDegrees() const; // 角度でオイラー角を取得.

	// ワールド行列を生成.
	inline DirectX::XMMATRIX GetWorldMatrix() const
	{
		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Scale));
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Quaternion));
		DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Position));

		return scaleMatrix * rotationMatrix * translationMatrix;
	}

	// セッター.
	void SetPosition(const DirectX::XMFLOAT3& newPosition);
	void SetRotation(const DirectX::XMFLOAT3& eulerAngles); // ラジアンで設定.
	void SetRotationX(float X); // ラジアンで設定.
	void SetRotationY(float Y); // ラジアンで設定.
	void SetRotationZ(float Z); // ラジアンで設定.
	void SetRotationDegrees(const DirectX::XMFLOAT3& eulerAnglesInDegrees); // 角度で回転.
	void SetQuaternion(const DirectX::XMFLOAT4& newQuaternion);
	void SetScale(const DirectX::XMFLOAT3& newScale);

	// 移動・回転.
	void Translate(const DirectX::XMFLOAT3& translation);
	void Rotate(const DirectX::XMFLOAT3& eulerAngles); // ラジアンで回転を適用.
	void RotateDegrees(const DirectX::XMFLOAT3& eulerAnglesInDegrees); // 角度で回転を適用.
	void Rotate(const DirectX::XMFLOAT4& quaternion); // クォータニオンで回転を適用.
	void RotateToDirection(const DirectX::XMFLOAT3& NormVecDirection);

	// オイラー角とクォータニオンの同期.
	void UpdateRotationFromQuaternion();
	void UpdateQuaternionFromRotation();


	inline Transform operator+(const Transform& other) const
	{
		using namespace DirectX;
		DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot1 = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&Scale);

		DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&other.Position);
		DirectX::XMVECTOR rot2 = DirectX::XMLoadFloat3(&other.Rotation);
		DirectX::XMVECTOR quat2 = DirectX::XMLoadFloat4(&other.Quaternion);
		DirectX::XMVECTOR scale2 = DirectX::XMLoadFloat3(&other.Scale);

		XMFLOAT3 newPos, newRot, newScale;
		XMFLOAT4 newQuat;

		DirectX::XMStoreFloat3(&newPos, DirectX::XMVectorAdd(pos1, pos2)); // 位置は加算.
		DirectX::XMStoreFloat3(&newRot, DirectX::XMVectorAdd(rot1, rot2)); // オイラー角は加算 (非推奨だが実装を維持).
		DirectX::XMStoreFloat4(&newQuat, DirectX::XMQuaternionMultiply(quat1, quat2)); // クォータニオンは乗算で結合.
		DirectX::XMStoreFloat3(&newScale, DirectX::XMVectorAdd(scale1, quat2)); // スケールは加算.

		return Transform{ newPos, newRot, newQuat, newScale };
	}

	inline Transform operator-(const Transform& other) const
	{
		DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot1 = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&Scale);

		DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&other.Position);
		DirectX::XMVECTOR rot2 = DirectX::XMLoadFloat3(&other.Rotation);
		DirectX::XMVECTOR quat2 = DirectX::XMLoadFloat4(&other.Quaternion);
		DirectX::XMVECTOR scale2 = DirectX::XMLoadFloat3(&other.Scale);

		DirectX::XMFLOAT3 newPos, newRot, newScale;
		DirectX::XMFLOAT4 newQuat;

		DirectX::XMStoreFloat3(&newPos, DirectX::XMVectorSubtract(pos1, pos2));
		DirectX::XMStoreFloat3(&newRot, DirectX::XMVectorSubtract(rot1, rot2));
		// クォータニオンの減算は無意味なため、ここでは単純に成分減算を維持 (ただし非推奨).
		DirectX::XMStoreFloat4(&newQuat, DirectX::XMVectorSubtract(quat1, quat2));
		DirectX::XMStoreFloat3(&newScale, DirectX::XMVectorSubtract(scale1, scale2));

		return Transform{ newPos, newRot, newQuat, newScale };
	}

	inline Transform operator*(const Transform& other) const
	{
		DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot1 = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&Scale);

		DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&other.Position);
		DirectX::XMVECTOR rot2 = DirectX::XMLoadFloat3(&other.Rotation);
		DirectX::XMVECTOR quat2 = DirectX::XMLoadFloat4(&other.Quaternion);
		DirectX::XMVECTOR scale2 = DirectX::XMLoadFloat3(&other.Scale);

		DirectX::XMFLOAT3 newPos, newRot, newScale;
		DirectX::XMFLOAT4 newQuat;

		// 位置とスケールは乗算で結合 (これはベクトル成分の乗算として解釈).
		DirectX::XMStoreFloat3(&newPos, DirectX::XMVectorMultiply(pos1, pos2));
		DirectX::XMStoreFloat3(&newRot, DirectX::XMVectorMultiply(rot1, rot2));
		DirectX::XMStoreFloat3(&newScale, DirectX::XMVectorMultiply(scale1, scale2));

		// クォータニオンは乗算で回転を結合 (これは正しい).
		DirectX::XMStoreFloat4(&newQuat, DirectX::XMQuaternionMultiply(quat1, quat2));

		return Transform{ newPos, newRot, newQuat, newScale };
	}

	inline Transform operator*(const float scalar) const
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&Scale);
		DirectX::XMVECTOR scalarVec = DirectX::XMVectorReplicate(scalar);

		DirectX::XMFLOAT3 newPos, newRot, newScale;
		DirectX::XMFLOAT4 newQuat;

		DirectX::XMStoreFloat3(&newPos, DirectX::XMVectorMultiply(pos, scalarVec));
		DirectX::XMStoreFloat3(&newRot, DirectX::XMVectorMultiply(rot, scalarVec));
		DirectX::XMStoreFloat4(&newQuat, DirectX::XMVectorMultiply(quat, scalarVec));
		DirectX::XMStoreFloat3(&newScale, DirectX::XMVectorMultiply(scale, scalarVec));

		return Transform{ newPos, newRot, newQuat, newScale };
	}

	inline Transform operator/(const float scalar) const
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&Scale);
		DirectX::XMVECTOR scalarVec = DirectX::XMVectorReplicate(scalar);

		DirectX::XMFLOAT3 newPos, newRot, newScale;
		DirectX::XMFLOAT4 newQuat;

		DirectX::XMStoreFloat3(&newPos, DirectX::XMVectorDivide(pos, scalarVec));
		DirectX::XMStoreFloat3(&newRot, DirectX::XMVectorDivide(rot, scalarVec));
		DirectX::XMStoreFloat4(&newQuat, DirectX::XMVectorDivide(quat, scalarVec));
		DirectX::XMStoreFloat3(&newScale, DirectX::XMVectorDivide(scale, scalarVec));

		return Transform{ newPos, newRot, newQuat, newScale };
	}

	inline Transform& operator+=(const Transform& other)
	{
		DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot1 = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&Scale);

		DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&other.Position);
		DirectX::XMVECTOR rot2 = DirectX::XMLoadFloat3(&other.Rotation);
		DirectX::XMVECTOR quat2 = DirectX::XMLoadFloat4(&other.Quaternion);
		DirectX::XMVECTOR scale2 = DirectX::XMLoadFloat3(&other.Scale);

		// 位置、回転(オイラー角)、スケールは加算.
		DirectX::XMStoreFloat3(&Position, DirectX::XMVectorAdd(pos1, pos2));
		DirectX::XMStoreFloat3(&Rotation, DirectX::XMVectorAdd(rot1, rot2));
		DirectX::XMStoreFloat3(&Scale, DirectX::XMVectorAdd(scale1, scale2));

		// クォータニオンは乗算 (回転の結合).
		DirectX::XMStoreFloat4(&Quaternion, DirectX::XMQuaternionMultiply(quat1, quat2));

		UpdateRotationFromQuaternion();

		return *this;
	}

	inline Transform& operator-=(const Transform& other)
	{
		DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot1 = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&Scale);

		DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&other.Position);
		DirectX::XMVECTOR rot2 = DirectX::XMLoadFloat3(&other.Rotation);
		DirectX::XMVECTOR quat2 = DirectX::XMLoadFloat4(&other.Quaternion);
		DirectX::XMVECTOR scale2 = DirectX::XMLoadFloat3(&other.Scale);

		DirectX::XMStoreFloat3(&Position, DirectX::XMVectorSubtract(pos1, pos2));
		DirectX::XMStoreFloat3(&Rotation, DirectX::XMVectorSubtract(rot1, rot2));
		DirectX::XMStoreFloat4(&Quaternion, DirectX::XMVectorSubtract(quat1, quat2)); // (非推奨だが実装を維持).
		DirectX::XMStoreFloat3(&Scale, DirectX::XMVectorSubtract(scale1, scale2));

		return *this;
	}

	inline Transform& operator*=(const float scalar)
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&Scale);
		DirectX::XMVECTOR scalarVec = DirectX::XMVectorReplicate(scalar);

		DirectX::XMStoreFloat3(&Position, DirectX::XMVectorMultiply(pos, scalarVec));
		DirectX::XMStoreFloat3(&Rotation, DirectX::XMVectorMultiply(rot, scalarVec));
		DirectX::XMStoreFloat4(&Quaternion, DirectX::XMVectorMultiply(quat, scalarVec));
		DirectX::XMStoreFloat3(&Scale, DirectX::XMVectorMultiply(scale, scalarVec));

		return *this;
	}

	inline Transform& operator/=(const float scalar)
	{
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&Position);
		DirectX::XMVECTOR rot = DirectX::XMLoadFloat3(&Rotation);
		DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&Quaternion);
		DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&Scale);
		DirectX::XMVECTOR scalarVec = DirectX::XMVectorReplicate(scalar);

		DirectX::XMStoreFloat3(&Position, DirectX::XMVectorDivide(pos, scalarVec));
		DirectX::XMStoreFloat3(&Rotation, DirectX::XMVectorDivide(rot, scalarVec));
		DirectX::XMStoreFloat4(&Quaternion, DirectX::XMVectorDivide(quat, scalarVec));
		DirectX::XMStoreFloat3(&Scale, DirectX::XMVectorDivide(scale, scalarVec));

		return *this;
	}

	//---------------------------------------------------------------------------------
	// 比較演算子オーバーロードの実装.
	//---------------------------------------------------------------------------------

	inline bool operator==(const Transform& other) const
	{
		using namespace DirectX;
		return DirectX::XMVector3Equal(XMLoadFloat3(&Position), DirectX::XMLoadFloat3(&other.Position)) &&
			DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&Rotation), DirectX::XMLoadFloat3(&other.Rotation)) &&
			DirectX::XMVector4Equal(DirectX::XMLoadFloat4(&Quaternion), DirectX::XMLoadFloat4(&other.Quaternion)) &&
			DirectX::XMVector3Equal(DirectX::XMLoadFloat3(&Scale), DirectX::XMLoadFloat3(&other.Scale));
	}

	inline bool operator!=(const Transform& other) const
	{
		return !(*this == other);
	}


#if _DEBUG
	// デバッグ用に内容を文字列化.
	std::string ToString() const;
#endif
};
