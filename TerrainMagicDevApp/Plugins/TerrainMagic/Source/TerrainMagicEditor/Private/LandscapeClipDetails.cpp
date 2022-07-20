#include "LandscapeClipDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "LandscapeClip.h"

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

	// You can get properties using the DetailBuilder:
	//MyProperty= DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(MyClass, MyClassPropertyName));

	MyCategory.AddCustomRow(LOCTEXT("RowName", "Hello Row"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("KeyName", "Actions"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("InvalidateButton", "Invalidate"))
			.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickInvalidate)
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
