// Copyright 2022 Guganana. All Rights Reserved.

using UnrealBuildTool;

public class ExpressiveText : ModuleRules
{
	public ExpressiveText(ReadOnlyTargetRules Target) : base(Target)
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
					"Core",
					"CoreUObject",
					"Engine",
                    "Json",
                    "JsonUtilities",
                    "HTTP",
					"Projects",
                    "Slate",
					"SlateCore",
					"UMG",
#if UE_4_26_OR_LATER 
					"DeveloperSettings",
#endif			
				}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if(Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("MaterialUtilities");
		}
	}
}
