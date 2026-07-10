#include "CustomBlockouts.h"
#include "IPlacementModeModule.h"
#include "ActorFactories/ActorFactoryStaticMesh.h"
#include "Engine/StaticMesh.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "CustomBlockouts"

static const FName CustomBlockoutsCategoryName("CustomBlockouts");

void FCustomBlockoutsModule::StartupModule()
{
    RegisterCategory();
}

void FCustomBlockoutsModule::ShutdownModule()
{
    if (IPlacementModeModule::IsAvailable())
    {
        IPlacementModeModule::Get().UnregisterPlacementCategory(CustomBlockoutsCategoryName);
    }
}

void FCustomBlockoutsModule::RegisterCategory()
{
    if (!IPlacementModeModule::IsAvailable())
    {
        return;
    }

    IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

    const FPlacementCategoryInfo CategoryInfo(
        LOCTEXT("CategoryName", "Custom BLockouts"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "PlacementBrowser.Icons.Basic"),
        CustomBlockoutsCategoryName,
        TEXT("PMCustomBlockouts"),
        35
        );

    PlacementModeModule.RegisterPlacementCategory(CategoryInfo);

    int32 SortOrder = 0;

    // Register meshes // Need more meshes to actually load at this point

    // Here is a case example of how to register a mesh type.
    /*
    RegisterBlockoutMesh(
        CustomBlockoutsCategoryName,
        TEXT("/Game/StarterContent/Props/Blockout_01.Blockout_01"),
        LOCTEXT("Blockout_01", "Blockout_01"),
        SortOrder += 10,
        FName("Sample Mesh")
        );

    */
}

void FCustomBlockoutsModule::RegisterBlockoutMesh(
    const FName& CategoryHandle,
    const TCHAR* AssetPath,
	const FText& DisplayName,
	int32 SortOrder,
	FName IconName)
{
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, AssetPath);
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CustomBlockouts: Failed to load mesh at %s"), AssetPath);
        return;
    }

    IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

    PlacementModeModule.RegisterPlaceableItem(
        CategoryHandle,
        MakeShareable(new FPlaceableItem(
            *UActorFactoryStaticMesh::StaticClass(),
            FAssetData(Mesh),
            IconName,
            IconName,
            TOptional<FLinearColor>(),
            SortOrder,
            DisplayName
            ))
            );
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FCustomBlockoutsModule, CustomBlockouts)