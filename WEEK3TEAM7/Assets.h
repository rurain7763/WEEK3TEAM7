#pragma once

#include "FAsset.h"
#include <d3d11.h>
#include <wrl/client.h>

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