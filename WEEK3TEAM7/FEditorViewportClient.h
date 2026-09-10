#pragma once
#include "Vector.h"

#include <d3d11.h>
#include "World.h"
#include "Camera.h"
#include "RenderInfo.h"
#include "Gizmo.h"

class AActor;
class FSceneManager;

struct FEditorViewportClient
{
public:
	void RayCast(D3D11_VIEWPORT ViewportInfo, UWorld* World, float perspectiveRatio);
	float GetFov() const { return mCamera.mFovDegree; }
	void Update(float deltaTime, D3D11_VIEWPORT ViewportInfo, FSceneManager* sceneManager, float perspectiveRatio);
	bool IsMouseHit() const { return bMouseHit; }

	void Reset();

	FCamera& GetCamera() { return mCamera; }

	FCamera mCamera;
	FGizmo mGizmo;

private:
	//마우스 밑 무언가의
	FRenderInfo mHoveredRenderInfo;

	// 선택된 액터의 RenderInfo는 캐시하지 않는다. 필요할 때 ClickedActor->GetRenderInfos()로 그때그때 뽑는다.
	//마우스 밑 무언가가 Actor이면 저장. RayCast 에서 채워야 함 (아직 미구현)
	// INFO: mClickedActor moved to FSceneManager::mSelectedActor.
	//AActor* mClickedActor = nullptr;


	bool RayIntersectsTriangle( // 두개의 
		const FVector& Origin,
		const FVector& Dir,
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		float& OutT, float& OutU, float& OutV);

	void DeprojectScreenToWorld(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		FVector& OutNearPoint, FVector& OutFarPoint);

	void DeprojectScreenToWorldForOrtho(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		FVector& OutNearPoint, FVector& OutFarPoint);

	void DeprojectScreenToWorldForUnified(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		float orthoDistance, float perspectiveRatio,
		FVector& OutNearPoint, FVector& OutFarPoint
	);



	bool bMouseHit = false;
	
	// RayCast가 이번 프레임에 쏜 광선. 기즈모 드래그가 같은 광선을 다시 쓴다
	FVector mRayNear;
	FVector mRayFar;
};
