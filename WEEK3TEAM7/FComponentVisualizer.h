#pragma once

#include "RenderInfo.h"
#include "ActorComponent.h"
#include "TMap.h"
#include "Object.h"
#include "UTextComponent.h"
#include "EngineMathLibrary.h"

class FComponentVisualizer
{
public:
	virtual ~FComponentVisualizer() = default;

	virtual void VisualizeComponent(UActorComponent* Component, FRenderCollector& RenderCollector) = 0;
};

class FSpotLightComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void VisualizeComponent(UActorComponent* Component, FRenderCollector& RenderCollector) override
	{
		USpotLightComponent* SpotLightComponent = Component->Cast<USpotLightComponent>();
		if (!SpotLightComponent)
		{
			return;
		}

		const FTransform Transform = SpotLightComponent->GetTransformMatrix();
		const FVector Origin = Transform.Location;
		const FMatrix Rotation = FMatrix::Rotate(Transform.Rotation);
		const FVector Forward = Rotation.GetUnitAxis(EAxis::X);
		const FVector Right = Rotation.GetUnitAxis(EAxis::Y);
		const FVector Up = Rotation.GetUnitAxis(EAxis::Z);
		const float Range = SpotLightComponent->GetRange();

		if (Range <= 0.f) 
		{ 
			return; 
		}

		const float OuterAngle = FMath::Clamp(SpotLightComponent->GetOuterConeAngle(), 0.f, 89.f);
		const float InnerAngle = FMath::Clamp(SpotLightComponent->GetInnerConeAngle(), 0.f, OuterAngle);

		constexpr int32 CircleSegments = 32;
		constexpr int32 ArcSegments = 16;

		auto AddLine = [&](const FVector& Start, const FVector& End, const FVector4& Color)
		{
			FRenderLineInfo Line;
			Line.Start = Start;
			Line.End = End;
			Line.Color = Color;
			Line.Thickness = 5.0f;
			RenderCollector.LineInfos.Add(Line);
		};

		auto DrawCone = [&](float AngleDegrees, const FVector4& Color)
		{
			const float Theta = FMath::DegreesToRadians(AngleDegrees);
			const float Height = Range * FMath::Cos(Theta);
			const float Radius = Range * FMath::Sin(Theta);

			FVector CirclePoints[CircleSegments];
			GenerateCircleVertices([&](int32 Index, const FVector2& CircleVertex) {
				CirclePoints[Index] = Origin + Forward * Height + Right * (CircleVertex.X * Radius) + Up * (CircleVertex.Y * Radius);
			}, 1.f, CircleSegments);

			for (int32 Index = 0; Index < CircleSegments; ++Index)
			{
				int32 NextIndex = (Index + 1) % CircleSegments;
				AddLine(CirclePoints[Index], CirclePoints[NextIndex], Color);
			}

			for (int32 Index = 0; Index < CircleSegments; Index += CircleSegments / 4)
			{
				AddLine(Origin, CirclePoints[Index], Color);
			}
		};

		DrawCone(OuterAngle, FVector4(1.f, 1.f, 1.f, 1.f));
		if (InnerAngle > 0.f && InnerAngle < OuterAngle)
		{
			DrawCone(InnerAngle, FVector4(1.f, 1.f, 1.f, 1.f));
		}
	}
};

class FComponentVisualizerModule
{
public:
	void RegisterVisualizer(const FClassInfo* ComponentClass, TSharedPtr<FComponentVisualizer> Visualizer)
	{
		mVisualizers.Add(ComponentClass, Visualizer);
	}

	void UnregisterVisualizer(const FClassInfo* ComponentClass)
	{
		mVisualizers.Remove(ComponentClass);
	}

	FComponentVisualizer* FindVisualizer(const FClassInfo* ComponentClass)
	{
		TSharedPtr<FComponentVisualizer>* Visualizer = mVisualizers.Find(ComponentClass);
		return Visualizer ? Visualizer->get() : nullptr;
	}

private:
	TMap<const FClassInfo*, TSharedPtr<FComponentVisualizer>> mVisualizers;
};