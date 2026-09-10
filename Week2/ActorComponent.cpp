
#include "ActorComponent.h"

UActorComponent::UActorComponent()
	: mOwner(nullptr)
{
}

UActorComponent::~UActorComponent()
{
}

void UActorComponent::SetOwner(AActor* owner)
{
	assert(mOwner == nullptr);

	mOwner = owner;
}

AActor* UActorComponent::GetOwner() const
{
	return mOwner;
}

void UActorComponent::Update(TArray<FRenderInfo>* outRenderInfos)
{
	// Todo: Do nothing, must override, some components may not call Update()
	// assert(false);
}

void UActorComponent::GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	// Todo: Do nothing, must override, some components may not call GetRenderInfos()
	// assert(false);
}
