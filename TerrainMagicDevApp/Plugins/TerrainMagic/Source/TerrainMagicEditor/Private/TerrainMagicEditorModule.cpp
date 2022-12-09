// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "TerrainMagicEditorModule.h"
#include "TerrainMagic/Public/LandscapeClip.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "ToolMenus.h"
#include "LandscapeClipDetails.h"
#include "LevelEditor.h"
#include "TerrainMagicCommands.h"
#include "TerrainMagicStyles.h"
#include "Interfaces/IPluginManager.h"

void LoadDLLs()
{
	FString BaseDir = IPluginManager::Get().FindPlugin("TerrainMagic")->GetBaseDir();

	auto dllPath = FPaths::Combine(*BaseDir, TEXT("Source\\TerrainMagicEditor\\ThirdParty\\geos_c.dll"));
	FPlatformProcess::GetDllHandle(*dllPath);

	dllPath = FPaths::Combine(*BaseDir, TEXT("Source\\TerrainMagicEditor\\ThirdParty\\gdal304.dll"));
	FPlatformProcess::GetDllHandle(*dllPath);
}

void FTerrainMagicEditorModule::StartupModule()
{
	LoadDLLs();
	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Initialize Styles & Commands
	FTerrainMagicStyles::Initialize();
	FTerrainMagicStyles::ReloadTextures();
	FTerrainMagicCommands::Register();

	// Handles Keyboard Shortcuts
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(FTerrainMagicCommands::Get().CommandsList.ToSharedRef());

	// Inject the button
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FTerrainMagicEditorModule::RegisterMenus));

	// Initialize the Custom Details Panel
	PropertyModule.RegisterCustomClassLayout(
		ALandscapeClip::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FLandscapeClipDetails::MakeInstance));
}

void FTerrainMagicEditorModule::ShutdownModule()
{
}

void FTerrainMagicEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

#if ENGINE_MAJOR_VERSION == 5
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
#else
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
#endif
	
	const FTerrainMagicCommands Commands = FTerrainMagicCommands::Get();
	FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(Commands.InvalidateLandscapeClipsAction));
	Entry.SetCommandList(Commands.CommandsList);

	FToolMenuEntry& Entry2 = Section.AddEntry(FToolMenuEntry::InitToolBarButton(Commands.TogglePreviewLandscapeClipsAction));
	Entry2.SetCommandList(Commands.CommandsList);
}

IMPLEMENT_MODULE(FTerrainMagicEditorModule, TerrainMagic)
