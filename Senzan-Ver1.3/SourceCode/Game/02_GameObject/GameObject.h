#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <d3d11.h>

#include "Engine/Utility/ComPtr/ComPtr.h"
#include "Engine/Utility/Transform/Transform.h"

/**********************************************************************************
* @author    : mattya3713.
* @date      : 2026/04/17.
* @brief     : ゲームオブジェクト基底クラス.
**********************************************************************************/
class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Update() = 0;
	virtual void LateUpdate() = 0;
	virtual void Draw() = 0;

public: 

    // Transformを取得.
	inline const std::shared_ptr<Transform>& GetTransform() const noexcept { return m_spTransform; }	
	// Transformをコピー設定.
    void SetTransform(const Transform& TransformValue);											
	// shared_ptrのTransformを設定.
	inline void SetTransform(const std::shared_ptr<Transform>& transformPtr) { m_spTransform = transformPtr; }		
    // shared_ptrのTransformをムーブ設定.
	inline void SetTransform(std::shared_ptr<Transform>&& transformPtr) { m_spTransform = std::move(transformPtr); }
	
    // 位置を取得.
	inline const DirectX::XMFLOAT3& GetPosition() const noexcept { return m_spTransform->Position; }
	inline float GetPositionX() const noexcept { return m_spTransform->Position.x; }
	inline float GetPositionY() const noexcept { return m_spTransform->Position.y; }
	inline float GetPositionZ() const noexcept { return m_spTransform->Position.z; }

    // 位置を設定.
	void SetPosition(const DirectX::XMFLOAT3& Position);
	void SetPosition(float X, float Y, float Z);
	void SetPositionX(float X);
	void SetPositionY(float Y);
	void SetPositionZ(float Z);

	// 位置を加算.
	void AddPosition(const DirectX::XMFLOAT3& Position);
	void AddPosition(float X, float Y, float Z);
	void AddPositionX(float X);
	void AddPositionY(float Y);
	void AddPositionZ(float Z);


    // 回転を取得.
	inline const DirectX::XMFLOAT3& GetRotation() const noexcept { return m_spTransform->Rotation; }
	inline float GetRotationX() const noexcept { return m_spTransform->Rotation.x; }
	inline float GetRotationY() const noexcept { return m_spTransform->Rotation.y; }
	inline float GetRotationZ() const noexcept { return m_spTransform->Rotation.z; }
	inline const DirectX::XMFLOAT4& Quaternion() const noexcept { return m_spTransform->Quaternion; }

	// 回転を設定.
	void SetRotation(const DirectX::XMFLOAT3& Rotation);
	void SetRotation(float X, float Y, float Z);
	void SetRotationX(float X);
	void SetRotationY(float Y);
	void SetRotationZ(float Z);
	void SetRotationAroundAxis(const DirectX::XMFLOAT3& Axis, float Angle);


    // 拡縮を取得.
	inline const DirectX::XMFLOAT3& GetScale() const noexcept { return m_spTransform->Scale; }
	inline float GetScaleX() const noexcept { return m_spTransform->Scale.x; }
	inline float GetScaleY() const noexcept { return m_spTransform->Scale.y; }
	inline float GetScaleZ() const noexcept { return m_spTransform->Scale.z; }

	// 拡縮を設定.
	void SetScale(const DirectX::XMFLOAT3& Scale);
	void SetScale(float XYZ);
	void SetScale(float X, float Y, float Z);
	void SetScaleX(float X);
	void SetScaleY(float Y);
	void SetScaleZ(float Z);


	// タグを取得/設定.
	const std::string& GetTag()const;
    void SetTag(const std::string& Tag);

	// 更新有効状態の取得/設定.
	bool IsActive() const;
	void SetIsActive(bool IsActive);

	// 描画有効状態の取得/設定.
	bool IsRenderActive() const;
	void SetIsRenderActive(bool IsActive);
	
	// 個別の時間倍率を設定.
	void SetTimeScale(float NewTimeScale);
	float GetTimeScale();

    // 角度を目標値へ回転補間.
    void RotetToTarget(float TargetRote, float RotetionSpeed);

protected:
	float GetDelta();

protected:
	std::shared_ptr<Transform> m_spTransform;
	std::string			m_Tag;

    // 時間倍率(通常1f, 2fで倍速, -1でワールド時間倍率を使用).
	float m_TimeScale;

	bool m_IsActive;
	bool m_IsRenderActive;


   MyComPtr<ID3D11DeviceContext> m_pContext11;
};

