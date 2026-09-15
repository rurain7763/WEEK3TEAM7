#pragma once

#include "SceneComponent.h"
#include "Assets.h"

class UPrimitiveComponent : public USceneComponent
{
	REFLECT_CLASS(UPrimitiveComponent, USceneComponent)

public:
	UPrimitiveComponent();

	//void Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive);
	//void Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);

	using USceneComponent::Initialize;
	void Initialize(EPrimitive ePrimitive);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);

	virtual ~UPrimitiveComponent();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	//virtual void Render();
	virtual void Render(FRenderCollector& RenderCollector) override;
	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override final;

	inline const TSharedPtr<FStaticMeshAsset>& GetMesh() const { return mMeshAsset; }

	inline void SetTexture(const TSharedPtr<FTexture2DAsset>& textureAsset) { mTextureAsset = textureAsset; }
	inline const TSharedPtr<FTexture2DAsset>& GetTexture() const { return mTextureAsset; }

	inline EPrimitive GetPrimitiveType() const { return mePrimitive; }

protected:
	//GraphicsManager* mGraphicsManager;
	EPrimitive mePrimitive;
	TSharedPtr<FStaticMeshAsset> mMeshAsset;
	TSharedPtr<FTexture2DAsset> mTextureAsset;
};



