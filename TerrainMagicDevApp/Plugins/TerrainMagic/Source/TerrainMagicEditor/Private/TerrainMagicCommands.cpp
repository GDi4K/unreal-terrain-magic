// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "TerrainMagicCommands.h"
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

#undef LOCTEXT_NAMESPACE
