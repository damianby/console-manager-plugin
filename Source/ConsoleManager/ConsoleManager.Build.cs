// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

using UnrealBuildTool;

using System.IO;

public class ConsoleManager : ModuleRules
{
	public ConsoleManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		

		//string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);

		//string srcrt_path = EnginePath + "Source/Editor/";
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
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"MainFrame",
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"LevelEditor",
				"Settings",
				"CoreUObject",
				"PropertyEditor",
				"Engine",
				"Slate",
				"SlateCore",
				"AssetTools",
				"DeviceProfileEditor",
				"EditorStyle"
				// ... add private dependencies that you statically link with here ...	
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
