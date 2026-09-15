#include "LaunchEngineLoop.h"

#include <windows.h>
#include "Renderer.h"
#include "WindowApplication.h"
#include "Console.h"
#include "GraphicsManager.h"
#include "CubeComponent.h"
#include "ObjectFactory.h"
#include "Cube.h"
#include "Sphere.h"
#include "Circle.h"
#include "Triangle.h"
#include "Plane.h"
#include "Object.h"
#include "GizmoArrow.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"
#include "Actor.h"
#include "World.h"
#include <FLogManager.h>
#include "Assets.h"

void FEngineLoop::Init(HINSTANCE hInstance, WNDPROC WndProc)
{
	// Initialize window infos
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(
		0,
		WindowClass,
		Title,
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 1024,
		nullptr, nullptr, hInstance, nullptr
	);

	// 창을 화면 크기에 맞게 최대화하여 표시
	ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	// 최대화된 후의 실제 클라이언트 크기를 구해 콘솔에 전달
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;		// Generic Desktop
	rid.usUsage = 0x02;			// Mouse
	rid.dwFlags = 0;		// 포커스 있을 때만 수신
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	mGraphicsManager = new FGraphicsManager(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(mGraphicsManager->GetRenderer()->Device, mGraphicsManager->GetRenderer()->DeviceContext);
	auto& IO = ImGui::GetIO();
	IO.Fonts->AddFontFromFileTTF(
		"C:/Windows/Fonts/malgun.ttf",
		18.0f,
		nullptr,
		IO.Fonts->GetGlyphRangesKorean()
	);

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::Get();
	console.Init(clientWidth);

	FrameTimer = new FFrameTimer(120);
	ViewportClient = new FEditorViewportClient(*mGraphicsManager->GetRenderer()); // Todo: cChange to class

	const FVector4 NearTint(1.0f, 0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint(0.25f, 0.55f, 1.0f, 0.85f); // 파랑 = 먼 쪽

	mSceneManager = new FSceneManager();
	mFileManager = new FFileManager();
	mFontManager = new FFontManager();
	InitAssetManager();

	mSceneManager->NewScene();

	//test code
	//{
	//	UCubeComponent* cubeComonent = FObjectFactory::ConstructObject<UCubeComponent>(FVector(0), FRotator(), FVector(1));
	//	AActor* cubeActor = FObjectFactory::ConstructObject<AActor>();
	//	cubeActor->AddComponent(cubeComonent);
	//	mSceneManager.GetCurrentWorld()->AddActor(cubeActor);
	//}
}

void FEngineLoop::InitAssetManager()
{
	mAssetManager = new FAssetManager();

	URenderer* renderer = mGraphicsManager->GetRenderer();
	
	// Register built-in asset types
	TSharedPtr<FStaticMeshAsset> cubeAsset = MakeShared<FStaticMeshAsset>(FName("CubeMesh"), *renderer, Cube_vertices, sizeof(Cube_vertices) / sizeof(FVertexSimple));
	mAssetManager->RegisterAsset(cubeAsset);

	TSharedPtr<FStaticMeshAsset> sphereAsset = MakeShared<FStaticMeshAsset>(FName("SphereMesh"), *renderer, Sphere_vertices, sizeof(Sphere_vertices) / sizeof(FVertexSimple));
	mAssetManager->RegisterAsset(sphereAsset);

	TSharedPtr<FStaticMeshAsset> circleAsset = MakeShared<FStaticMeshAsset>(FName("CircleMesh"), *renderer, Circle_vertices, sizeof(Circle_vertices) / sizeof(FVertexSimple));
	mAssetManager->RegisterAsset(circleAsset);

	TSharedPtr<FStaticMeshAsset> triangleAsset = MakeShared<FStaticMeshAsset>(FName("TriangleMesh"), *renderer, Triangle_vertices, sizeof(Triangle_vertices) / sizeof(FVertexSimple));
	mAssetManager->RegisterAsset(triangleAsset);

	TSharedPtr<FStaticMeshAsset> gizmoArrowAsset = MakeShared<FStaticMeshAsset>(FName("GizmoArrowMesh"), *renderer, GizmoArrow_vertices, sizeof(GizmoArrow_vertices) / sizeof(FVertexSimple));
	mAssetManager->RegisterAsset(gizmoArrowAsset);

	TSharedPtr<FStaticMeshAsset> PlaneAsset = MakeShared<FStaticMeshAsset>(FName("PlaneMesh"), *renderer, Plane_vertices, sizeof(Plane_vertices) / sizeof(FVertexSimple));
	mAssetManager->RegisterAsset(PlaneAsset);

	TSharedPtr<FTexture2DAssetLoader> TextureLoader = MakeShared<FTexture2DAssetLoader>(*renderer);
	TSharedPtr<FFontAssetLoader> FontLoader = MakeShared<FFontAssetLoader>(*mFontManager);

	TSharedPtr<FFileAssetSource> FileAssetSource = MakeShared<FFileAssetSource>(*mFileManager, "Textures/Test.jpg");
	mAssetManager->RegisterAsset(FName("TestTexture"), TextureLoader, FileAssetSource);

	TSharedPtr<FFileAssetSource> SpotLightIconAssetSource = MakeShared<FFileAssetSource>(*mFileManager, "Textures/Icon_SpotLight.png");
	mAssetManager->RegisterAsset(FName("SpotLightIcon"), TextureLoader, SpotLightIconAssetSource);

	TSharedPtr<FFileAssetSource> FontAssetSource = MakeShared<FFileAssetSource>(*mFileManager, "Fonts/BMKkubulimTTF.ttf");
	mAssetManager->RegisterAsset(FName("TestFont"), FontLoader, FontAssetSource);
	
	TSharedPtr<FFontAsset> TestFontAsset = mAssetManager->GetAssetAs<FFontAsset>(FName("TestFont"), true);
	TSharedPtr<FFontAtlasAsset> FontAtlasAsset = MakeShared<FFontAtlasAsset>(FName("TestFontAtlas"), *renderer, TestFontAsset, 512, 512, 2, 2);
	mAssetManager->RegisterAsset(FontAtlasAsset);
}

void FEngineLoop::Tick(bool bPumpMessages)
{
	if (GInTick) return;
	GInTick = true;

	FrameTimer->StartFrame();
	float deltaTime = FrameTimer->GetDeltaTime();
	ConsoleWindow& console = ConsoleWindow::Get();

	FRenderCollector& RenderCollector = mGraphicsManager->GetRenderCollector();
	RenderCollector.Camera = &ViewportClient->GetCamera();

	//Input Threads
	{
		WindowApplication.ProcessDeferredEvents();

		//ImGui Input
		{
			mSceneManager->UpdateGUI({ *FrameTimer, mGraphicsManager, ViewportClient, mFileManager, mAssetManager });
		}

		mGraphicsManager->UpdateProjectionTransition(deltaTime);
		ViewportClient->Update(deltaTime, mGraphicsManager->GetRenderer()->ViewportInfo, mSceneManager, mGraphicsManager->GetPerspectiveRatio(), RenderCollector);
	}

	//Physics Threads
	{

	}

	//Game Threads
	{
		// 레이캐스트보다 먼저 돌려야 한다.
		// 여기서 RenderInfos 가 갱신되고, RayCast 가 그걸 읽는다.
		mSceneManager->Update(deltaTime, RenderCollector);
	}

	//Render Threads
	{
		if (WindowApplication.bPendingResize)
		{
			float viewportWidth = mSceneManager->GetPanelWidth();
			float viewportHeight = (1.f - ConsoleWindow::HEIGHT_RATIO) * WindowApplication.PendingHeight;

			mGraphicsManager->GetRenderer()->OnResize(WindowApplication.PendingWidth, WindowApplication.PendingHeight, viewportWidth, viewportHeight);
			WindowApplication.bPendingResize = false;
		}

		mGraphicsManager->Update(deltaTime);
		mGraphicsManager->Prepare(&ViewportClient->mCamera);

		mGraphicsManager->DrawWorldAxis();
		mGraphicsManager->FlushLines();

		mGraphicsManager->Render();
		
		//월드 축. 액터 뒤에 그려서 같은 깊이 버퍼로 가려지게 한다 (기즈모와 달리 깊이를 지우지 않는다)

		//강조
		if (mSceneManager->GetSelectedActor())
		{
			FRenderInfo clickedRenderInfo;
			mSceneManager->GetSelectedActor()->GetFirstRenderInfo(clickedRenderInfo);
			mGraphicsManager->RenderHighLight(clickedRenderInfo);
		}

		ViewportClient->mGizmo.Draw(mSceneManager->GetSelectedActor(), ViewportClient->mCamera.Transform.Location, mGraphicsManager->GetViewProjectionMatrix());

		//ImGui
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		mGraphicsManager->Display();
	}

	FrameTimer->EndFrame();

	GInTick = false;
}

void FEngineLoop::End()
{
	const std::string Value = std::format("{:.6f}", ViewportClient->GetCamera().Sensitivity);

	if (!WritePrivateProfileStringA("Camera", "Sensitivity", Value.c_str(), ".\\editor.ini"))
	{
		UE_LOG_ERROR("Failed to save camera sensitivity to editor.ini");
	}
	mSceneManager->DeleteScene();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete ViewportClient;
	delete FrameTimer;
	delete mSceneManager;
	delete mFileManager;
	delete mAssetManager;
	delete mFontManager;

	delete mGraphicsManager;
}
