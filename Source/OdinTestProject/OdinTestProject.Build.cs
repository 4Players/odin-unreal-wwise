// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OdinTestProject : ModuleRules
{
	public OdinTestProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
			"Odin",
			"AkAudio",
			"Wwise",
			"OdinLibrary"  });
	}
}
