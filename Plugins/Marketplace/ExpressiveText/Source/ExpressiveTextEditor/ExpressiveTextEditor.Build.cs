// Copyright 2022 Guganana. All Rights Reserved.
using UnrealBuildTool;

public class ExpressiveTextEditor : ModuleRules
{
	public ExpressiveTextEditor(ReadOnlyTargetRules Target) : base(Target)
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
					"ApplicationCore",
					"AssetRegistry",
					"Blutility",
					"Core",
					"CoreUObject",
					"EditorStyle",
					"EditorScriptingUtilities",
					"EditorSubsystem",
					"Engine",
					"ExpressiveText",
					"HTTP",
					"InputCore",
					"Json",
					"JsonUtilities",
					"MaterialEditor",
					"MaterialUtilities",
					"Persona",
					"Projects",
					"PropertyEditor",
					"Sequencer",
					"SharedSettingsWidgets",
					"Slate",
					"SlateCore",
					"MovieScene",
					"ToolMenus",
					"UMG",
					"UMGEditor",
					"UnrealEd"
				}
			);
		
		
		if(Target.Version.MajorVersion == 5)
        {

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"ToolWidgets",
				}
			);

			if(Target.Version.MinorVersion >= 3)
			{
				PublicDependencyModuleNames.Add("ScriptableEditorWidgets");
			}
		}

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorWidgets",
				"SlateCore",
				"Engine"
			}
		);
	}
}
