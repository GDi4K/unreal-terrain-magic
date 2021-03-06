#pragma once

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

	// Helpers
	TArray<ALandscapeClip*> GetSelectedLandscapeClips();
	
};
