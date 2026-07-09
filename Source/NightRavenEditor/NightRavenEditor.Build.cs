using UnrealBuildTool;

public class NightRavenEditor : ModuleRules
{
    public NightRavenEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                // Editor Specifics
                "UnrealEd",
                "EditorSubsystem",
                "LevelEditor",
                "DeveloperSettings"

            }
        );

        PublicIncludePaths.AddRange(new string[]
        {
            
        });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                //Editor Access
                "ToolMenus"
            }
        );
    }
}