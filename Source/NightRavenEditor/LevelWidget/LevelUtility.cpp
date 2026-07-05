#include "LevelUtility.h"

#include "CoreMinimal.h"
#include "NightRavenEditor/DeveloperSettings/NightRavenDeveloperSettings.h" // trim this path
#include "Modules/ModuleManager.h"
#include "ToolMenu.h"
#include "ToolMenus.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "NightRavenEditor"

static bool HasPlayWorld()
{
	return GEditor->PlayWorld != nullptr;
}

static bool HasNoPlayWorld()
{
	return !HasPlayWorld();
}

static void OpenMap_Clicked(const FString MapPath)
{
	if (ensure(MapPath.Len()))
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(MapPath);
	}
}

static bool CanShowMaps()
{
	return HasNoPlayWorld() && !GetDefault<UNightRavenDeveloperSettings>()->EditorMaps.IsEmpty();
}

static TSharedRef<SWidget> GetMapsDropdown()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	for (const FSoftObjectPath& Path : GetDefault<UNightRavenDeveloperSettings>()->EditorMaps)
	{
		if (!Path.IsValid())
		{
			continue;
		}

		const FText DisplayName = FText::FromString(Path.GetAssetName());
		MenuBuilder.AddMenuEntry(
			DisplayName,
			LOCTEXT("PathDescription", "Opens this map in the editor"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&OpenMap_Clicked, Path.ToString()),
				FCanExecuteAction::CreateStatic(&HasNoPlayWorld),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateStatic(&HasNoPlayWorld)
				)
			);
	}
	return MenuBuilder.MakeWidget();
}

static void RegisterGameEditorMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolbar.PlayToolBar");
	FToolMenuSection& Section = Menu->AddSection("PlayGameExtension", TAttribute<FText>(), FToolMenuInsert("Play", EToolMenuInsertType::After));

	FToolMenuEntry MapEntry = FToolMenuEntry::InitComboButton(
		"MapOptions",
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateStatic(&HasNoPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateStatic(&CanShowMaps)),
			FOnGetContent::CreateStatic(&GetMapsDropdown),
			LOCTEXT("Maps_Label", "Maps"),
			LOCTEXT("Maps_ToolTip", "A list of maps while using the editor"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level")
			);
	MapEntry.StyleNameOverride = "CalloutToolbar";
	Section.AddEntry(MapEntry);
}

class FNightRavenEditorModule : public FDefaultGameModuleImpl
{
	typedef FNightRavenEditorModule ThisClass;

	virtual void StartupModule() override
	{
		if (!IsRunningGame())// There's a lot of cool things in IsRunning like dedicated server
			//( might be worth looking into for server overhead tools
		{
			if (FSlateApplication::IsInitialized())
			{
				ToolMenusHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterGameEditorMenus));
			}
		}
	}
private:
	FDelegateHandle ToolMenusHandle;
};


IMPLEMENT_MODULE(FNightRavenEditorModule, NightRavenEditorModule);

#undef LOCTEXT_NAMESPACE