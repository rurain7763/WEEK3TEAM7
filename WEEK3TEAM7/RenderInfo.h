#pragma once

#include "Transform.h"
#include "Object.h"
#include "FName.h"

struct FRenderInfo
{
	FName StaticMeshName;
	EPrimitive ePrimitive;
	FMatrix WorldTransformMatrix;
	FObjectID ObejctID;
	FVector4 Color;
};
