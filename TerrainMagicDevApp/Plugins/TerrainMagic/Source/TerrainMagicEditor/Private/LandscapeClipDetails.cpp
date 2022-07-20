#include "LandscapeClipDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "LandscapeClip.h"
#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "LandscapeClipDetails"

TSharedRef<IDetailCustomization> FLandscapeClipDetails::MakeInstance()
{
	return  MakeShareable(new FLandscapeClipDetails());
}

void FLandscapeClipDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("01-General", LOCTEXT("CatName", "01-General"), ECategoryPriority::Important);
	DetailBuilder.GetObjectsBeingCustomized(CustomizingActors);
	
	MyCategory.AddCustomRow(LOCTEXT("RowName", "Hello Row"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("KeyName", "Actions"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SWrapBox)
			.Orientation(EOrientation::Orient_Vertical)
			+SWrapBox::Slot().Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("InvalidateButton", "Invalidate"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickInvalidate)
			]
			+SWrapBox::Slot().Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("MatchLandscapeSizeButton", "Match Landscape Size"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickMatchLandscapeSize)
			]
			+SWrapBox::Slot().Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("ToggleOutlineButton", "Toggle Outline"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickToggleOutline)
			]
			+SWrapBox::Slot().Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("TogglePreviewButton", "Toggle Preview"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickTogglePreview)
			]
			+SWrapBox::Slot().Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("ToggleSoloButton", "ToggleSolo"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickToggleSolo)
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
