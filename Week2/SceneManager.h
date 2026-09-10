#pragma once

#include <string_view>

#include "SceneData.h"
#include "TArray.h"
#include "RenderInfo.h"
#include "enum.h"

inline constexpr std::string_view kSceneDataDir = "SceneData\\";
inline constexpr std::string_view kSceneDataSuffix = ".Scene";

class FFileManager;
class FFrameTimer;
class FEditorViewportClient;
class FGraphicsManager;
class UWorld;

struct FGuiReference
{
	const FFrameTimer& FrameTimer;
	FGraphicsManager* GraphicsManager;
	FEditorViewportClient* ViewportClient;
	const FFileManager* FileManager;
};

struct FGuiInputField
{
	/* Spawn Actor */
	EPrimitive PrimitiveType = EPrimitive::EP_Cube;
	int32 SpawnCount = 1;

	/* Scene Control */
	char SceneName[512] = "Default";

	/* Object Lists */
	TArray<UObject*> SortedObjectLists;
	uint64 LastGUObjectRevision = -1;
};

class FSceneManager
{
public:
	FSceneManager();
	~FSceneManager();

	void Update(float delaTime);
	void UpdateGUI(const FGuiReference& guiReference);

	const TArray<FRenderInfo> GetRenderInfos() const;
	const TArray<FRenderInfo> GetAxisRenderInfos();

	// Clear world
	void NewScene();
	void DeleteScene();

	void SaveScene(std::string_view sceneName, const FFileManager& fileManager);
	void LoadScene(std::string_view sceneName, const FFileManager& fileManager);

	UWorld* GetCurrentWorld() const { return mCurrentWorld; }

	AActor* GetSelectedActor() const { return mSelectedActor; }
	bool IsActorSelected() const { return mSelectedActor != nullptr; }
	void SetSelectedActor(AActor* actor);
	void ResetSelectedActor() { mSelectedActor = nullptr; }

	float GetPanelWidth() const;

private:
	static constexpr float MIN_WIDTH_RATIO = 0.2f;
	static constexpr float MAX_WIDTH_RATIO = 0.6f;

	static constexpr float CONTROL_PANEL_HEIGHT_RATIO = 0.4f;
	static constexpr float WINDOW_PROPERTY_HEIGHT_RATIO = 0.3f;

	float mPanelWidth;

	UWorld* mCurrentWorld = nullptr;
	AActor* mSelectedActor = nullptr;
	FGuiInputField mGuiInputField;

	void updateControlPanelGUI(const FGuiReference& guiReference);
	void updatePropertyWindowGUI(const FGuiReference& guiReference);
	void updateObjectListPanelGUI(const FGuiReference& guiReference);
};
