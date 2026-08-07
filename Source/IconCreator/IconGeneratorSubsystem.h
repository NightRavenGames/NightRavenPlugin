// Fill out your copyright notice in the Description page of Project Settings.
/*
#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h" // per frame editor callback used to poll a light's transform while it's being dragged
#include "EditorSubsystem.h"
#include "EditorUndoClient.h" // Access to PostUndo / PostRedo so the preview can accept CTRL+Z / Y
#include "IconCreatorTypes.h"
#include "IconGeneratorSubsystem.generated.h"

class UStaticMesh;
class UTexture2D;
struct FIconCreatorRig; // The image "Studio"

UCLASS()
class ICONCREATOR_API UIconGeneratorSubsystem : public UEditorSubsystem, public FEditorUndoClient
{
	GENERATED_BODY()

public:
	// while ON moving or editing tagged stage actors in the level automaticlly re-captures the lights and re-renders the preview
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	bool bLiveLevelSync = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "Preview", meta = (DisplayName = "Preview Mesh"))
	TObjectPtr<UStaticMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Settings", meta = (ShowOnlyInnerProperties))
	FIconOutputSettings OutputSettings;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Settings", meta = (ShowOnlyInnerProperties))
	FIconGenSettings WidgetSettings;

	// Reads the current editor level and fills the WidgetSettings
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	int32 CaptureEffectsFromLevel();

	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	void LoadProjectDefaults();

	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	void SaveAsProjectDefaults();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets", meta = (DisplayName = "New Preset Name"))
	FString NewPresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets", meta = (DisplayName = "Preset", GetOptions = "GetPresetNames"))
	FString SelectedPreset;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Presets", meta = (DisplayName = "Save Preset"))
	void SavePreset();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Presets", meta = (DisplayName = "Load Preset"))
	void LoadPreset();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Presets", meta = (DisplayName = "Delete Preset"))
	void DeletePreset();

	UFUNCTION(BlueprintCallable, Category = "Presets")
	TArray<FString> GetPresetNames();

	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	UTexture2D* RefreshPreview();

	// keep just in case (for widget compatibility) but is identical to RefreshPreview()
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	UTexture2D* PreviewFirstSelected();

	// preview button function
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	UTexture2D* SetPreviewMesh(UStaticMesh* InMesh);

	// generate from selection function
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	TArray<UTexture2D*> GenerateFromSelection();

	// called repeatedly during a LMB drag
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	UTexture2D* OrbitPreview(float DeltaYaw, float DeltaPitch);

	// called repeatedly during a RMB drag
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	UTexture2D* RotateMeshPreview(float DeltaYaw, float DeltaPitch);
	
	// force the next preview to rebuild the scene from scratch
	UFUNCTION(BlueprintCallable, Category = "Icon Creator|Widget")
	void InvalidatePreviewRig();

	// Generate icons for a batch of meshes
	UFUNCTION(BlueprintCallable, Category = "Icon Creator")
	TArray<UTexture2D*> GenerateIcons(const TArray<UStaticMesh*>& Meshes, const FIconGenSettings& Settings);

	// preview the icon function
	UFUNCTION(BlueprintCallable, Category = "Icon Creator")
	UTexture2D* PreviewIcon(UStaticMesh* Mesh, const FIconGenSettings& InSettings);

private:
	// reused preview output texture updated in place for live dragging; keeping 1 x preview texture and overwrite its pixels each drag frame cheaply
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> PreviewTexture;

	// cached capture scene for fast repeated previews
	FIconCreatorRig* CachedRig = nullptr;
	FIconGenSettings CachedRigSettings;

	// live level sync hooks, just read level actors but never cache them
	FDelegateHandle ObjectPropertyChangedHandle;
	FDelegateHandle ActorMovedHandle;
	void HandleObjectPropertyChanged(UObject* Object, struct FPropertyChangedEvent& Event);
	void HandleActorMoved(AActor* Actor);
	bool bInAutoPreview = false;

	// true until RefreshPreview has done at least one CaptureEffectsFromLevel; without this tagged actors placed before the first preview are invisible
	bool bNeedsInitialCapture = true;

	// live drag sync: OnActorMoved only fires on release so update the icon while the mouse is held
	FDelegateHandle BeginMovementHandle;
	FDelegateHandle EndMovementHandle;
	FTSTicker::FDelegateHandle DragTickHandle;
	
	bool bPoseTransactionOpen = false;
	double LastPoseChangeTime = 0.0;
	FTSTicker::FDelegateHandle PoseTransactionTickHandle;
	static constexpr double PoseIdleSeconds = 0.35; // idle time which determines when "the dragging is finished"

	void EnsurePoseTransaction(const FText& Description);
	
	bool TickPoseTransaction(float DeltaTime);

	void ClosePoseTransaction();
	bool bDragActive = false;;
	void HandleBeginObjectMovement(UObject& Object);
	void HandleEndObjectMovement(UObject& Object);
	bool HandleDragTick(float DeltaTime);

	// fast path for the drag tick: update only the moved lights and decals with changed properties
	void RefreshStageTransforms();
	
	// deferred live sync
	bool bLiveSyncPending = false;
	FTSTicker::FDelegateHandle LiveSyncTickHandle;
	void RequestLiveSync();
	bool TickLiveSync(float DeltaTime);
	
	mutable bool bRenderInProgress = false;

	double LastLiveSyncTime = 0.0;
	static constexpr double LiveSyncMinInterval = 0.08; // approx. 12 HZ cap shared by all live-sync entry points

	//small shared helpers that de-duplicate the live-sync handlers/capture
	static const AActor* ResolveOwningActor(const UObject* Object);
	bool TryConsumeLiveSyncSlot();
	static bool FindStageSubjectFrame(class UWorld* World, FVector& OutOrigin, float& OutRadius);
	
	static bool IsStageActor(const AActor* Actor);

	//this guards against using a rig whose objects were garbage collected
	static bool RigIsValid(const FIconCreatorRig* Rig);

	static bool RigCompatible(const FIconGenSettings& A, const FIconGenSettings& B);

	//constructs the private scene, camera, lights, render target
	void BuildRig(FIconCreatorRig* Rig, const FIconGenSettings& Settings) const;

	// the icon render: mesh to pixels
	bool RenderIconPixels(FIconCreatorRig* Rig, UStaticMesh* Mesh, const FIconGenSettings& Settings, TArray<FColor>& OutPixels) const;
	
	// turn a finished pixel buffer into a saved UTexture2D asset on disk
	UTexture2D* WriteTextureAsset(
	const TArray<FColor>& Pixels,
	int32 Width,
	int32 Height,
	const FString& AssetBaseName,
	const FIconGenSettings& Settings) const;;
	
	//skip if unchanged hashing (opt-in by toggling on bSkipIfUnchanged)
	static FString MakeIconAssetName(const UStaticMesh* Mesh, const FIconGenSettings& Settings);

	static FString MakeIconObjectPath(const FString& AssetBaseName, const FIconGenSettings& Settings);

	static FString ExportSettingsText(const FIconGenSettings& Settings);
	static FString ComputeIconHash(const UStaticMesh* Mesh, const FString& SettingsText);

	static FString ReadIconSourceHash(const UTexture2D* Texture);
	static void WriteIconSourceHash(UTexture2D* Texture, const FString& Hash);
	
};
*/