// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "Clips/EarthLandscapeClip.h"
#include "LandscapeClip.h"
#include "Editor/PropertyEditor/Public/IDetailCustomization.h"
#include "Input/Reply.h"

class FLandscapeClipDetails : public IDetailCustomization
{
public:
	TArray<TWeakObjectPtr<UObject>> CustomizingActors;

	// Core
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

	// Events
	FReply OnClickInvalidate();
	FReply OnClickMatchLandscapeSize();
	FReply OnClickToggleOutline();
	FReply OnClickToggleSolo();
	FReply OnClickTogglePreview();
	FReply OnClickDownloadTile();
	FReply OnOpenMap();
	FReply OnImportGeoTiff();
	FReply OnUpdateLandscapeSize();
	FReply OnResetLandscapeSize();

	// Helpers
	TArray<ALandscapeClip*> GetSelectedLandscapeClips();
	bool IsEarthLandscapeClip();
	bool IsGeoTiffLandscapeClip();
	bool IsBaseLandscapeClip();
};
