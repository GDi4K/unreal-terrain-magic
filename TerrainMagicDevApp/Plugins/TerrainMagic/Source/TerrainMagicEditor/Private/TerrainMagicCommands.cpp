// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "TerrainMagicCommands.h"
#include "InputCoreTypes.h"
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
	// Put your "OnButtonClicked" stuff here
	const FText DialogText = FText::Format(
							LOCTEXT("TerrainMagic Invalidated!", "We clicked it: {0}, {1}"),
							FText::FromString(TEXT("FClickMeModule::PluginButtonClicked()")),
							FText::FromString(TEXT("ClickMe.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

#undef LOCTEXT_NAMESPACE
