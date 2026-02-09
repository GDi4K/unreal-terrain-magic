// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TerrainMagicDevApp : ModuleRules
{
	public TerrainMagicDevApp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"TerrainMagic"
		});

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "XRBase" });
	}
}
