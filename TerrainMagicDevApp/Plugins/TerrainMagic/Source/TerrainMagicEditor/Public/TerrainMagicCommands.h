// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TerrainMagicStyles.h"
#include "Framework/Commands/Commands.h"

class FTerrainMagicCommands : public TCommands<FTerrainMagicCommands>
{
	static void ShowSelectAnActorNotification();
public:

	FTerrainMagicCommands()
		: TCommands<FTerrainMagicCommands>(TEXT("TerrainMagicCommands"), NSLOCTEXT("Contexts", "TerrainMagic", "TerrainMagic Plugin"), NAME_None, FTerrainMagicStyles::GetStyleSetName())
	{
		CommandsList = MakeShareable(new FUICommandList);
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;
	
	TSharedPtr<class FUICommandList> CommandsList;
	
	TSharedPtr< FUICommandInfo > InvalidateLandscapeClipsAction;
	TSharedPtr< FUICommandInfo > TogglePreviewLandscapeClipsAction;
	TSharedPtr< FUICommandInfo > DownloadTileAction;

	void OnInvalidateLandscapeClips() const;
	void OnTogglePreviewLandscapeClips() const;
	void OnDownloadTile() const;
};
