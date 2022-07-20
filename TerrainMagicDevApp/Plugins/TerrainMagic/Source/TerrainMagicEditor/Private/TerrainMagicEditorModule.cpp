// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "TerrainMagicEditorModule.h"
#include "TerrainMagic/Public/LandscapeClip.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "LandscapeClipDetails.h"

void FTerrainMagicEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Custom properties
	PropertyModule.RegisterCustomClassLayout(
		ALandscapeClip::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FLandscapeClipDetails::MakeInstance));
	
	UE_LOG(LogTemp, Warning, TEXT("Hello From Editor"))
}

void FTerrainMagicEditorModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FTerrainMagicEditorModule, TerrainMagic)
