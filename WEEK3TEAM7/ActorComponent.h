#pragma once

#include "Object.h"

struct FRenderInfo;
class FRenderCollector;

class UActorComponent : public UObject
{
	REFLECT_CLASS(UActorComponent, UObject)
public:
	UActorComponent();
	virtual ~UActorComponent();

	void SetOwner(AActor* owner);
	AActor* GetOwner() const;

	// Todo: Make as pure class
	virtual void Tick(float deltaTime);
	virtual void Render(FRenderCollector& RenderCollector);
	virtual void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const;

protected:
	AActor* mOwner;
};

