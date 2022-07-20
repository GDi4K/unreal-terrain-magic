// Copyright (c) 2022 GDi4K. All Rights Reserved.

using UnrealBuildTool;

public class TerrainMagicEditor : ModuleRules
{
	public TerrainMagicEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;

		PublicIncludePaths.Add(ModuleDirectory + "/Public");
		PrivateIncludePaths.Add(ModuleDirectory + "/Private");

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine",
			"UnrealEd", "DetailCustomizations",
			"PropertyEditor", "EditorStyle"
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"Slate", "SlateCore", "TerrainMagic"
		});
	}
}