#pragma once

#include "SceneComponent.h"
#include "PrimitiveComponent.h"
#include "Assets.h"
#include "Camera.h"
#include "Actor.h"
#include "FAssetManager.h"
#include "ShowFlags.h"

class UPlaneComponent : public UPrimitiveComponent
{
	REFLECT_CLASS(UPlaneComponent, UPrimitiveComponent)

public:
	UPlaneComponent()
	{
		mePrimitive = EPrimitive::EP_Plane;
		mMeshAsset = FAssetManager::Get().GetAssetAs<FStaticMeshAsset>(FName("PlaneMesh"), true);
	}

	void Tick(float DeltaTime) override
	{
		// NOTE: SpotLightComponent의 위치와 회전을 부모 액터에 맞춘다. 현재 Hierarchy가 없으므로 부모 액터의 위치와 회전만 가져와서 적용한다.
		FTransform ParentTransform = mOwner->GetTransform();
		SetRelativeLocation(ParentTransform.Location);
		SetRelativeRotation(ParentTransform.Rotation);

		if (mBillboardCamera && mbBillboard)
		{
			FTransform PivotTransform = GetTransformMatrix();
			FRotator Rotation = FRotator::LookAt(PivotTransform.Location, PivotTransform.Location + mBillboardCamera->GetForwardVector());
			SetRelativeRotation(Rotation);
		}
	}

	void Render(FRenderCollector& RenderCollector) override
	{
		FTransform PivotTransform = GetTransformMatrix();

		FRenderQuadInfo QuadInfo;
		QuadInfo.Model = PivotTransform.MakeMatrix();
		QuadInfo.Color = FVector4(1.f, 1.f, 1.f, 1.f);
		QuadInfo.TextureSRV = mTextureAsset ? mTextureAsset->GetSRV() : nullptr;
		QuadInfo.EnableDepthTest = mEnableDepthTest;
		QuadInfo.EnableDepthWrite = mEnableDepthWrite;

		RenderCollector.AddQuadInfo(QuadInfo);
	}

	inline void SetBillboardCamera(FCamera& camera) { mBillboardCamera = &camera; }
	inline void SetBillboard(bool billboard) { mbBillboard = billboard; }
	inline void SetDepthState(bool enableDepthTest, bool enableDepthWrite) { mEnableDepthTest = enableDepthTest; mEnableDepthWrite = enableDepthWrite; }

private:
	FCamera* mBillboardCamera = nullptr;
	bool mbBillboard = false;
	bool mEnableDepthTest = true;
	bool mEnableDepthWrite = true;
};

class USpotLightComponent : public USceneComponent
{
	REFLECT_CLASS(USpotLightComponent, USceneComponent)

public:
	void Tick(float DeltaTime) override
	{
		// NOTE: SpotLightComponent의 위치와 회전을 부모 액터에 맞춘다. 현재 Hierarchy가 없으므로 부모 액터의 위치와 회전만 가져와서 적용한다.
		FTransform ParentTransform = mOwner->GetTransform();
		SetRelativeLocation(ParentTransform.Location);
		SetRelativeRotation(ParentTransform.Rotation);
		SetRelativeScale3D(ParentTransform.Scale);
	}

	inline float GetRange() const { return Range; }
	inline float GetInnerConeAngle() const { return mInnerConeAngle; }
	inline float GetOuterConeAngle() const { return mOuterConeAngle; }

private:
	float Range = 5.0f;
	float mInnerConeAngle = 30.0f;
	float mOuterConeAngle = 45.0f;
};

class ASpotLight : public AActor
{
	REFLECT_CLASS(ASpotLight, AActor)

public:
	ASpotLight()
	{
		USpotLightComponent* SpotLightComponent = FObjectFactory::ConstructObject<USpotLightComponent>(FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1));
		AddRootSceneComponent(SpotLightComponent);
	}

	void SerializeClass(json::JSON& outJson) const override
	{
	}

	void DeserializeClass(const json::JSON& inJson) override
	{
	}
};

class UText3DComponent : public USceneComponent
{
	REFLECT_CLASS(UText3DComponent, USceneComponent)

public:
	UText3DComponent() = default;

	void Tick(float DeltaTime) override
	{
		if (mBillboardCamera && mbBillboard)
		{
			// Match the camera's full orientation, including roll.
			SetRelativeRotation(mBillboardCamera->Transform.Rotation);
		}
		
		FVector Location = mOwner->GetRootComponent()->GetRelativeLocation();
		// Place billboard labels above the actor along the camera's screen-up axis.
		const FVector LabelUp = (mBillboardCamera && mbBillboard)
			? mBillboardCamera->GetUpVector()
			: FVector(0.f, 0.f, 1.f);
		Location += LabelUp * 1.0f;
		SetRelativeLocation(Location);
	}

	void Render(FRenderCollector& RenderCollector) override
	{
		// Show Flags에서 끄면 쿼드를 아예 만들지 않는다.
		// 만들고 거르는 게 아니라 글자 수만큼의 계산 자체가 사라진다.
		if (!FShowFlags::Get().IsEnabled(EShowFlag::UUIDText))
		{
			return;
		}

		if (!mFontAtlasAsset)
		{
			return;
		}

		const TSharedPtr<FFontAtlas>& fontAtlas = mFontAtlasAsset->GetFontAtlas();
		if (!fontAtlas)
		{
			return;
		}

		// Calculate the total size of the text in world units
		const float WorldLineHeight = fontAtlas->LineHeight() * WorldUnitPerPixel;
		const float WorldAscender = fontAtlas->Ascender() * WorldUnitPerPixel;
		const float WorldDescender = fontAtlas->Descender() * WorldUnitPerPixel;

		const FVector CameraForward = RenderCollector.Camera->GetForwardVector();
		const FVector CameraRight = RenderCollector.Camera->GetRightVector();
		const FVector CameraUp = RenderCollector.Camera->GetUpVector();

		float TotalWidth = 0.0f;
		float TotalHeight = 0.0f;
		uint32 LineCount = 1;

		float CurrentLineWidth = 0.0f;
		for (wchar_t C : mText)
		{
			if (C == L'\n')
			{
				TotalWidth = FPlatformMath::Max(TotalWidth, CurrentLineWidth);
				LineCount++;
				CurrentLineWidth = 0.0f;
				continue;
			}

			if (!fontAtlas->HasGlyph(C))
			{
				fontAtlas->AddGlyph(C);
			}

			const FFontGlyph& Glyph = fontAtlas->GetGlyph(C);

			float WorldAdvanceX = Glyph.AdvanceX * WorldUnitPerPixel;

			CurrentLineWidth += WorldAdvanceX;
		}
		TotalWidth = FPlatformMath::Max(TotalWidth, CurrentLineWidth);
		TotalHeight = (WorldAscender - WorldDescender) + (LineCount - 1) * WorldLineHeight;

		// Append the text quads to the output array
		FTransform PivotTransform = GetTransformMatrix();

		FVector TextLocation = FVector(0.f, -TotalWidth * 0.5f, TotalHeight * 0.5f - WorldAscender);
		for (wchar_t C : mText)
		{
			if (C == L'\n')
			{
				TextLocation.y = -TotalWidth * 0.5f;
				TextLocation.z -= WorldLineHeight;
				continue;
			}

			if (!fontAtlas->HasGlyph(C))
			{
				continue;
			}

			const FFontGlyph& Glyph = fontAtlas->GetGlyph(C);

			float WorldWidth = Glyph.Width * WorldUnitPerPixel;
			float WorldHeight = Glyph.Height * WorldUnitPerPixel;
			float WorldAdvance = Glyph.AdvanceX * WorldUnitPerPixel;
			float WorldBearingX = Glyph.BearingX * WorldUnitPerPixel;
			float WorldBearingY = Glyph.BearingY * WorldUnitPerPixel;

			FVector GlyphCenter(TextLocation.x, TextLocation.y + WorldBearingX + WorldWidth * 0.5f, TextLocation.z + WorldBearingY - WorldHeight * 0.5f);
			FMatrix TextModel = FMatrix::Scale(FVector3(1.0f, WorldWidth, WorldHeight)) * FMatrix::Translation(GlyphCenter);

			TextModel *= PivotTransform.MakeMatrix();

			FRenderQuadInfo QuadInfo;
			QuadInfo.Model = TextModel;
			QuadInfo.Color = mColor;
			QuadInfo.TextureSRV = mFontAtlasAsset->GetSRV();
			QuadInfo.SubUV = Glyph.SubUV;
			QuadInfo.EnableDepthTest = mEnableDepthTest;
			QuadInfo.EnableDepthWrite = mEnableDepthWrite;

			RenderCollector.AddQuadInfo(QuadInfo);

			TextLocation.y += WorldAdvance;
		}
	}

	inline void SetBillboardCamera(FCamera& camera) { mBillboardCamera = &camera; }
	inline void SetBillboard(bool billboard) { mbBillboard = billboard; }

	inline void SetText(const std::wstring& text) { mText = text; }
	inline const std::wstring& GetText() const { return mText; }

	inline void SetFontAtlasAsset(const TSharedPtr<FFontAtlasAsset>& fontAtlasAsset) { mFontAtlasAsset = fontAtlasAsset; }

	inline void SetColor(const FVector4& color) { mColor = color; }
	inline void SetDepthState(bool enableDepthTest, bool enableDepthWrite) { mEnableDepthTest = enableDepthTest; mEnableDepthWrite = enableDepthWrite; }

private:
	FCamera* mBillboardCamera = nullptr;
	bool mbBillboard = false;
	std::wstring mText;
	TSharedPtr<FFontAtlasAsset> mFontAtlasAsset;
	FVector4 mColor = FVector4(1, 1, 1, 1);
	bool mEnableDepthTest = true;
	bool mEnableDepthWrite = true;
};
