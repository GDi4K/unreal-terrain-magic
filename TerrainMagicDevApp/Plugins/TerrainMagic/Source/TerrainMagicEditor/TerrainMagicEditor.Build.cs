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
		
		PublicDelayLoadDLLs.Add("gdal304.dll");
		PublicDelayLoadDLLs.Add("geos_c.dll");
		
		// // We don't need to load this DLL at runtime. That's we don't want to do this.
		// RuntimeDependencies.Add(PluginDirectory + "Binaries/Win64/gdal304.dll", ModuleDirectory + "/ThirdParty/gdal304.dll");
		// RuntimeDependencies.Add(PluginDirectory + "Binaries/Win64/geos_c.dll", ModuleDirectory + "/ThirdParty/geos_c.dll");
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine",
			"UnrealEd", "DetailCustomizations",
			"PropertyEditor", "EditorStyle", "DesktopPlatform"
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"Slate", "SlateCore", "TerrainMagic",
			"InputCore", "RenderCore",
			"ToolMenus", "Projects", "EditorStyle", "Landscape",
#if UE_5_0_OR_LATER
                "EditorFramework",
#endif
		});
	}
}