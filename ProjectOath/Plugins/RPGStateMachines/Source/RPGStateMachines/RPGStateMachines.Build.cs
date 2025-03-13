// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System;
using UnrealBuildTool;

public class RPGStateMachines : ModuleRules
{
	public RPGStateMachines(ReadOnlyTargetRules Target) : base(Target)
	{
		//Console.WriteLine("Engine directory: " + EngineDirectory);

		//// List available engine plugins
		//string pluginsDir = Path.Combine(EngineDirectory, "Plugins");
		//if (Directory.Exists(pluginsDir))
		//{
		//    string[] plugins = Directory.GetDirectories(pluginsDir, "*", SearchOption.AllDirectories);
		//    Console.WriteLine("Available engine plugins:");
		//    foreach (string plugin in plugins)
		//    {
		//        Console.WriteLine("  " + plugin);
		//    }
		//}

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
				"CoreUObject",
				"Engine",
				"InputCore",
				"StateTreeModule"
				// ... add other public dependencies that you statically link with here ...
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        // Add this to ensure the StateTree headers are found
        //PublicIncludePaths.Add(Path.Combine(EngineDirectory, "Plugins", "Runtime", "StateTree", "Source", "StateTreeModule", "Public"));
        //PublicIncludePaths.Add("E:/UE_5.5/Engine/Plugins/Runtime/StateTree/Source/StateTreeModule/Public");

        //PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Plugins", "Runtime", "StateTree", "Source", "StateTreeModule", "Private"));
        //PrivateIncludePaths.Add("E:/UE_5.5/Engine/Plugins/Runtime/StateTree/Source/StateTreeModule/Private");
    }
}
