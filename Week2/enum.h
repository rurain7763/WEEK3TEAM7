#pragma once

enum class EAxis : int { X = 0, Y = 1, Z = 2 };

enum class EPrimitive
{
	EP_Sphere,
	EP_Cube,
	EP_Triangle,
	EP_GizmoArrow,
	EP_Circle,
};

enum EGIZMO_AXIS //어떤축이 선택되었는지
{
	NONE,
	X,
	Y,
	Z
};

enum EGIZMO_TYPE {
	TRANSLATE,
	ROTATE,
	SCALE,
};
