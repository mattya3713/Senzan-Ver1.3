#pragma once
#include "Constant.h"
#include <cmath>
#include <DirectXMath.h>

namespace MyMath {

	// floatなどの!=を近似値比較する.
	template <typename T>
	inline bool IsNearlyEqual(T a, T b, T epsilon = EPSILON_E4) noexcept
	{
		// aとbの差の絶対値が、許容誤差以内であればtrue.
		return std::abs(a - b) <= epsilon;
	}

	// 2Dベクトルaがbに近いかをチェック.
	inline bool IsVector2NearlyZero(const DirectX::XMFLOAT2& a, float b = 0.0f, float epsilon = EPSILON_E4)
	{
		return IsNearlyEqual(a.x, b, epsilon) && IsNearlyEqual(a.y, b, epsilon);
	}

	// 3Dベクトルaがbに近いかをチェック.
	inline bool IsVector3NearlyZero(const DirectX::XMFLOAT3& a, float b = 0.0f, float epsilon = EPSILON_E4)
	{
		return IsNearlyEqual(a.x, b, epsilon) && IsNearlyEqual(a.y, b, epsilon) && IsNearlyEqual(a.z, b, epsilon);
	}

	inline float NormalizeAngleDegrees(float angle);

	/*************************************************************
* @brief	3Dベクトルを水平成分(XZ)のみで正規化し、2Dベクトルとして返す。.
* @return	正規化された2Dベクトル (DirectX::XMFLOAT2)。.
* @param[in]	vec	：正規化したい3Dベクトル。.
	* ************************************************************/
	inline DirectX::XMFLOAT2 NormalizeVector3To2D(const DirectX::XMFLOAT3& vec)
	{
		// X, Z 成分の長さの二乗を計算.
		float lengthSq = vec.x * vec.x + vec.z * vec.z;

		// 長さ（L2ノルム）を計算.
		float length = std::sqrt(lengthSq);

		// 長さが0でなければ正規化し、そうでなければゼロベクトルを返す.
		if (length > EPSILON_E4) // ゼロ除算を避けるための微小値チェック.
		{
			return DirectX::XMFLOAT2(vec.x / length, vec.z / length);
		}
		return DirectX::XMFLOAT2(0.0f, 0.0f);
	}

	/*************************************************************
	* @brief	値を減少(増加)させる.
	* @return	中央値になったかどうか.	
	* @param[inout] Value	：増減させたい値.
	* @param[in]	Median	：中央値.
	* @param[in]	Amount	：増減させる量.
	* ************************************************************/
	template<typename T>
	inline bool DecreaseToValue(T& Value, T Median, T Amount) noexcept
	{
		if (std::abs(Value - Median) <= Amount)
		{
			Value = Median;
			return true;
		}

		if (Value >= Median)
		{
			Value -= Amount;
		}
		else
		{
			Value += Amount;
		}
		return false;
	}

	/*************************************************************
	* @brief ワールドをスクリーン座標に変換して返す.
	* @param[in]	worldPos	：ワールド座標.
	* ************************************************************/
	const DirectX::XMFLOAT2 ConvertWorldPosToScreenPos(
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& proj,
		const DirectX::XMFLOAT3& worldPos);

	/*************************************************************
	* @brief スクリーン座標をワールド座標に変換して返す.
	* @param[in]	screenPos	：スクリーン座標.
	* ************************************************************/
	const DirectX::XMFLOAT3 ConvertScreenPosToWorldPos(
		const DirectX::XMMATRIX& view,
		const DirectX::XMMATRIX& proj,
		const DirectX::XMFLOAT3& screenPos);

	/*************************************************************
	* @brief 目標の座標に向かって回転する値を計算して返す.
	* @param[in]	currentPosition	：現在座標.
	* @param[in]	targetPosition	：目標座標.
	* @param[out]	outQuaternion	：現在のクォータニオン.
	* @param[in]	speed			：補間速度.
	*************************************************************/
	void CalcLookAtRotation(
		const DirectX::XMFLOAT3& currentPosition,
		const DirectX::XMFLOAT3& targetPosition,
		DirectX::XMFLOAT4& outQuaternion,
		const float& speed = 0.1f);

	/*************************************************************
	* @brief 目標の方向に向かって回転するクォータニオンを計算する.
	* @param[in]	targetDirection	：目標の方向.
	* @param[out]	outQuaternion	：現在のクォータニオン.
	* @param[in]	speed			：補間速度.
	*************************************************************/
	void CalcLookAtRotation(
		const DirectX::XMFLOAT3& targetDirection,
		DirectX::XMFLOAT4& outQuaternion,
		const float& speed = 0.1f);
}
#include "Math.inl"
