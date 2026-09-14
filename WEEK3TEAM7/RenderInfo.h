#pragma once

#include "Transform.h"
#include "Object.h"
#include "FName.h"
#include "Assets.h"
#include "TArray.h"

struct FRenderInfo
{
	FName StaticMeshName;
	TSharedPtr<FTexture2DAsset> Texture;
	EPrimitive ePrimitive;
	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
};

struct FRenderQuadInfo
{
	FMatrix Model;
	FVector4 Color = { 1.f, 1.f, 1.f, 1.f };
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;
	FVector4 SubUV = { 0.f, 0.f, 1.f, 1.f };
	bool EnableDepthTest = true;
	bool EnableDepthWrite = true;
};

// 이번 프레임에 그릴 것들을 한데 모은다. 소유자는 FGraphicsManager.
struct FRenderCollector
{
	enum { DEFAULT_RESERVE_MEM = 1024U };

	TArray<FRenderInfo>     RenderInfos;   // 메시 패스
	TArray<FRenderQuadInfo> QuadInfos;     // 쿼드 패스
};