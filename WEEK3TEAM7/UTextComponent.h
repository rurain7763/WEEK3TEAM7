#pragma once

#include "SceneComponent.h"
#include "Assets.h"

class UTextComponent : public USceneComponent
{
	REFLECT_CLASS(UTextComponent, USceneComponent)

public:
	UTextComponent() = default;

	inline void GetRenderQuadInfos(TArray<FRenderQuadInfo>& OutRenderQuadInfos) const
	{
		if (!mFontAtlasAsset)
		{
			return;
		}

		const TSharedPtr<FFontAtlas>& fontAtlas = mFontAtlasAsset->GetFontAtlas();
		if (fontAtlas)
		{
			// Calculate the total size of the text in world units
			const float WorldLineHeight = fontAtlas->LineHeight() * WorldUnitPerPixel;
			const float WorldAscender = fontAtlas->Ascender() * WorldUnitPerPixel;
			const float WorldDescender = fontAtlas->Descender() * WorldUnitPerPixel;

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
			FMatrix PivotMatrix = GetTransformMatrix().MakeMatrix();
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

				FRenderQuadInfo QuadInfo;
				QuadInfo.Model = TextModel * PivotMatrix;
				QuadInfo.Color = mColor;
				QuadInfo.TextureSRV = mFontAtlasAsset->GetSRV();
				QuadInfo.SubUV = Glyph.SubUV;
				OutRenderQuadInfos.Add(QuadInfo);

				TextLocation.y += WorldAdvance;
			}
		}
	}

	inline void SetText(const std::wstring& text) { mText = text; }
	inline const std::wstring& GetText() const { return mText; }

	inline void SetFontAtlasAsset(const TSharedPtr<FFontAtlasAsset>& fontAtlasAsset) { mFontAtlasAsset = fontAtlasAsset; }

private:
	std::wstring mText;
	TSharedPtr<FFontAtlasAsset> mFontAtlasAsset;
	FVector4 mColor = FVector4(1, 1, 1, 1);
};
