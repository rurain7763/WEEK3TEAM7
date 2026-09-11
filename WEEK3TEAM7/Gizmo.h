#pragma once

#include "Vector.h"
#include "RenderInfo.h"
#include "TArray.h"
#include "Transform.h"
#include "enum.h"

class AActor;
class URenderer;

enum class AxisNumber { None, X, Y, Z, Cameara };

class FGizmo
{
public:
    FGizmo(URenderer& InRenderer);
    void SetWorldMode(bool bInWorldMode);
    void SetOperation(EGIZMO_TYPE Operation);
    EGIZMO_TYPE GetOperation() const;

    void Update(AActor* TargetActor);
    void Draw(AActor* TargetActor, const FVector& CameraPosition, const FMatrix& ViewProjection);
    bool IsMouseOverHandle() const;
    bool IsDragging() const { return bIsSelected; }
    void Reset();

private:
    // Draw에서 그린 선분과 해당 축만 입력 단계에 공유한다.
    struct FHandleSegment
    {
        FVector2 Start, End;
        FVector Direction;
        AxisNumber Axis;
    };
    TArray<FHandleSegment> HandleScreenSegments;
    static constexpr float HandleHitRadius = 5.f;
    URenderer& Renderer;
    bool bWorldMode = true;
    EGIZMO_TYPE CurrentOperation = EGIZMO_TYPE::TRANSLATE;
    FVector2 PrevMousePos;
    FVector2 HandleScreenDirection;
    FVector AxisDirection;
    bool bIsSelected = false;
    bool bIsHoveredAxis = false;
    AxisNumber SelectedAxis = AxisNumber::None;
    AxisNumber HoveredAxis = AxisNumber::None;
    int32 TargetUUID = -1;
};