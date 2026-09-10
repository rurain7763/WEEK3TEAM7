#pragma once
#include "MathUtility.h"

typedef struct FVector
{
    float x, y, z;
	FVector() : x(0), y(0), z(0) {}

	FVector(float n) : x(n), y(n), z(n){}
    FVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    const FVector operator-(const FVector& Others) const
    {
        return FVector(x - Others.x, y - Others.y, z - Others.z);
    }

    void operator+=(const FVector& Others)
    {
        x += Others.x;
        y += Others.y;
        z += Others.z;
    }

    void operator-=(const FVector& Others)
    {
        x -= Others.x;
        y -= Others.y;
        z -= Others.z;
    }

	FVector operator-() const
	{
		return FVector(-x, -y, -z);
	}

	//내적
    inline static float dot(const FVector& A, const FVector& B)
    {
        return A.x * B.x + A.y * B.y + A.z * B.z;
    }

	//외적
	inline static FVector cross(const FVector& A, const FVector& B)
	{
		return	FVector(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
	}

	float Length() const { return FMath::Sqrt(x * x + y * y + z * z); }
	float LengthSquared() const { return x * x + y * y + z * z; }

	void Normalize()
	{
		float len = Length();
		x /= len;
		y /= len;
		z /= len;
	}

	inline bool IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return LengthSquared() < Tolerance;
	}
	
} FVector3;

inline const FVector operator*(const FVector& v, float f)
{
    return FVector(v.x * f, v.y * f, v.z * f);
}

inline const FVector operator*(float f, const FVector& v)
{
    return FVector(v.x * f, v.y * f, v.z * f);
}

inline FVector operator+(const FVector& A, const FVector& B)
{
	return FVector(A.x + B.x, A.y + B.y, A.z + B.z);
}


//Vector 4
typedef struct FVector4
{
	float x, y, z, w;
	FVector4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) : x(_x), y(_y), z(_z), w(_w) {}

	const FVector4 operator-(const FVector4& Others) const
	{
		return FVector4(x - Others.x, y - Others.y, z - Others.z, w - Others.w);
	}

	void operator+=(const FVector4& Others)
	{
		x += Others.x;
		y += Others.y;
		z += Others.z;
		w += Others.w;
	}

	void operator-=(const FVector4& Others)
	{
		x -= Others.x;
		y -= Others.y;
		z -= Others.z;
		w -= Others.w;
	}

	//내적
	inline static float dot(const FVector4& A, const FVector4& B)
	{
		return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
	}

	//4차원에는 외적이 없다.

	float Length() const { return FMath::Sqrt(x * x + y * y + z * z + w * w); }

} FVector4;
