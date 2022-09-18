// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "TerrainMagicCommands.h"

#include "EarthLandscapeClip.h"
#include "InputCoreTypes.h"
#include "LandscapeClip.h"
#include "Engine/Selection.h"
#define LOCTEXT_NAMESPACE "FTerrainMagicCommands"

void FTerrainMagicCommands::RegisterCommands()
{
	UI_COMMAND(InvalidateLandscapeClipsAction, "Invalidate", "Invalidate Landscape Clips", EUserInterfaceActionType::Button, FInputChord(EKeys::Q, false, false, true, false));
	CommandsList->MapAction(
		InvalidateLandscapeClipsAction,
		FExecuteAction::CreateRaw(this, &FTerrainMagicCommands::OnInvalidateLandscapeClips),
		FCanExecuteAction());

	UI_COMMAND(DownloadTileAction, "Downlod Tile", "Download Tile", EUserInterfaceActionType::Button, FInputChord(EKeys::D, false, false, true, false));
	CommandsList->MapAction(
		DownloadTileAction,
		FExecuteAction::CreateRaw(this, &FTerrainMagicCommands::OnDownloadTile),
		FCanExecuteAction());
}

void FTerrainMagicCommands::OnInvalidateLandscapeClips() const
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);
	
	for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
	{
		ALandscapeClip* Clip = Cast<ALandscapeClip>(*Iter);
		if (Clip)
		{
			Clip->_Invalidate();
			UE_LOG(LogTemp, Display, TEXT("Invalidating Landscape Clip: %s"), *Clip->GetName())
		}
	}

	
}

void FTerrainMagicCommands::OnDownloadTile() const
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);
	
	for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
	{
		AEarthLandscapeClip* Clip = Cast<AEarthLandscapeClip>(*Iter);
		if (Clip)
		{
			Clip->DownloadTile();
			UE_LOG(LogTemp, Display, TEXT("Downloding Tile: %s"), *Clip->GetName())
		}
	}
}

#undef LOCTEXT_NAMESPACE
