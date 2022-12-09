// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "LandscapeClipDetails.h"
#include "BaseLandscapeClip.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EarthLandscapeClip.h"
#include "IDetailGroup.h"
#include "LandscapeClip.h"
#include "TerrainMagicManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "gdal/gdal.h"
#include "gdal/gdal_priv.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "LandscapeClipDetails"

void LoadDLL()
{
	const auto Handle = FPlatformProcess::GetDllHandle(TEXT("C:\\Data\\Repos\\GDi4K\\terrain-magic\\TerrainMagicDevApp\\Plugins\\TerrainMagic\\Source\\TerrainMagicEditor\\ThirdParty\\gdal304.dll"));
	if (Handle == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load required library"));
	}
}

TSharedRef<IDetailCustomization> FLandscapeClipDetails::MakeInstance()
{
	return  MakeShareable(new FLandscapeClipDetails());
}

void FLandscapeClipDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("00-TerrainMagic", LOCTEXT("CatName", "TerrainMagic"), ECategoryPriority::Important);
	DetailBuilder.GetObjectsBeingCustomized(CustomizingActors);
	
	auto WidgetRow = SNew(SGridPanel)
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
			];

	if (IsEarthLandscapeClip())
	{
		WidgetRow = SNew(SGridPanel)
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
			+SGridPanel::Slot(0, 3).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenMapButton", "Open the Map"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnOpenMap)
			]
			+SGridPanel::Slot(1, 3).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("DownloadButton", "Download Tile"))
				.ToolTipText(LOCTEXT("DownloadButtonTooltip", "Use ALT+D"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickDownloadTile)
			];
	}

	if (IsBaseLandscapeClip())
	{
		WidgetRow = SNew(SGridPanel)
			+SGridPanel::Slot(0, 0).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("InvalidateButton", "Invalidate"))
				.ToolTipText(LOCTEXT("InvalidateButtonToolTip", "Use ALT+Q or Editor Toolbar"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnClickInvalidate)
			];
	}	
	
	MyCategory.AddGroup("Actions", LOCTEXT("Actions", "Actions"), false, true)
		.AddWidgetRow()[WidgetRow];
}

FReply FLandscapeClipDetails::OnClickInvalidate()
{
	for (TWeakObjectPtr<UObject> Actor: CustomizingActors)
	{
		if (!Actor.IsValid())
		{
			continue;
		}

		ATerrainMagicManager* Manager = Cast<ATerrainMagicManager>(UGameplayStatics::GetActorOfClass(Actor->GetWorld(), ATerrainMagicManager::StaticClass()));
		Manager->InvalidateClips();
		break;
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
	for (TWeakObjectPtr<UObject> Actor: CustomizingActors)
	{
		if (!Actor.IsValid())
		{
			continue;
		}

		ATerrainMagicManager* Manager = Cast<ATerrainMagicManager>(UGameplayStatics::GetActorOfClass(Actor->GetWorld(), ATerrainMagicManager::StaticClass()));
		Manager->TogglePreview();
		break;
	}
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnClickDownloadTile()
{
	UE_LOG(LogTemp, Warning, TEXT("Download Me."))
	GDALAllRegister();
	GDALDataset* SourceRasterDS = (GDALDataset*) GDALOpen("C:\\Data\\Tmp\\terrain-files\\sample.tif",GA_ReadOnly);
	// auto Band = SourceRasterDS->GetRasterBand(1);
	// auto data = Band->AsMDArray();
	// data->GetDimensionCount();
	double transform[10];
	auto ref = SourceRasterDS->GetGeoTransform(transform);
	UE_LOG(LogTemp, Warning, TEXT("Transform: %f"), transform[0])
	
	// for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	// {
	// 	AEarthLandscapeClip* EarthLandscapeClip = Cast<AEarthLandscapeClip>(Clip);
	// 	if (EarthLandscapeClip != nullptr)
	// 	{
	// 		EarthLandscapeClip->DownloadTile([](const FEarthTileDownloadStatus Status)
	// 		{
	// 			if (Status.IsError)
	// 			{
	// 				UE_LOG(LogTemp, Error, TEXT("Tile Download Error: %s"), *Status.ErrorMessage);
	// 				
	// 				FNotificationInfo Info(LOCTEXT("SpawnNotification_Notification", "Tile Download Error"));
	// 				Info.ExpireDuration = 10.0f;
	// 				Info.WidthOverride = 300.0f;
	// 				Info.Hyperlink = FSimpleDelegate();
	// 				Info.Hyperlink.BindLambda([]() {});
	// 				Info.HyperlinkText = FText::FromString("Check Output Log for details");
	// 				FSlateNotificationManager::Get().AddNotification(Info);
	// 			}
	// 		});
	// 	}
	// }
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnOpenMap()
{
	FPlatformProcess::LaunchURL(TEXT("https://www.gdi4k.com/terrainmagic/map"), NULL, NULL);
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

bool FLandscapeClipDetails::IsEarthLandscapeClip()
{
	const TArray<ALandscapeClip*> SelectedClips = GetSelectedLandscapeClips();
	if (SelectedClips.Num() == 0)
	{
		return false;
	}

	for (ALandscapeClip* Clip: SelectedClips)
	{
		const AEarthLandscapeClip* EarthLandscapeClip = Cast<AEarthLandscapeClip>(Clip);
		if (EarthLandscapeClip == nullptr)
		{
			return false;
		}
	}

	return true;
}

bool FLandscapeClipDetails::IsBaseLandscapeClip()
{
	const TArray<ALandscapeClip*> SelectedClips = GetSelectedLandscapeClips();
	if (SelectedClips.Num() == 0)
	{
		return false;
	}

	for (ALandscapeClip* Clip: SelectedClips)
	{
		const ABaseLandscapeClip* BaseLandscapeClip = Cast<ABaseLandscapeClip>(Clip);
		if (BaseLandscapeClip == nullptr)
		{
			return false;
		}
	}

	return true;
}


#undef LOCTEXT_NAMESPACE
