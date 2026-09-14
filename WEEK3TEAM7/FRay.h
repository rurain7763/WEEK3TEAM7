#pragma once

#include "Vector.h"

struct FRay
{
	FVector Origin;
	FVector Direction;

	FRay() = default;
	FRay(const FVector& InOrigin, const FVector& InDirection)
		: Origin(InOrigin)
		, Direction(InDirection) 
	{
	}
};

struct FAABB
{
	FVector Min;
	FVector Max;

	FAABB() = default;
	FAABB(const FVector& InMin, const FVector& InMax)
		: Min(InMin)
		, Max(InMax)
	{
	}
};
