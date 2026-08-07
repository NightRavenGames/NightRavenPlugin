using UnrealBuildTool;

public class IconCreator : ModuleRules
{
    public IconCreator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                
                "Slate", "SlateCore", "UnrealEd", "EditorSubsystem", "Blutility", "AdvancedPreviewScene", "AssetRegistry",
                "RenderCore", "RHI", "ImageCore", "ToolMenus", "ContentBrowser", "DeveloperSettings"
            }
        );
    }
}