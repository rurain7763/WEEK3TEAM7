#pragma once

#include "UTextComponent.h"
#include <wrl/client.h>
#include "Assets.h"

class UAtlasAnimationComponent : public UPlaneComponent
{
	REFLECT_CLASS(UAtlasAnimationComponent, UPlaneComponent)

public:
	UAtlasAnimationComponent();

	using UPrimitiveComponent::Initialize;
	void Initialize(EPrimitive PrimitiveType, const TSharedPtr<FSpriteAtlasAsset>& textureAsset);

	void Play(int32 StartFrame = 0, bool bIsLooping = true, bool bBackwardAnimate = false);
	void Pause();
	void Resume();
	void Reset();
	void SetLoop() { bLooping = false; }
	
	//초당 표현되는 프레임 수
	void SetFrameRate(int32 InFrameRate) { FrameRate = InFrameRate; }

	virtual void Tick(float deltaTime) override;
private:

	TSharedPtr<FSpriteAtlasAsset> Asset;
	bool bPlaying = false;
	bool bLooping = true;
	bool bBackward = false;
	int32 Frame = 0;
	int32 FrameRate = 36;
	float CurrentDeltaTime;
	float FrameAccumulator = 0.f;
};
