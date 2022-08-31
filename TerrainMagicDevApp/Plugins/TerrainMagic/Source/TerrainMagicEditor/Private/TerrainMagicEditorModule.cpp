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

void FTerrainMagicEditorModule::StartupModule()
{
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
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
	
	const FTerrainMagicCommands Commands = FTerrainMagicCommands::Get();
	FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(Commands.InvalidateLandscapeClipsAction));
	Entry.SetCommandList(Commands.CommandsList);
}

IMPLEMENT_MODULE(FTerrainMagicEditorModule, TerrainMagic)
