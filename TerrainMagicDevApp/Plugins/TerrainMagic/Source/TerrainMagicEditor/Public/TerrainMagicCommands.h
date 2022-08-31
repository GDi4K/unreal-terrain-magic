// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TerrainMagicStyles.h"
#include "Framework/Commands/Commands.h"

class FTerrainMagicCommands : public TCommands<FTerrainMagicCommands>
{
public:

	FTerrainMagicCommands()
		: TCommands<FTerrainMagicCommands>(TEXT("TerrainMagicCommands"), NSLOCTEXT("Contexts", "TerrainMagic", "TerrainMagic Plugin"), NAME_None, FTerrainMagicStyles::GetStyleSetName())
	{
		CommandsList = MakeShareable(new FUICommandList);
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<class FUICommandList> CommandsList;
	TSharedPtr< FUICommandInfo > InvalidateLandscapeClipsAction;

	void OnInvalidateLandscapeClips() const;
};
