#include "Renderer.h"

void URenderer::Create(HWND hWindow)
{
#if 0
	CreateStencilMarkState();
	CreateStencilOutlineState();
	CreateNoColorWriteBlendState();
	CreateRasterizerState();
#else 
	CreateDeviceAndSwapChain(hWindow);
	CreateFrameBuffer();
	CreateDepthStencilBuffer();

	DefaultPipeline = MakeShared<FRenderPipeline>(Device, DeviceContext);
	DefaultPipeline->SetRasterRizerState(D3D11_CULL_BACK, 0, {EViewModeIndex::VMI_Lit, EViewModeIndex::VMI_Wireframe});
	DefaultPipeline->SetDepthStencilState(true, true);
	DefaultPipeline->SetShader("Assets/Shaders/Mesh.hlsl");
	DefaultPipeline->AddConstantBuffer<FConstants>();
	DefaultPipeline->AddConstantBuffer<FMatrix>();

	Line2DPipeline = MakeShared<FRenderPipeline>(Device, DeviceContext);
	Line2DPipeline->SetRasterRizerState(D3D11_CULL_NONE);
	Line2DPipeline->SetDepthStencilState(false, false);
	Line2DPipeline->SetShader("Assets/Shaders/Line2D.hlsl");
	Line2DPipeline->AddConstantBuffer<FLine2DConstants>();

	Circle2DPipeline = MakeShared<FRenderPipeline>(Device, DeviceContext);
	Circle2DPipeline->SetRasterRizerState(D3D11_CULL_NONE);
	Circle2DPipeline->SetDepthStencilState(false, false);
	Circle2DPipeline->SetShader("Assets/Shaders/Circle2D.hlsl");
	Circle2DPipeline->AddConstantBuffer<FCircle2DConstants>();

	Triangle2DPipeline = MakeShared<FRenderPipeline>(Device, DeviceContext);
	Triangle2DPipeline->SetRasterRizerState(D3D11_CULL_NONE);
	Triangle2DPipeline->SetDepthStencilState(false, false);
	Triangle2DPipeline->SetShader("Assets/Shaders/Triangle2D.hlsl");
	Triangle2DPipeline->AddConstantBuffer<FTriangle2DConstants>();

	WorldAxisPipeline = MakeShared<FRenderPipeline>(Device, DeviceContext);
	WorldAxisPipeline->SetRasterRizerState(D3D11_CULL_NONE);
	WorldAxisPipeline->SetDepthStencilState(true, true);
	WorldAxisPipeline->SetShader("Assets/Shaders/WorldAxis.hlsl");
	WorldAxisPipeline->AddConstantBuffer<FWorldAxisConstants>();

	WorldGridPipeline = MakeShared<FRenderPipeline>(Device, DeviceContext);
	WorldGridPipeline->SetRasterRizerState(D3D11_CULL_NONE);
	WorldGridPipeline->SetDepthStencilState(true, false);

	CD3D11_BLEND_DESC BlendDesc = {};
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	WorldGridPipeline->SetBlendState(BlendDesc);

	WorldGridPipeline->SetShader("Assets/Shaders/WorldGrid.hlsl");
	WorldGridPipeline->AddConstantBuffer<FWorldGridConstants>();
#endif
}

void URenderer::CreateDeviceAndSwapChain(HWND hWindow)
{
	D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	SwapChainDesc.BufferDesc.Width = 0;
	SwapChainDesc.BufferDesc.Height = 0;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.OutputWindow = hWindow;
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	UINT CreateDeviceFlags = 0;

#if defined(_DEBUG)
	CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | CreateDeviceFlags,
		FeatureLevels, ARRAYSIZE(FeatureLevels), D3D11_SDK_VERSION,
		&SwapChainDesc, &SwapChain, &Device, nullptr, &DeviceContext);

	SwapChain->GetDesc(&SwapChainDesc);
	Width = SwapChainDesc.BufferDesc.Width;
	Height = SwapChainDesc.BufferDesc.Height;
	ViewportInfo = { 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f };
	Projection2D = FMatrix::Ortho(0.f, Width, Height, 0.f, 0.0f, 1.0f);
}

void URenderer::ReleaseDeviceAndSwapChain()
{
	if (DeviceContext)
	{
		DeviceContext->Flush();
	}

	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}

	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}
}

void URenderer::CreateFrameBuffer()
{
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
	framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
}

void URenderer::ReleaseFrameBuffer()
{
	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}

	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}
}

void URenderer::ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
{
	vertexBuffer->Release();
}

#if 0
// 선분은 매 프레임 내용이 바뀌므로 IMMUTABLE로는 만들 수 없다.
// DYNAMIC + CPU_ACCESS_WRITE 라야 Map으로 덮어쓸 수 있다. (상수 버퍼와 같은 조합)
void URenderer::CreateLineVertexBuffer(uint32 maxVertices)
{
	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = maxVertices * sizeof(FVertexSimple);
	vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (SUCCEEDED(Device->CreateBuffer(&vertexbufferdesc, nullptr, &LineVertexBuffer)))
	{
		LineVertexCapacity = maxVertices;
	}
}

void URenderer::ReleaseLineVertexBuffer()
{
	if (LineVertexBuffer)
	{
		LineVertexBuffer->Release();
		LineVertexBuffer = nullptr;
	}

	LineVertexCapacity = 0;
}
#endif

void URenderer::Release()
{
	WorldGridPipeline.reset();
	WorldAxisPipeline.reset();
	Triangle2DPipeline.reset();
	Circle2DPipeline.reset();
	Line2DPipeline.reset();
	DefaultPipeline.reset();
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	DepthStencilView->Release();
	DepthStencilBuffer->Release();
	ReleaseFrameBuffer();
	ReleaseDeviceAndSwapChain();
}

void URenderer::SwapBuffer()
{
	SwapChain->Present(1, 0);
}

void URenderer::Prepare(const FMatrix& ViewProjectionMatrix)
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetViewports(1, &ViewportInfo);

	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	DefaultPipeline->UpdateConstantBuffer(1, ViewProjectionMatrix);
}

void URenderer::BindPipeline(const TSharedPtr<FRenderPipeline>& Pipeline) const
{
	// RSSetState는 드로우 직전마다 갈아치워지므로 뷰 모드 선택은 여기서 해야 한다.
	// 이 모드를 지원하지 않는 파이프라인(2D/기즈모)은 Lit 상태로 폴백된다.
	DeviceContext->RSSetState(Pipeline->GetRasterizerState(ViewModeIndex));
	DeviceContext->OMSetDepthStencilState(Pipeline->DepthStencilState, 0);
	DeviceContext->OMSetBlendState(Pipeline->BlendState, nullptr, 0xffffffff);
	DeviceContext->IASetInputLayout(Pipeline->InputLayout);
	DeviceContext->VSSetShader(Pipeline->VertexShader, nullptr, 0);
	DeviceContext->PSSetShader(Pipeline->PixelShader, nullptr, 0);
	DeviceContext->VSSetConstantBuffers(0, Pipeline->ConstantBuffers.Num(), &Pipeline->ConstantBuffers[0]);
	DeviceContext->PSSetConstantBuffers(0, Pipeline->ConstantBuffers.Num(), &Pipeline->ConstantBuffers[0]);
}

void URenderer::RSUpdateState()
{
	DeviceContext->RSSetState(RasterizerState[0]);
}

#if 0
// 쌓아둔 선분 전체를 한 번의 Draw로 그린다.
// 토폴로지를 바꾸므로 반드시 이 함수 안에서 되돌린다. 안 그러면 뒤에 그리는 것들이 전부 깨진다.
void URenderer::RenderLines(const FVertexSimple* vertices, uint32 numVertices)
{
	if (!LineVertexBuffer || vertices == nullptr || numVertices == 0) return;

	if (numVertices > LineVertexCapacity)
	{
		numVertices = LineVertexCapacity;   // 넘치면 자른다. 늘리려면 CreateLineVertexBuffer의 인자를 키운다
	}

	// WRITE_DISCARD: 이전 내용을 버리고 새 메모리를 받는다.
	// GPU가 지난 프레임 데이터를 아직 읽고 있어도 CPU가 기다리지 않는다.
	D3D11_MAPPED_SUBRESOURCE lineBufferMSR;
	if (FAILED(DeviceContext->Map(LineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lineBufferMSR)))
	{
		return;
	}
	memcpy(lineBufferMSR.pData, vertices, numVertices * sizeof(FVertexSimple));
	DeviceContext->Unmap(LineVertexBuffer, 0);

	// 직전에 메시 버퍼가 물려 있으므로 갈아끼워야 한다
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &LineVertexBuffer, &Stride, &offset);
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	DeviceContext->Draw(numVertices, 0);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void URenderer::RenderHighlight(ID3D11Buffer* pBuffer, uint32 Num, FMatrix mViewProjectionMatrix, FMatrix Outline, const FRenderInfo& RI)
{
	// (a) 스텐실에 1 마킹. 색은 쓰지 않으므로 화면 변화 없음.
	//     다른 오브젝트에 가려진 부분도 반드시 마킹해야 한다. 여기서 빠지면
	//     (b)의 != 1 조건을 통과해 버려서 겹친 영역 전체가 단색으로 칠해진다.
	DeviceContext->OMSetBlendState(NoColorWriteBlendState, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(StencilMarkState, 1);
	UpdateConstant(RI.WorldTransformMatrix, mViewProjectionMatrix);
	RenderPrimitive(pBuffer, Num);

	// (b) 확대판을 단색으로. 스텐실 != 1 인 곳만 통과 -> 테두리
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->OMSetDepthStencilState(StencilOutlineState, 1);
	UpdateConstant(Outline, mViewProjectionMatrix, FVector4(1.f, 0.6f, 0.f, 1.f));
	RenderPrimitive(pBuffer, Num);

	// (c) 원상복구
	DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
}
#endif

void URenderer::RenderPrimitive(const TSharedPtr<FRenderPipeline>& Pipeline, ID3D11Buffer* Buffer, UINT NumVertices) const
{
	BindPipeline(Pipeline);

	UINT Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &Buffer, &Pipeline->Stride, &Offset);
	DeviceContext->Draw(NumVertices, 0);
}

void URenderer::RenderPrimitive(ID3D11Buffer* Buffer, UINT NumVertices, const FMatrix& Model) const
{
	DefaultPipeline->UpdateConstantBuffer(0, FConstants{ Model, FVector4(1.0f, 1.0f, 1.0f, 1.0f), 1 });

	RenderPrimitive(DefaultPipeline, Buffer, NumVertices);
}

void URenderer::RenderPrimitive(ID3D11Buffer* Buffer, UINT NumVertices, const FMatrix& Model, const FVector4& Color) const
{
	DefaultPipeline->UpdateConstantBuffer(0, FConstants{ Model, Color, 0 });

	RenderPrimitive(DefaultPipeline, Buffer, NumVertices);
}

void URenderer::RenderLine2D(const FVector2& Start, const FVector2& End, const FVector4& Color, float Thickness) const
{
	Line2DPipeline->UpdateConstantBuffer(0, FLine2DConstants{ Projection2D, Color, Start, End, Thickness });

	BindPipeline(Line2DPipeline);

	UINT Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 0, NULL, NULL, &Offset);
	DeviceContext->Draw(6, 0);
}

void URenderer::RenderCircle2D(const FVector2& Center, const FVector4& Color, float Radius) const
{
	Circle2DPipeline->UpdateConstantBuffer(0, FCircle2DConstants{ Projection2D, Color, Center, Radius });

	BindPipeline(Circle2DPipeline);

	UINT Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 0, NULL, NULL, &Offset);
	DeviceContext->Draw(6, 0);
}

void URenderer::RenderTriangle2D(const FVector2& Center, const FVector4& Color, float Size, float Rotation) const
{
	Triangle2DPipeline->UpdateConstantBuffer(0, FTriangle2DConstants{ Projection2D, Color, Center, Size, Rotation - PI * 0.5f });

	BindPipeline(Triangle2DPipeline);

	UINT Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 0, NULL, NULL, &Offset);
	DeviceContext->Draw(3, 0);
}

void URenderer::RenderWorldAxis(const FMatrix& View, const FMatrix& Projection, const FVector4& Color, const FVector& Axis, float Thickness) const
{
	WorldAxisPipeline->UpdateConstantBuffer(0, FWorldAxisConstants{ View, Projection, Color, Axis, Thickness });

	BindPipeline(WorldAxisPipeline);

	UINT Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 0, NULL, NULL, &Offset);
	DeviceContext->Draw(6, 0);
}

void URenderer::RenderWorldGrid(const FMatrix& ViewProjection) const
{
	WorldGridPipeline->UpdateConstantBuffer(0, FWorldGridConstants{ ViewProjection });

	BindPipeline(WorldGridPipeline);

	UINT Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 0, NULL, NULL, &Offset);
	DeviceContext->Draw(6, 0);
}

//=============================================
void URenderer::CreateDepthStencilBuffer()
{
	D3D11_TEXTURE2D_DESC DepthTextureDesc = {};
	DepthTextureDesc.Width = Width;
	DepthTextureDesc.Height = Height;
	DepthTextureDesc.MipLevels = 1;
	DepthTextureDesc.ArraySize = 1;
	DepthTextureDesc.SampleDesc.Count = 1;
	DepthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Device->CreateTexture2D(&DepthTextureDesc, nullptr, &DepthStencilBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
	DsvDesc.Format = DepthTextureDesc.Format;
	DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	Device->CreateDepthStencilView(DepthStencilBuffer, &DsvDesc, &DepthStencilView);
}

#if 0
void URenderer::CreateStencilMarkState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	// 아웃라인 패스가 깊이를 무시하므로 마킹도 깊이를 무시해야 짝이 맞는다.
	// 가려진 픽셀까지 전부 마킹해야 실루엣 내부가 비지 않는다.
	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // 깊이는 건드리지 않는다
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	desc.StencilEnable = TRUE;							// 스텐실 사용
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0xFF;

	// 실루엣에 덮이는 모든 픽셀에 StencilRef를 기록
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace = desc.FrontFace;

	Device->CreateDepthStencilState(&desc, &StencilMarkState);
}

void URenderer::CreateStencilOutlineState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = FALSE;							// 항상 위에 그린다
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	desc.StencilEnable = TRUE;
	desc.StencilReadMask = 0xFF;
	desc.StencilWriteMask = 0x00;						// 읽기만, 쓰지 않는다

	// 마킹된 곳(=원본 실루엣)은 통과 못 함 -> 바깥 테두리만 남는다
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace = desc.FrontFace;

	Device->CreateDepthStencilState(&desc, &StencilOutlineState);
}

// 렌더타겟에 색을 전혀 쓰지 않는 상태. 스텐실 마킹 전용 패스에 쓴다
void URenderer::CreateNoColorWriteBlendState()
{
	D3D11_BLEND_DESC desc = {};
	desc.RenderTarget[0].BlendEnable = FALSE;
	desc.RenderTarget[0].RenderTargetWriteMask = 0;

	Device->CreateBlendState(&desc, &NoColorWriteBlendState);
}
#endif

void URenderer::OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight)
{
	if (!SwapChain || width == 0 || height == 0) return;
	if (ViewportInfo.Width == viewportWidth && ViewportInfo.Height == viewportHeight) return;

#if 0
	//해상도에 의존하는 프레임 버퍼와 뎁스 스텐실 버퍼를 재생성한다.
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	ReleaseFrameBuffer();
	ReleaseDepthStencilBuffer();

	HRESULT hr = SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr)) return;

	DXGI_SWAP_CHAIN_DESC desc;
	SwapChain->GetDesc(&desc);

	ViewportInfo = { viewportWidth, 0.0f, static_cast<float>(width) - viewportWidth, viewportHeight, 0.0f, 1.0f };

	//상태는 이전에 생성한 걸 그대로 재사용
	CreateFrameBuffer();
	CreateDepthStencilBuffer(width, height);
#else
	DeviceContext->OMSetRenderTargets(0, 0, 0);

	FrameBuffer->Release();
	FrameBufferRTV->Release();
	DepthStencilBuffer->Release();
	DepthStencilView->Release();

	SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

	Width = width;
	Height = height;
	ViewportInfo = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	Projection2D = FMatrix::Ortho(0.f, Width, Height, 0.f, 0.0f, 1.0f);

	CreateFrameBuffer();
	CreateDepthStencilBuffer();
#endif
}

void URenderer::ClearDepth()
{
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
