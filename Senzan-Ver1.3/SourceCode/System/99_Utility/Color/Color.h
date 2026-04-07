#pragma once
#include "Global.h"

namespace ColorPreset
{
	// Alphaアニメーション用.
	const DirectX::XMFLOAT3 White3      = { 1.0f,  1.0f,  1.0f };
	const DirectX::XMFLOAT3 Selected	= { 0.07f, 0.66f, 1.0f };
	const DirectX::XMFLOAT3 Black3		= { 0.0f, 0.0f, 0.0f};

	// 固定.
	const DirectX::XMFLOAT4 UnActive	= { 0.0f, 0.0f, 0.0f, 0.5f };
	const DirectX::XMFLOAT4 TitleCyan	= { 0.1f, 0.6f, 0.896f, 1.0f };
	const DirectX::XMFLOAT4 Green		= { 0.0f, 0.9f, 0.2f, 1.0f };
	const DirectX::XMFLOAT4 WarmYellow	= { 1.0f, 0.75f, 0.2f, 1.0f };
	const DirectX::XMFLOAT4 Invisible	= { 0.0f, 0.0f, 0.0f, 0.0f };
	const DirectX::XMFLOAT4 White4		= { 1.0f, 1.0f, 1.0f, 1.0f };
	const DirectX::XMFLOAT4 LightGray	= { 0.8f, 0.8f, 0.8f, 1.0f };
	const DirectX::XMFLOAT4 Black		= { 0.0f, 0.0f, 0.0f, 1.0f };
}

namespace ColorUtil {
	inline DirectX::XMFLOAT4 RGBA(
		const DirectX::XMFLOAT3& rgb,
		float a)
	{
		return DirectX::XMFLOAT4(rgb.x, rgb.y, rgb.z, a);
	}
}
