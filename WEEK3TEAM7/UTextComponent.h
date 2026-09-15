#pragma once

#include "SceneComponent.h"
#include "PrimitiveComponent.h"
#include "Assets.h"
#include "Camera.h"
#include "Actor.h"
#include "FAssetManager.h"

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

		RenderCollector.QuadInfos.Add(QuadInfo);
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

class ASpotLight : public AActor
{
	REFLECT_CLASS(ASpotLight, AActor)

public:
	ASpotLight()
	{
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
			FTransform PivotTransform = GetTransformMatrix();
			FRotator Rotation = FRotator::LookAt(PivotTransform.Location, PivotTransform.Location + mBillboardCamera->GetForwardVector());
			SetRelativeRotation(Rotation);
		}
		
		// NOTE: 일단 TextComponent는 Owner의 위치를 따라간다. 추후에 Hierarchy를 구현하면, 부모부터 계산해서 내려온 WorldTransform을 기반으로 계산해야 한다.
		FVector Location = mOwner->GetRootComponent()->GetRelativeLocation();
		Location.z += 1.0f;
		SetRelativeLocation(Location);
	}

	void Render(FRenderCollector& RenderCollector) override
	{
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
			
#if 0
			if (mbBillboard)
			{
				FMatrix BillboardMatrix = FMatrix(
					FVector4(CameraForward.x, CameraForward.y, CameraForward.z, 0.f),
					FVector4(CameraRight.x, CameraRight.y, CameraRight.z, 0.f),
					FVector4(CameraUp.x, CameraUp.y, CameraUp.z, 0.f),
					FVector4(0.f, 0.f, 0.f, 1.f)
				);

				TextModel *= FMatrix::Scale(PivotTransform.Scale) * BillboardMatrix * FMatrix::Translation(PivotTransform.Location);
			}
			else
			{
			}
#endif

			TextModel *= PivotTransform.MakeMatrix();

			FRenderQuadInfo QuadInfo;
			QuadInfo.Model = TextModel;
			QuadInfo.Color = mColor;
			QuadInfo.TextureSRV = mFontAtlasAsset->GetSRV();
			QuadInfo.SubUV = Glyph.SubUV;
			QuadInfo.EnableDepthTest = mEnableDepthTest;
			QuadInfo.EnableDepthWrite = mEnableDepthWrite;

			RenderCollector.QuadInfos.Add(QuadInfo);

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
