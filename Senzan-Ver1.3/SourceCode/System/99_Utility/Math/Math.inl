#pragma once

#include "Math.h"

namespace MyMath {
	inline const DirectX::XMFLOAT2 ConvertWorldPosToScreenPos(
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& proj,
		const DirectX::XMFLOAT3& worldPos)
	{
		using namespace DirectX;
		XMVECTOR vec_world = XMLoadFloat3(&worldPos);
		XMVECTOR clipSpace = XMVector3TransformCoord(vec_world, view * proj);

		if (XMVectorGetW(clipSpace) != 0.0f)
		{
			clipSpace = XMVectorDivide(clipSpace, XMVectorReplicate(XMVectorGetW(clipSpace)));
		}

		XMFLOAT2 screenPos;
		screenPos.x = (XMVectorGetX(clipSpace) * 0.5f + 0.5f) * WND_W;
		screenPos.y = (1.0f - (XMVectorGetY(clipSpace) * 0.5f + 0.5f)) * WND_H;

		return screenPos;
	}

	inline const DirectX::XMFLOAT3 ConvertScreenPosToWorldPos(
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& proj,
		const DirectX::XMFLOAT3& screenPos)
	{
		using namespace DirectX;
		float ndcX = (screenPos.x / WND_W) * 2.0f - 1.0f;
		float ndcY = (1.0f - (screenPos.y / WND_H)) * 2.0f - 1.0f;

		XMMATRIX inverseViewProj = XMMatrixInverse(nullptr, view * proj);

		XMVECTOR vec_world = XMVector3TransformCoord(XMVectorSet(ndcX, ndcY, screenPos.z, 1.0f), inverseViewProj);

		XMFLOAT3 worldPos;
		XMStoreFloat3(&worldPos, vec_world);

		return worldPos;
	}

	inline void CalcLookAtRotation(
		const DirectX::XMFLOAT3& currentPosition,
		const DirectX::XMFLOAT3& targetPosition,
		DirectX::XMFLOAT4& outQuaternion,
		const float& speed)
	{
		using namespace DirectX;
		XMVECTOR vec_current = XMLoadFloat3(&currentPosition);
		XMVECTOR vec_target = XMLoadFloat3(&targetPosition);
		XMVECTOR vec_direction = XMVectorSubtract(vec_target, vec_current);

		XMFLOAT3 direction;
		XMStoreFloat3(&direction, vec_direction);

		CalcLookAtRotation(direction, outQuaternion, speed);
	}

	inline void CalcLookAtRotation(
		const DirectX::XMFLOAT3& targetDirection,
		DirectX::XMFLOAT4& outQuaternion,
		const float& speed)
	{
		using namespace DirectX;
		XMVECTOR vec_direction = XMLoadFloat3(&targetDirection);

		if (XMVectorGetX(XMVector3LengthSq(vec_direction)) < 1e-6f)
		{
			return;
		}
		vec_direction = XMVector3Normalize(vec_direction);

		XMVECTOR vec_axis = XMVector3Cross(vec_direction, Axis::FORWARD_VECTOR);

		if (XMVectorGetX(XMVector3LengthSq(vec_axis)) < 1e-6f)
		{
			vec_axis = Axis::UP_VECTOR;
		}
		else
		{
			vec_axis = XMVector3Normalize(vec_axis);
		}

		float dot = XMVectorGetX(XMVector3Dot(vec_direction, Axis::FORWARD_VECTOR));
		dot = std::clamp(dot, -1.0f, 1.0f);
		float angle = std::acosf(dot);

		XMVECTOR targetQuaternion = XMQuaternionRotationAxis(vec_axis, angle);

		XMVECTOR vec_outQuaternion = XMLoadFloat4(&outQuaternion);
		if (XMVectorGetX(XMQuaternionLengthSq(vec_outQuaternion)) < 1e-6f)
		{
			vec_outQuaternion = XMQuaternionIdentity();
		}

		if (XMVectorGetX(XMQuaternionDot(vec_outQuaternion, targetQuaternion)) < 0.0f)
		{
			targetQuaternion = XMVectorNegate(targetQuaternion);
		}

		vec_outQuaternion = XMQuaternionSlerp(vec_outQuaternion, targetQuaternion, speed);
		XMStoreFloat4(&outQuaternion, vec_outQuaternion);
	}

	// 角度を -180.0f から 180.0f の範囲に正規化する.
	float NormalizeAngleDegrees(float angle)
	{
		static constexpr float FULL_CIRCLE = 360.0f;

		angle = std::fmod(angle, FULL_CIRCLE);

		if (angle > 180.0f)
		{
			angle -= FULL_CIRCLE;
		}
		else if (angle <= -180.0f)
		{
			angle += FULL_CIRCLE;
		}
		return angle;
	}
}