//TODO:Had to many typos and wrong types not properly linked up and all functions aren't properly binded. - https://www.youtube.com/watch?v=7Nqpb6mj84k
/*
#include "IconGeneratorSubsystem.h"
#include "Async/ParallelFor.h" // multithreads the per-pixel compositing loop
#include "AdvancedPreviewScene.h" // the self-contained mini-world the icon renders into
#include "AssetRegistry/AssetRegistryModule.h" // to notify the editor when a new asset was created
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ContentStreaming.h" //used to fully stream textures in before capture (determinism)
#include "Components/DecalComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LocalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/ReflectionCaptureComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Editor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EditorUtilityLibrary.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Engine/TextureRenderTarget2D.h" // the GPU texture the camera renders into before we read it back
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h" // for making the PNG export directory
#include "ImageCore.h"		// CPU image containers used for resizing + PNG
#include "ImageUtils.h"		// FImageUtils::SaveImageByExtension writes the PNG
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"	// turns the combined input fingerprint into a short hex hash
#include "Misc/ScopeLock.h"		// the "Generating icons..." progress dialog
#include "PackageTools.h"		// UPackageTools::SanitizePackageName cleans user-typed folder names
#include "RenderingThread.h"	// wait for the GPU so read-backs are valid
#include "ShaderCompiler.h" 	// force synchronous shader compilation for new materials
#include "TextureResource.h"	// the render-thread side of the render target
#include "UObject/Package.h" 	// the container a saved asset lives in

#define LOCTEXT_NAMESPACE "IconCreator"

static const FName IconSubjectTag(TEXT("IconSubject")); // tag to use on the stand-in mesh actor
static const FName IconEffectsTag(TEXT("IconEffects")); // tag to use effects (lights, post-process, decals, props etc.)

// the capture studio built once per batch/preview session
struct FIconCreatorRig
{
	// used a TUniquePtr to own it directly and it auto-destructs when the rig is destroyed
	TUniquePtr<FAdvancedPreviewScene> Scene;
	
	UStaticMeshComponent* MeshComp = nullptr;
	USceneCaptureComponent2D* Capture = nullptr;
	UTextureRenderTarget2D* RenderTarget = nullptr; // the GPU surface the camera draws to

	//background mode
	TArray<FColor> BackgroundPixels;		// preresized backdrop pixels computed once so per-frame compositing is cheap

	// stage light components
	TArray<TMap<FString, FProperty*>> StageLightPropLookup;

	//decals
	TArray<UDecalComponent*> DecalComps;

	// Props
	TArray<UStaticMeshComponent*> PropComps;

	//without this the first render would be lit slightly differently from the later ones, the first icon in a batch wouldn't match the rest
	// cleared after the warm-up in RenderIconPixels
	bool bNeedsWarmUp = true;
	
};


void UIconGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//undo support
	SetFlags(RF_Transactional);

	// load the saved project defaults
	WidgetSettings = GetDefault<UIconCreatorSettings>()->DefaultSettings;
	OutputSettings = WidgetSettings.Output;

	// subscribe to the global "Some object's property changed" event so live level sync can react to edits
	// AddUObject binds safely to a UObject method
	ObjectPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(
	this, &UIconGeneratorSubsystem::HandleObjectPropertyChanged);

	// subscribe to "an actor moved" too, but only if the editor exists
	if(GEditor)
	{
		GEditor->RegisterForUndo(this);
		
		ActorMovedHandle = GEditor->OnActorMoved().AddUObject(
		this, &UIconGeneratorSubsystem::HandleActorMoved);

		// viewport gizmo drags so update the icon whilst dragging; OnActorMoved above only fires when the mouse is released
		BeginMovementHandle = GEditor->OnBeginObjectMovement().AddUObject(this, &UIconGeneratorSubsystem::HandleBeginObjectMovement);
		EndMovementHandle = GEditor->OnEndObjectMovement().AddUObject(this, &UIconGeneratorSubsystem::HandleEndObjectMovement);
	}
}

void UIconGeneratorSubsystem::Deinitialize()
{
	if(ObjectPropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(ObjectPropertyChangedHandle);
	}

	if (GEditor && ActorMovedHandle.IsValid())
	{
		GEditor->OnActorMoved().Remove(ActorMovedHandle);
	}

	if (LiveSyncTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(LiveSyncTickHandle);
		LiveSyncTickHandle.Reset();
	}

	ClosePoseTransaction();
	if (PoseTransactionTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PoseTransactionTickHandle);
		PoseTransactionTickHandle.Reset();
	}

	// stop recieving undo notifications
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}

	//drag events
	if (GEditor && BeginMovementHandle.IsValid())
	{
		GEditor->OnBeginObjectMovement().Remove(BeginMovementHandle);
	}
	
	if (GEditor && EndMovementHandle.IsValid())
	{
		GEditor->OnEndObjectMovement().Remove(EndMovementHandle);
	}
	
	// if the editor closes mid-drag, make sure the per-frame ticker is gone
	if (DragTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DragTickHandle);
		DragTickHandle.Reset();
	}

	InvalidatePreviewRig(); // free the cached capture scene (manual raw-pointer cleanup)
	Super::Deinitialize();
}

void UIconGeneratorSubsystem::PostUndo(bool bSuccess)
{
	ClosePoseTransaction();
	if (bSuccess && PreviewTexture)
	{
		RefreshPreview();
	}
}

void UIconGeneratorSubsystem::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

bool UIconGeneratorSubsystem::FindStageSubjectFrame(UWorld* World, FVector& OutOrigin, float& OutRadius)
{

}

int32 UIconGeneratorSubsystem::CaptureEffectsFromLevel()
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Capture From Level: no editor world"));
		return 0;
	}

	FVector Origin;
	float SubjectRadius = 50.f;
	if (!FindStageSubjectFrame(World, Origin, SubjectRadius))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Capture From Level: no actor tagged '%s'. Tag your posed mesh actor (Details > Actor > Tags)"), *IconSubjectTag.ToString());
		return 0;
	}

	WidgetSettings.Effects.Lights.Reset();
	
	if (!bInAutoPreview)
	{
		WidgetSettings.Effects.LightReferenceRadius = SubjectRadius;
	}

	WidgetSettings.Effects.bHasPostProcess = false; // clear any previously captured post-process
	WidgetSettings.Effects.Decals.Reset(); // clear any previously captured decals

	// preserve user-set bIgnoreEffects flags before rebuilding the prop list
	// match by mesh pointer so the flag survives a recapture
	TMap<UStaticMesh*, bool> SavedIgnoreFlags;
	for (const FIconStudioProp& OldProp : WidgetSettings.Effects.Props)
	{
		if (OldProp.Mesh && OldProp.bIgnoreEffects)
		{
			SavedIgnoreFlags.Add(OldProp.Mesh, true);
		}
	}
	WidgetSettings.Effects.Props.Reset(); // clear any previously captured props

	// second pass over all actors: collect effects whose actor or component has the IconEffects tag
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		const bool bActorTagged = Actor->ActorHasTag(IconEffectsTag);

		TInlineComponentArray<ULightComponent*> LightComps(Actor);
		for (ULightComponent* Comp : LightComps)
		{
			if (!IsValid(Comp))
			{
				continue;
			}
			if (!bActorTagged && !Comp->ComponentHasTag(IconEffectsTag))
			{
				continue;
			}

			//build the plain-data record for this light aka copy everything from it
			FIconStudioLight Data;
			Data.Color = Comp->GetLightColor();
			Data.Intensity = Comp->Intensity;
			Data.bCastShadows = Comp->CastShadows != 0;
			Data.Direction = Comp->GetComponentRotation();
			Data.RelativePosition = (Comp->GetComponentLocation() - Origin) / SubjectRadius;
			Data.ComponentClass = Comp->GetClass();

			// full property snapshot
			{
				static const TSet<FName> SkipProps = {
				TEXT("RelativeLocation"), TEXT("RelativeRotation"), TEXT("RelativeScale3D"),
					TEXT("Mobility"), TEXT("AttachParent"), TEXT("AttachSocketName"),
					TEXT("Intensity"), TEXT("AttenuationRadius")
				};
				
				//instance that it's compared against, so only store what was actually changed
				const UObject* CDO = Comp->GetClass()->GetDefaultObject();

				// walk every reflected property on the light's class
				for (TFieldIterator<FProperty> PropIt(Comp->GetClass()); PropIt; ++PropIt)
				{
					FProperty* Prop = *PropIt;
					
					// skip a property if any of these are true
					if (!Prop->HasAnyPropertyFlags(CPF_Edit) ||
						Prop->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_EditConst) ||
						SkipProps.Contains(Prop->GetFName()) ||
						Prop->Identical_InContainer(Comp,CDO))
					{
						continue;
					}

					// export the property's value to a text string and stash it under its name, ImportText later replays it verbatim
					FString Value;
					Prop->ExportText_InContainer(0, Value,  Comp, Comp, nullptr, PPF_None);
					Data.PropertyValues.Add(Prop->GetName(), Value);
				}
			}

			//fill in type specific fields depending on the concrete light class
			if (USpotLightComponent* Spot = Cast<USpotLightComponent>(Comp))
			{
				Data.Type = EIconStudioLightType::Spot;
				Data.AttenuationRadiusScale = Spot->AttenuationRadius / SubjectRadius;
				Data.bInverseSquaredFalloff = Spot->bUseInverseSquaredFalloff != 0;
				Data.InnerConeAngle = Spot->InnerConeAngle;
				Data.OuterConeAngle = Spot->OuterConeAngle;
			}
			else if (UPointLightComponent* Point = Cast<UPointLightComponent>(Comp))
			{
				Data.Type = EIconStudioLightType::Point;
				Data.AttenuationRadiusScale = Point->AttenuationRadius / SubjectRadius;
				Data.bInverseSquaredFalloff = Point->bUseInverseSquaredFalloff != 0;
			}

			else if (URectLightComponent* RectComp = Cast<URectLightComponent>(Comp))
			{
				Data.Type = EIconStudioLightType::Rect;
				Data.AttenuationRadiusScale = RectComp->AttenuationRadius / SubjectRadius;
				Data.bInverseSquaredFalloff = true;;
			}
			
			else if (Cast<UDirectionalLightComponent>(Comp))
			{
				Data.Type = EIconStudioLightType::Directional;
			}
			else
			{
				continue;
			}
			
			// add the light to the list
			WidgetSettings.Effects.Lights.Add(Data);
		}

		// post process on the actor
		if (!WidgetSettings.Effects.bHasPostProcess)
		{
			if (APostProcessVolume* PPV = Cast<APostProcessVolume>(Actor))
			{
				if (bActorTagged)
				{
					WidgetSettings.Effects.PostProcess = PPV->Settings;
					WidgetSettings.Effects.bHasPostProcess = true;
				}
			}
			else
			{
				TInlineComponentArray<UPostProcessComponent*> PPComps(Actor);
				for (UPostProcessComponent* PP : PPComps)
				{
					if (!IsValid(PP))
					{
						continue;
					}
					if (!bActorTagged && !PP->ComponentHasTag(IconEffectsTag))
					{
						continue;
					}
					WidgetSettings.Effects.PostProcess = PP->Settings;
					WidgetSettings.Effects.bHasPostProcess = true;
					break;
				}
			}
		}
		
		//Props
		TInlineComponentArray<UStaticMeshComponent*> PropMeshes(Actor);
		for (UStaticMeshComponent* PropComp : PropMeshes)
		{
			if (!IsValid(PropComp) || !PropComp->GetStaticMesh())
			{
				continue;
			}
			if (!bActorTagged && !PropComp->ComponentHasTag(IconEffectsTag))
			{
				continue;
			}
			FIconStudioProp P;
			P.Mesh = PropComp->GetStaticMesh();
			P.RelativePosition = (PropComp->GetComponentLocation() - Origin) / SubjectRadius;
			P.Rotation = PropComp->GetComponentRotation();
			P.PropScale = PropComp->GetComponentScale();
			P.bCastShadow = PropComp->CastShadow != 0;

			if (PropComp->GetStaticMesh())
			{
				const int32 SlotCount = PropComp->GetStaticMesh()->GetStaticMaterials().Num();
				for (int32 MatIdx = 0; MatIdx < SlotCount; ++MatIdx)
				{
					UMaterialInterface* Override = PropComp->GetMaterial(MatIdx);
					UMaterialInterface* Default = PropComp->GetStaticMesh()->GetStaticMaterials()[MatIdx].MaterialInterface;

					if (Override != Default)
					{
						P.MaterialOverrides.SetNum(FMath::Max(P.MaterialOverrides.Num(), MatIdx + 1));
						P.MaterialOverrides[MatIdx] = Override;
					}
				}
			}

			if (const bool* bWasIgnored = SavedIgnoreFlags.Find(P.Mesh))
			{
				P.bIgnoreEffects = *bWasIgnored;
			}
			WidgetSettings.Effects.Props.Add(P);
		}

		//Decal
		TInlineComponentArray<UDecalComponent*> DecalCompsOnActor(Actor);
		for (UDecalComponent* Dec : DecalCompsOnActor)
		{
			if (!IsValid(Dec))
			{
				continue;
			}

			if (!bActorTagged && !Dec->ComponentHasTag(IconEffectsTag))
			{
				continue;
			}
			FIconStudioDecal D;
			D.ComponentClass = Dec->GetClass();
			D.DecalMaterial = Dec->GetDecalMaterial();
			D.RelativePosition = (Dec->GetComponentLocation() - Origin) / SubjectRadius;
			D.Rotation = Dec->GetComponentRotation();
			D.DecalSizeScale = Dec->DecalSize / SubjectRadius;
			D.SortOrder = Dec->SortOrder;
			WidgetSettings.Effects.Decals.Add(D);
		}
	}

	//report log
	UE_LOG(LogTemp, Log, TEXT("[Icon Creator] Captured %d light(s), %s post-process, %d decal(s), %d prop(s) from level (subject radius %.0f)"),
		WidgetSettings.Effects.Lights.Num(),
		WidgetSettings.Effects.bHasPostProcess ? TEXT("1") : TEXT("no"),
		WidgetSettings.Effects.Decals.Num(),
		WidgetSettings.Effects.Props.Num(),
		 SubjectRadius);

	if (PreviewTexture)
	{
		PreviewFirstSelected();
	}

	return WidgetSettings.Effects.Lights.Num();
}

bool UIconGeneratorSubsystem::TryConsumeLiveSyncSlot()
{
	const double Now = FPlatformTime::Seconds();
	if (Now - LastLiveSyncTime < LiveSyncMinInterval)
	{
		return false;
	}
	LastLiveSyncTime = Now;
	return true;
}

void UIconGeneratorSubsystem::HandleActorMoved(AActor* Actor)
{
	if (!bLiveLevelSync || bInAutoPreview || !PreviewTexture || !IsStageActor(Actor) || !TryConsumeLiveSyncSlot())
	{
		return;
	}

	// NOTE: doing the capture inline would rebuild a world from inside an editor move notification = crash :)
	RequestLiveSync();
}

void UIconGeneratorSubsystem::HandleObjectPropertyChanged(UObject* Object, struct FPropertyChangedEvent& Event)
{
	if (!bLiveLevelSync || bInAutoPreview || !PreviewTexture)
	{
		return;
	}

	if (!IsStageActor(ResolveOwningActor(Object)) || !TryConsumeLiveSyncSlot())
	{
		return;
	}

	RequestLiveSync();
}

// raise the flag and make sure a ticker is scheduled; cheap and safe to call from inside any editor notification because it touches nothing but two members
void UIconGeneratorSubsystem::RequestLiveSync()
{
	bLiveSyncPending = true;
	if (!LiveSyncTickHandle.IsValid())
	{
		LiveSyncTickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UIconGeneratorSubsystem::TickLiveSync));
	}
}

// runs on the next tick, after the editor has finished whatever edit raised the flag and the world is in a settled state
// this is where the capture and re-render actually happen
bool UIconGeneratorSubsystem::TickLiveSync(float DeltaTime)
{
	LiveSyncTickHandle.Reset();
	if (bLiveSyncPending && bLiveLevelSync && PreviewTexture)
	{
		bLiveSyncPending = false;
		TGuardValue<bool> Guard(bInAutoPreview, true);
		CaptureEffectsFromLevel();
	}

	bLiveSyncPending = false;
	return false;
}

// drag start
void UIconGeneratorSubsystem::HandleBeginObjectMovement(UObject& Object)
{
	if (!bLiveLevelSync || !PreviewTexture || !IsStageActor(ResolveOwningActor(&Object)))
	{
		return;
	}
	
	bDragActive = true;
	if (!DragTickHandle.IsValid())
	{
		DragTickHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UIconGeneratorSubsystem::HandleDragTick));
	}
}


// per frame whilst dragging re-read the actor's live transform and re-previews it
bool UIconGeneratorSubsystem::HandleDragTick(float DeltaTime)
{
	if (bDragActive && bLiveLevelSync && PreviewTexture && TryConsumeLiveSyncSlot())
	{
		TGuardValue<bool> Guard(bInAutoPreview, true);
		RefreshStageTransforms();
	}
	return true;
}

// drag fast path: reread only the transform derived fields of the already captured effects in the same order the full capture took them then re-preview
// this skips the per frame ExportText snapshot of every light properly which never changes during a move so dragging stays smooth
// if the set of tagged effects do not line up with what was captured it safely defers to the full capture instead
void UIconGeneratorSubsystem::RefreshStageTransforms()
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;

	FVector Origin;
	float SubjectRadius = 50.f;
	if(!FindStageSubjectFrame(World, Origin, SubjectRadius))
	{
		return;
	}

	int32 Index = 0;
	int32 DecalIndex = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		const bool bActorTagged = Actor->ActorHasTag(IconEffectsTag);

		//lights
		TInlineComponentArray<ULightComponent*> LightComps(Actor);
		for (ULightComponent* Comp : LightComps)
		{
			if (!IsValid(Comp))
			{
				continue;
			}
			
			if (!bActorTagged && !Comp->ComponentHasTag(IconEffectsTag))
			{
				continue;
			}

			if (!WidgetSettings.Effects.Lights.IsValidIndex(Index))
			{
				CaptureEffectsFromLevel();
				return;
			}
			FIconStudioLight& Data = WidgetSettings.Effects.Lights[Index];
			Data.Direction = Comp->GetComponentRotation();
			Data.RelativePosition = (Comp->GetComponentLocation() - Origin) / SubjectRadius;
			++Index;
		}

		// decals
		TInlineComponentArray<UDecalComponent*> DecalCompsOnActor(Actor);
		for (UDecalComponent* Dec : DecalCompsOnActor)
		{
			if (!IsValid(Dec))
			{
				continue;
			}

			if (!bActorTagged && !Dec->ComponentHasTag(IconEffectsTag))
			{
				continue;
			}
			
			if(!WidgetSettings.Effects.Decals.IsValidIndex(DecalIndex))
			{
				CaptureEffectsFromLevel();
				return;
			}
			FIconStudioDecal& D = WidgetSettings.Effects.Decals[DecalIndex];
			D.RelativePosition = (Dec->GetComponentLocation() - Origin) / SubjectRadius;
			D.Rotation = Dec->GetComponentRotation();
			D.DecalSizeScale = Dec->DecalSize / SubjectRadius;
			++DecalIndex;
		}
	}

	if (Index != WidgetSettings.Effects.Lights.Num()
		|| DecalIndex != WidgetSettings.Effects.Decals.Num())
	{
		CaptureEffectsFromLevel();
		return;
	}
	
	if (PreviewTexture)
	{
		PreviewFirstSelected();
	}
}

//Drag end
void UIconGeneratorSubsystem::HandleEndObjectMovement(UObject& Object)
{
	if(!bDragActive)
	{
		return;
	}
	bDragActive = false;

	if (DragTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DragTickHandle);
		DragTickHandle.Reset();
	}
	RequestLiveSync();
}

//widget:: load / save the saved projects defaults into the live widget settings
void UIconGeneratorSubsystem::EnsurePoseTransaction(const FText& Description)
{
	LastPoseChangeTime = FPlatformTime::Seconds();

	if (bPoseTransactionOpen)
	{
		return;
	}
	
	UKismetSystemLibrary::BeginTransaction(TEXT("Icon Creator"), Description, nullptr);
	UKismetSystemLibrary::TransactObject(this);
	bPoseTransactionOpen = true;

	if (!PoseTransactionTickHandle.IsValid())
	{
		PoseTransactionTickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UIconGeneratorSubsystem::TickPoseTransaction), 0.1f);
	}
}

bool UIconGeneratorSubsystem::TickPoseTransaction(float DeltaTime)
{
	if (bPoseTransactionOpen && (FPlatformTime::Seconds() - LastPoseChangeTime) > PoseIdleSeconds)
	{
		ClosePoseTransaction();
		PoseTransactionTickHandle.Reset();
		return false;
	}
	return true;
}

void UIconGeneratorSubsystem::ClosePoseTransaction()
{
	if (bPoseTransactionOpen)
	{
		UKismetSystemLibrary::EndTransaction();
		bPoseTransactionOpen = false;
	}
}

void UIconGeneratorSubsystem::LoadProjectDefaults()
{
	WidgetSettings = GetDefault<UIconCreatorSettings>()->DefaultSettings;
	OutputSettings = WidgetSettings.Output;

	InvalidatePreviewRig();
	if (PreviewTexture)
	{
		RefreshPreview();
	}
}

void UIconGeneratorSubsystem::SaveAsProjectDefaults()
{
	WidgetSettings.Output = OutputSettings;
	UIconCreatorSettings* Defaults = GetMutableDefault<UIconCreatorSettings>();
	Defaults->DefaultSettings = WidgetSettings;
	Defaults->TryUpdateDefaultConfigFile();
}

void UIconGeneratorSubsystem::SavePreset()
{
	const FString Name = NewPresetName.IsEmpty() ? SelectedPreset : NewPresetName;
	if (Name.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Save Preset: type a name into the New Preset Name first"))
		return;
	}

	UIconCreatorSettings* Config = GetMutableDefault<UIconCreatorSettings>();
	WidgetSettings.Output = OutputSettings;
	Config->Presets.Add(Name, WidgetSettings);
	Config->TryUpdateDefaultConfigFile();

	SelectedPreset = Name;
	NewPresetName.Reset();
	UE_LOG(LogTemp, Log, TEXT("[Icon Creator] Saved preset '%s'. "), *Name);
}

void UIconGeneratorSubsystem::LoadPreset()
{
	if (SelectedPreset.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Load Preset: select a preset from the dropdown first"))
		return;
	}

	const FIconGenSettings* Found = GetDefault<UIconCreatorSettings>()->Presets.Find(SelectedPreset);
	if (!Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Load Preset: preset '%s' not found"), *SelectedPreset);
		return;
	}

	WidgetSettings = *Found;
	OutputSettings = WidgetSettings.Output;

	InvalidatePreviewRig();
	if (PreviewTexture)
	{
		RefreshPreview();
	}
	UE_LOG(LogTemp, Log, TEXT("[Icon Creator] Loaded preset '%s'. "), *SelectedPreset);
}

void UIconGeneratorSubsystem::DeletePreset()
{
	if (SelectedPreset.IsEmpty() || GetMutableDefault<UIconCreatorSettings>()->Presets.Remove(SelectedPreset) == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Delete Preset: select a preset from the dropdown first"))
		return;
	}

	UIconCreatorSettings* Config = GetMutableDefault<UIconCreatorSettings>();
	Config->TryUpdateDefaultConfigFile();
	UE_LOG(LogTemp, Log, TEXT("[Icon Creator] Deleted preset '%s'. "), *SelectedPreset);
	SelectedPreset.Reset();
}

TArray<FString> UIconGeneratorSubsystem::GetPresetNames()
{
	TArray<FString> Names;
	GetDefault<UIconCreatorSettings>()->Presets.GetKeys(Names);
	Names.Sort();
	return Names;
}

// PreviewMesh if one is pinned ignores the Content Browser entirely so you can click away without losing your subject
// otherwise the first Static Mesh selected in the content browser
UTexture2D* UIconGeneratorSubsystem::RefreshPreview()
{
	if (bLiveLevelSync && bNeedsInitialCapture)
	{
		bNeedsInitialCapture = false;
		CaptureEffectsFromLevel();
	}

	if (IsValid(PreviewMesh))
	{
		return PreviewIcon(PreviewMesh, WidgetSettings);
	}

	for (UObject* Obj : UEditorUtilityLibrary::GetSelectedAssets())
	{
		if (UStaticMesh* Mesh = Cast<UStaticMesh>(Obj))
		{
			PreviewMesh = Mesh;
			return PreviewIcon(Mesh, WidgetSettings);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Set a preview mesh or select a static mesh in the Content Browser"));
	return nullptr;
}

//for the widget to call the preview
UTexture2D* UIconGeneratorSubsystem::PreviewFirstSelected()
{
	return RefreshPreview();
}

//explicitly pin the current Content Browser selection; useful as a "Use Selected" button next to the Preview Mesh field
UTexture2D* UIconGeneratorSubsystem::SetPreviewMesh(UStaticMesh* InMesh)
{
	PreviewMesh = InMesh;
	return RefreshPreview();
}

//Generate icons for every selected static mesh
void UIconGeneratorSubsystem::InvalidatePreviewRig()
{
	if (bRenderInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Icon Creator] Ignored a rig invalidation that arrived during a render"));
		return;
	}

	delete CachedRig;
	CachedRig = nullptr;
}

//Level Capture: level actors to plain data
UTexture2D* UIconGeneratorSubsystem::OrbitPreview(float DeltaYaw, float DeltaPitch)
{
	EnsurePoseTransaction(LOCTEXT("IconCreatorOrbit", "Icon Creator: Orbit Camera"));

	WidgetSettings.CameraOrbit.Yaw = FRotator::NormalizeAxis(WidgetSettings.CameraOrbit.Yaw);
	WidgetSettings.CameraOrbit.Pitch = FMath::Clamp(WidgetSettings.CameraOrbit.Pitch + DeltaPitch, -89.f, 89.f);
	return PreviewFirstSelected();
}

//RMB drag posing
UTexture2D* UIconGeneratorSubsystem::RotateMeshPreview(float DeltaYaw, float DeltaPitch)
{
	EnsurePoseTransaction(LOCTEXT("IconCreatorRotate", "Icon Creator: Rotate Mesh"));

	WidgetSettings.MeshRotation.Yaw = FRotator::NormalizeAxis(WidgetSettings.MeshRotation.Yaw + DeltaYaw);
	WidgetSettings.MeshRotation..Pitch = FRotator::NormalizeAxis(WidgetSettings.MeshRotation.Pitch + DeltaPitch);
	return PreviewFirstSelected();
}

//preview (cached rig + in-place texture)
static bool IsPreviewWorldUsable(const FIconCreatorRig& Rig)
{
	if (!Rig.Scene.IsValid())
	{
		return false;
	}
	UWorld* World = Rig.Scene->GetWorld();
	return IsValid(World) && World->PersistentLevel != nullptr;
}

bool UIconGeneratorSubsystem::RigIsValid(const FIconCreatorRig* Rig)
{
	if (!IsPreviewWorldUsable(Rig) || !IsValid(Rig->MeshComp) || !IsValid(Rig.Capture) || !IsValid(Rig.RenderTarget))
	{
		return false;
	}
}



TArray<UTexture2D*> UIconGeneratorSubsystem::GenerateFromSelection()
{

}

TArray<UTexture2D*> UIconGeneratorSubsystem::GenerateIcons(const TArray<UStaticMesh*>& Meshes,
	const FIconGenSettings& Settings)
{
}

bool UIconGeneratorSubsystem::FindStageSubjectFrame(class UWorld* World, FVector& OutOrigin, float& OutRadius)
{
}

bool UIconGeneratorSubsystem::IsStageActor(const AActor* Actor)
{
}



bool UIconGeneratorSubsystem::RigCompatible(const FIconGenSettings& A, const FIconGenSettings& B)
{
}

UTexture2D* UIconGeneratorSubsystem::PreviewIcon(UStaticMesh* Mesh, const FIconGenSettings& InSettings)
{
	//Missing implementation

	// copy our CPU Pixels straight into the texture's first mip; lock gives a writeable pointer to the GPU-upload buffer
	void* Data = PreviewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, Pixels.GetData(),Pixels.Num()* sizeof(FColor));
	PreviewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	PreviewTexture->UpdateResource();
	return PreviewTexture;
}

void UIconGeneratorSubsystem::BuildRig(FIconCreatorRig* Rig, const FIconGenSettings& Settings) const
{
	Rig.Scene = MakeUnique<FAdvancedPreviewScene>(FPreviewScene::ConstructionValues());
	FAdvancedPreviewScene& Scene = *Rig.Scene;
	
	Scene.SetFloorVisibility(false, true); // hide the default floor
	Scene.SetEnvironmentVisibility(false, true); //never shot the HDRI backdrop sphere

	//the subject slot repointed at each mesh in the batch
	Rig.MeshComp = NewObject<UStaticMeshComponent>(GetTransientPackage());
	USceneCaptureComponent2D* Capture = Rig.Capture;

	//The Camera
	Rig.Capture = NewObject<USceneCaptureComponent2D>(GetTransientPackage());
	USceneCaptureComponent2D* Capture = Rig.Capture;
	Capture->bCaptureEveryFrame = false;

	//flush all deferred component registrations NOW, so every scene proxy (mesh, props, lights, decals etc) exists before the first CaptureScene()
	Scene.GetWorld()->SendAllEndOfFrameUpdates();
}

bool UIconGeneratorSubsystem::RenderIconPixels(FIconCreatorRig* Rig, UStaticMesh* Mesh,
	const FIconGenSettings& Settings, TArray<FColor>& OutPixels) const
{
}

UTexture2D* UIconGeneratorSubsystem::WriteTextureAsset(const TArray<FColor>& Pixels, int32 Width, int32 Height,
	const FString& AssetBaseName, const FIconGenSettings& Settings) const
{
}

FString UIconGeneratorSubsystem::MakeIconAssetName(const UStaticMesh* Mesh, const FIconGenSettings& Settings)
{
}

FString UIconGeneratorSubsystem::MakeIconObjectPath(const FString& AssetBaseName, const FIconGenSettings& Settings)
{
}

FString UIconGeneratorSubsystem::ExportSettingsText(const FIconGenSettings& Settings)
{
}

FString UIconGeneratorSubsystem::ComputeIconHash(const UStaticMesh* Mesh, const FString& SettingsText)
{
}

FString UIconGeneratorSubsystem::ReadIconSourceHash(const UTexture2D* Texture)
{
}

void UIconGeneratorSubsystem::WriteIconSourceHash(UTexture2D* Texture, const FString& Hash)
{
	if (!Texture)
	{
		return;
	}
	UIconCreatorIconTag* Tag = Cast<UIconCreatorIconTag>(
	Texture->GetAssetUserDataOfClass(UIconCreatorIconTag::StaticClass()));
	if (!Tag)
	{
		Tag = NewObject<UIconCreatorIconTag>(Texture, NAME_None, RF_Public);
		Texture->AddAssetUserData(Tag);
	}
	Tag->SourceHash = Hash;
}

#undef LOCTEXT_NAMESPACE
*/
