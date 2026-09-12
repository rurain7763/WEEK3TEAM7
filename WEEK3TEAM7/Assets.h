#pragma once

#include "Core.h"
#include "FAsset.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <filesystem>

class FFileManager;
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
	FStaticMeshAsset(const FName& InAssetName, Microsoft::WRL::ComPtr<ID3D11Buffer> InVertexBuffer, uint32 InVertexCount)
		: FAsset(InAssetName, EAssetType::StaticMesh)
		, VertexBuffer(InVertexBuffer)
		, VertexCount(InVertexCount)
	{
	}

	inline Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const { return VertexBuffer; }
	inline uint32 GetVertexCount() const { return VertexCount; }

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	uint32 VertexCount;
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
	virtual ~FTexture2DAssetLoader() = default;

	virtual TSharedPtr<FAsset> LoadAsset(const FName& AssetName, FAssetSource& AssetSource) override;
	virtual void UnloadAsset(TSharedPtr<FAsset> Asset) override;
	virtual EAssetType GetAssetType() const override { return EAssetType::Texture2D; }

private:
	URenderer& Renderer;
};