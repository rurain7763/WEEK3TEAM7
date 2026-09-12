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
