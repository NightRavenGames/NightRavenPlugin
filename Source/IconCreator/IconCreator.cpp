/*
#include "IconCreator.h"
#include "ContentBrowserMenuContexts.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "IconCreatorTypes.h"
#include "IconGeneratorSubsystem.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "IconCreator"

void FIconCreatorModule::StartupModule()
{
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FIconCreatorModule::RegisterMenus));
}

void FIconCreatorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);

    UToolMenus::UnregisterOwner(this);
}
/*
//Filtered selected assets down to StaticMeshes, grabs the editor subsystem and hands the meshes to generate the icon
static void GenerateIconsForAssets(const TArray<FAssetData>& SelectedAssets, const FIconGenSettings& Settings)
{
    TArray<UStaticMesh*> Meshes;
    for (const FAssetData& AssetData : SelectedAssets)
    {
        if (UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset()))
        {
            Meshes.Add(Mesh);
        }

        if (Meshes.Num() == 0 || !GEditor)
        {
            return;
        }
        if (UIconGeneratorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UIconGeneratorSubsystem>())
        {
  //FIX          Subsystem->GenerateIcons(Meshes, Settings);
        }
    }
}

void FIconCreatorModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.StaticMesh");

    FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");

    Section.AddDynamicEntry("IconCreator_GenerateIcons",
        FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
        {
            const UContentBrowserAssetContextMenuContext* Context =
                InSection.FindContext<UContentBrowserAssetContextMenuContext>();
            if (!Context)
            {
                return;
            }

            TArray<FAssetData> SelectedAssets = Context->SelectedAssets;

            InSection.AddSubMenu(
                "IconCreator_Generate",
                LOCTEXT("GenerateIconsLabel", "Generate Icon(s)"),
                LOCTEXT("GenerateIconsTooltip", "Render UI icon textures for the selected meshes using a saved preset or the project defaults."),
                FNewMenuDelegate::CreateLambda([SelectedAssets](FMenuBuilder& MenuBuilder)
                {
                    const UIconCreatorSettings* Config = GetDefault<UIconCreatorSettings>();

                    MenuBuilder.AddMenuEntry(
                        LOCTEXT("GenerateWithDefaults", "Project Defaults"),
                        LOCTEXT("GenerateWithDefaultsTip", "Use the settings from Project Settings > Plugins > Icon Forge."),
                        FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Texture2D"),
                        FUIAction(FExecuteAction::CreateLambda([SelectedAssets, Config]()
                        {
                            GenerateIconsForAssets(SelectedAssets, Config->DefaultSettings);
                        })));

                    if (Config->Presets.Num() == 0)
                    {
                        return;
                    }

                    MenuBuilder.BeginSection("IconForgePresets", LOCTEXT("PresetsHeading", "Presets"));

                    TArray<FString> PresetNames;
                    Config->Presets.GetKeys(PresetNames);
                    PresetNames.Sort();

                    for (const FString& PresetName : PresetNames)
                    {
                        const FIconGenSettings PresetSettings = Config->Presets[PresetName];
                        MenuBuilder.AddMenuEntry(
                            FText::FromString(PresetName),
                            FText::Format(LOCTEXT("GenerateWithPresetTip", "Generate using the '{0}' preset."), FText::FromString(PresetName)),
                            FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Texture2D"),
                            FUIAction(FExecuteAction::CreateLambda([SelectedAssets, PresetSettings]()
                            {
                                GenerateIconsForAssets(SelectedAssets, PresetSettings);
                            })));
                    }

                    MenuBuilder.EndSection();
                }),
                false,
                FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Texture2D"));
        }));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FIconCreatorModule, IconCreator)
*/