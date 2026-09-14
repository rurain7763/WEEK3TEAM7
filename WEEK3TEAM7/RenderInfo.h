#pragma once

#include "Transform.h"
#include "Object.h"
#include "FName.h"
#include "Assets.h"

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