#pragma once

#include "Core.h"
#include "FAsset.h"
#include "FFontAtlas.h"
#include "Vector.h"
#include "Matrix.h"
#include "FAABB.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <filesystem>
#include <ft2build.h>
#include FT_FREETYPE_H

class FFileManager;
class FFontManager;
class URenderer;

class FFileAssetSource : public FAssetSource
{
public:
	FFileAssetSource(FFileManager& InFileManager, const std::filesystem::path& InFilePath) : FileManager(InFileManager), FilePath(InFilePath) {}

	FString ReadFileToString() const;

private:
	FFileManager& FileManager;
	std::filesystem::path FilePath;
};

class FStaticMeshAsset : public FAsset
{
public:
	FStaticMeshAsset() = default;
	FStaticMeshAsset(const FName& InAssetName, URenderer& InRenderer, const FVertexSimple* InVertices, uint32 InVertexCount);
	FStaticMeshAsset(const FName& InAssetName, URenderer& InRenderer, const FVertexSimple* InVertices, uint32 InVertexCount, const uint32* InIndices, uint32 InIndexCount);

	inline Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const { return VertexBuffer; }
	inline uint32 GetVertexCount() const { return VertexCount; }
	inline Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const { return IndexBuffer; }
	inline uint32 GetIndexCount() const { return IndexCount; }
	inline const FAABB& GetLocalBoundingBox() const { return BoundingBox; }

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	uint32 VertexCount;

	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	uint32 IndexCount;

	FAABB BoundingBox;
};

class FTexture2DAsset : public FAsset
{
public:
	FTexture2DAsset() = default;
	FTexture2DAsset(const FName& InAssetName, Microsoft::WRL::ComPtr<ID3D11Texture2D> InTexture, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> InSRV)
		: FAsset(InAssetName, EAssetType::Texture2D)
		, Texture(InTexture)
		, SRV(InSRV)
	{
	}

	inline Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture() const { return Texture; }
	inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() const { return SRV; }

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
};

class FTexture2DAssetLoader : public FAssetLoader
{
public:
	FTexture2DAssetLoader(URenderer& InRenderer) : Renderer(InRenderer) {}
	~FTexture2DAssetLoader() = default;

	virtual TSharedPtr<FAsset> LoadAsset(const FName& AssetName, FAssetSource& AssetSource) override;
	virtual void UnloadAsset(TSharedPtr<FAsset> Asset) override;
	virtual EAssetType GetAssetType() const override { return EAssetType::Texture2D; }

private:
	URenderer& Renderer;
};

class FFontAsset : public FAsset
{
public:
	FFontAsset() = default;
	FFontAsset(const FName& InAssetName, FT_Face InFace, FString&& InFileContent)
		: FAsset(InAssetName, EAssetType::Font)
		, Face(InFace)
		, FileContent(std::move(InFileContent))
	{
	}

	~FFontAsset()
	{
		if (Face)
		{
			FT_Done_Face(Face);
		}
	}

	inline FT_Face GetFace() const { return Face; }

private:
	FString FileContent;
	FT_Face Face;
};

class FFontAssetLoader : public FAssetLoader
{
public:
	FFontAssetLoader(FFontManager& InFontManager) : FontManager(InFontManager) {}
	~FFontAssetLoader() = default;

	virtual TSharedPtr<FAsset> LoadAsset(const FName& AssetName, FAssetSource& AssetSource) override;
	virtual void UnloadAsset(TSharedPtr<FAsset> Asset) override;
	virtual EAssetType GetAssetType() const override { return EAssetType::Font; }

private:
	FFontManager& FontManager;
};

class FFontAtlasAsset : public FAsset, private FFontAtlasHandler
{
public:
	FFontAtlasAsset() = default;
	FFontAtlasAsset(const FName& InAssetName, URenderer& InRenderer, TSharedPtr<FFontAsset>& InFontAsset, uint32 InWidth, uint32 InHeight, uint32 InPaddingW, uint32 InPaddingH);

	inline TSharedPtr<FFontAtlas> GetFontAtlas() const { return FontAtlas; }
	inline Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture() const { return Texture; }
	inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() const { return SRV; }

private:
	bool HandleAddGlyph(FFontAtlas& FontAtlas, const FFontGlyph& InGlyph, const FFontGlyphBitmap& InBitmap) override;

private:
	URenderer& Renderer;

	TSharedPtr<FFontAsset> FontAsset;
	TSharedPtr<FFontAtlas> FontAtlas;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
};