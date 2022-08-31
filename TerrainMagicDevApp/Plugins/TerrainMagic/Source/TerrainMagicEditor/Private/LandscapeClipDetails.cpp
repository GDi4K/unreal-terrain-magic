#include "LandscapeClipDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "LandscapeClip.h"
#include "Widgets/Layout/SGridPanel.h"

#define LOCTEXT_NAMESPACE "LandscapeClipDetails"

TSharedRef<IDetailCustomization> FLandscapeClipDetails::MakeInstance()
{
	return  MakeShareable(new FLandscapeClipDetails());
}

void FLandscapeClipDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("00-TerrainMagic", LOCTEXT("CatName", "TerrainMagic"), ECategoryPriority::Important);
	DetailBuilder.GetObjectsBeingCustomized(CustomizingActors);

	MyCategory.AddGroup("Actions", LOCTEXT("Actions", "Actions"), false, true)
		.AddWidgetRow()
		[
			SNew(SGridPanel)
			+SGridPanel::Slot(0, 0).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("ToggleOutlineButton", "Toggle Outline"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickToggleOutline)
			]
			+SGridPanel::Slot(1, 0).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("MatchLandscapeSizeButton", "Match Landscape Size"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickMatchLandscapeSize)
			]
			+SGridPanel::Slot(0, 1).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("TogglePreviewButton", "Toggle Preview"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickTogglePreview)
			]
			+SGridPanel::Slot(1, 1).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("ToggleSoloButton", "ToggleSolo"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickToggleSolo)
			]
			+SGridPanel::Slot(0, 2).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("InvalidateButton", "Invalidate"))
				.ToolTipText(LOCTEXT("InvalidateButtonToolTip", "Use ALT+Q or Editor Toolbar"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickInvalidate)
			]
		];
}

FReply FLandscapeClipDetails::OnClickInvalidate()
{
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		Clip->_Invalidate();
	}
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnClickMatchLandscapeSize()
{
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		Clip->_MatchLandscapeSize();
	}
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnClickToggleOutline()
{
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		Clip->_ToggleOutline();
	}
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnClickToggleSolo()
{
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		Clip->_ToggleSolo();
	}
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnClickTogglePreview()
{
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		Clip->_TogglePreview();
	}
	
	return FReply::Handled();
}

TArray<ALandscapeClip*> FLandscapeClipDetails::GetSelectedLandscapeClips()
{
	TArray<ALandscapeClip*> SelectedLandscapeClips;
	
	for (TWeakObjectPtr<UObject> Actor: CustomizingActors)
	{
		if (!Actor.IsValid())
		{
			continue;
		}

		ALandscapeClip* Clip = Cast<ALandscapeClip>(Actor.Get());
		if (!Clip)
		{
			continue;
		}

		SelectedLandscapeClips.Push(Clip);
	}

	return SelectedLandscapeClips;
}

#undef LOCTEXT_NAMESPACE
