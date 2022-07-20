#include "LandscapeClipDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "LandscapeClipDetails"

TSharedRef<IDetailCustomization> FLandscapeClipDetails::MakeInstance()
{
	return  MakeShareable(new FLandscapeClipDetails());
}

void FLandscapeClipDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("01-General", LOCTEXT("CatName", "01-General"), ECategoryPriority::Important);

	// You can get properties using the DetailBuilder:
	//MyProperty= DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(MyClass, MyClassPropertyName));

	MyCategory.AddCustomRow(LOCTEXT("RowName", "Hello Row"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("KeyName", "Key"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ValueName", "Value"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];
}

#undef LOCTEXT_NAMESPACE