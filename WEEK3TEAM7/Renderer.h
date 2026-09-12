#pragma once

#include "Core.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "Matrix.h"
#include "Vector.h"
#include "RenderInfo.h"
#include "FRenderPipeline.h"

// 1. Define the triangle vertices
struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
	float u, v;       // Texture coordinates

	FVector GetPosition() const { return FVector(x, y, z); }
};

struct FConstants
{
	FMatrix Matrix;
	FVector4 Color;
	int32 UseVertexColor;
	int32 HasTexture;
	int32 Padding[2];
};

struct FLine2DConstants
{
	FMatrix Projection;
	FVector4 Color;
	FVector2 Start;
	FVector2 End;
	float Thickness;
	float Padding[3];
};

struct FCircle2DConstants
{
	FMatrix Projection;
	FVector4 Color;
	FVector2 Center;
	float Radius;
	float Padding[2];
};

struct FTriangle2DConstants
{
	FMatrix Projection;
	FVector4 Color;
	FVector2 Center;
	float Size;
	float Rotation;
};

struct FWorldAxisConstants
{
	FMatrix View;
	FMatrix Projection;
	FVector4 Color;
	FVector Axis;
	float Thickness;
};

struct FWorldGridConstants
{
	FMatrix ViewProjection;
};

struct FSamplerStateKey
{
	D3D11_FILTER Filter;
	D3D11_TEXTURE_ADDRESS_MODE AddressU;
	D3D11_TEXTURE_ADDRESS_MODE AddressV;

	bool operator==(const FSamplerStateKey& Other) const
	{
		return Filter == Other.Filter && AddressU == Other.AddressU && AddressV == Other.AddressV;
	}
};

struct FSamplerStateKeyHash
{
	std::size_t operator()(const FSamplerStateKey& Key) const
	{
		return std::hash<int>()(static_cast<int>(Key.Filter)) ^ (std::hash<int>()(static_cast<int>(Key.AddressU)) << 1) ^ (std::hash<int>()(static_cast<int>(Key.AddressV)) << 2);
	}
};

class FSamplerStatePool
{
public:
	ID3D11SamplerState* GetOrCreateSamplerState(ID3D11Device* Device, const FSamplerStateKey& Key)
	{
		ID3D11SamplerState** existing = SamplerStates.Find(Key);
		if (existing)
		{
			return *existing;
		}

		D3D11_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter = Key.Filter;
		SamplerDesc.AddressU = Key.AddressU;
		SamplerDesc.AddressV = Key.AddressV;
		SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.MipLODBias = 0.0f;
		SamplerDesc.MaxAnisotropy = 1;
		SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		SamplerDesc.BorderColor[0] = 0.0f;
		SamplerDesc.BorderColor[1] = 0.0f;
		SamplerDesc.BorderColor[2] = 0.0f;
		SamplerDesc.BorderColor[3] = 0.0f;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* SamplerState = nullptr;
		HRESULT Hr = Device->CreateSamplerState(&SamplerDesc, &SamplerState);
		if (FAILED(Hr))
		{
			return nullptr;
		}

		SamplerStates.Add(Key, SamplerState);

		return SamplerState;
	}

private:
	friend class URenderer;

	TMap<FSamplerStateKey, ID3D11SamplerState*, FSamplerStateKeyHash> SamplerStates;
};

class URenderer
{
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;

	FSamplerStatePool SamplerStatePool;
    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;

	ID3D11Texture2D* DepthStencilBuffer = nullptr;			// 실제 깊이값이 저장될 메모리
	ID3D11DepthStencilView* DepthStencilView = nullptr;		// 그 메모리를 "출력 대상"으로 보는 뷰

	TSharedPtr<FRenderPipeline> DefaultPipeline;
	TSharedPtr<FRenderPipeline> Line2DPipeline;
	TSharedPtr<FRenderPipeline> Circle2DPipeline;
	TSharedPtr<FRenderPipeline> Triangle2DPipeline;
	TSharedPtr<FRenderPipeline> WorldAxisPipeline;
	TSharedPtr<FRenderPipeline> WorldGridPipeline;

	UINT Width, Height;
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    D3D11_VIEWPORT ViewportInfo;
	FMatrix Projection2D;

	// 와이어프레임 여부. Prepare에서 갱신하고 BindPipeline이 읽는다.
	// RSSetState는 드로우 직전마다 덮어써지므로 플래그로 들고 있어야 한다.
	EViewModeIndex ViewModeIndex = EViewModeIndex::VMI_Lit;

#if 1
	ID3D11RasterizerState* RasterizerState[2] = {};
	ID3D11DepthStencilState* StencilMarkState = nullptr;	// 스텐실에 1 마킹용 상태
	ID3D11DepthStencilState* StencilOutlineState = nullptr; // 아웃라인 그리기용
	ID3D11BlendState* NoColorWriteBlendState = nullptr;		// 스텐실만 찍고 색은 쓰지 않는 상태

	// 매 프레임 내용이 바뀌는 선분용. 메시 버퍼와 달리 IMMUTABLE이 아니라 DYNAMIC이다
	ID3D11Buffer* LineVertexBuffer = nullptr;
	uint32 LineVertexCapacity = 0;
#endif

public:
	//create
	void Create(HWND hWindow);

	void CreateDeviceAndSwapChain(HWND hWindow);
	void ReleaseDeviceAndSwapChain();

	void CreateFrameBuffer();
	void ReleaseFrameBuffer();

	void CreateDepthStencilBuffer();

	void Release();

#if 0
	void CreateLineVertexBuffer(uint32 maxVertices);

	void CreateStencilMarkState();
	void CreateStencilOutlineState();
	void CreateNoColorWriteBlendState();

	//release
	void ReleaseLineVertexBuffer();
#endif

	template <typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateVertexBuffer(T* Vertices, UINT ByteWidth)
	{
		D3D11_BUFFER_DESC VertexBufferDesc = {};
		VertexBufferDesc.ByteWidth = ByteWidth;
		VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA VertexBufferSRD = { Vertices };

		Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
		Device->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, VertexBuffer.GetAddressOf());

		return VertexBuffer;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateTexture2D(const D3D11_TEXTURE2D_DESC& Desc, const void* InitialData = nullptr);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture, const D3D11_SHADER_RESOURCE_VIEW_DESC* Desc = nullptr);

	TSharedPtr<FRenderPipeline> CreateRenderPipeline();

	void BindPipeline(const TSharedPtr<FRenderPipeline>& Pipeline) const;

	//Update
	void RSUpdateState();

	//Rendering
	void Prepare(const FMatrix& ViewProjectionMatrix);
#if 0
	void RenderLines(const FVertexSimple* vertices, uint32 numVertices);
	void RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix Outline, const FRenderInfo& RI);
#endif

	void RenderPrimitive(const TSharedPtr<FRenderPipeline>& Pipeline, Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer, UINT NumVertices) const;
	void RenderPrimitive(Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer, UINT NumVertices, const FMatrix& Model) const;
	void RenderPrimitive(Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer, UINT NumVertices, const FMatrix& Model, const FVector4& Color) const;

	void RenderLine2D(const FVector2& Start, const FVector2& End, const FVector4& Color, float Thickness = 1.0f) const;
	void RenderCircle2D(const FVector2& Center, const FVector4& Color, float Radius = 1.0f) const;
	void RenderTriangle2D(const FVector2& Center, const FVector4& Color, float Size = 1.0f, float Rotation = 0.0f) const;
	void RenderWorldAxis(const FMatrix& View, const FMatrix& Projection, const FVector4& Color, const FVector& Axis, float Thickness = 1.0f) const;
	void RenderWorldGrid(const FMatrix& ViewProjection) const;

	void SwapBuffer();

	//Initialize
	void ClearDepth();

    //=============================================
	//해상도 변경 시 호출
	//void OnResize(UINT Width, UINT Height);
	void OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight);

	FORCEINLINE uint32 GetWidth() const { return Width; }
	FORCEINLINE uint32 GetHeight() const { return Height; }
};
