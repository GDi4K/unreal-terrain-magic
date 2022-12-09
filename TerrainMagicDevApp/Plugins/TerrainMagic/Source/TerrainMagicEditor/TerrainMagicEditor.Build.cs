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
		
		PublicAdditionalLibraries.Add(ModuleDirectory + "/ThirdParty/gdal_i.lib");
		RuntimeDependencies.Add(PluginDirectory + "Binaries/Win64/gdal304.dll");
		RuntimeDependencies.Add(PluginDirectory + "Binaries/Win64/geos_c.dll");
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine",
			"UnrealEd", "DetailCustomizations",
			"PropertyEditor", "EditorStyle"
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"Slate", "SlateCore", "TerrainMagic",
			"InputCore",
			"ToolMenus", "Projects", "EditorStyle",
#if UE_5_0_OR_LATER
                "EditorFramework",
#endif
		});
	}
}