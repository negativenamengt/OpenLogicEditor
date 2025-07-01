// Copyright 2024 - NegativeNameSeller

using UnrealBuildTool;

public class OpenLogicEditor : ModuleRules
{
	public OpenLogicEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PropertyEditor",
				"GraphEditor",
				"BlueprintGraph"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AssetTools",
				"UnrealEd",
                "OpenLogicV2",
                "Blutility",
				"UMG",
                "UMGEditor",
                "DetailCustomizations" 
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
				
			}
			);
	}
}
