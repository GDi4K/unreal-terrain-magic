// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "LandscapeClipDetails.h"
#include "Clips/BaseLandscapeClip.h"
#include "DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Clips/EarthLandscapeClip.h"
#include "Clips/GeoTiffLandscapeClip.h"
#include "IDetailGroup.h"
#include "LandscapeClip.h"
#include "TerrainMagicManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "gdal/gdal_priv.h"
#include "gdal/cpl_conv.h"
#include "IDesktopPlatform.h"
#include "Landscape.h"
#include "Utils/TerrainMagicThreading.h"

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

	if (IsGeoTiffLandscapeClip())
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
				.Text(LOCTEXT("Import", "Import GeoTiff"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnImportGeoTiff)
			]
			+SGridPanel::Slot(1, 3).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("UpdateLandscapeSize", "Update Landscape Size"))
				.ToolTipText(LOCTEXT("UpdateLandscapeSizeToolTip", "Update the landscape scale to match the real size of the terrain"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnUpdateLandscapeSize)
			]
			+SGridPanel::Slot(2, 3).Padding(5, 2)
			[
				SNew(SButton)
				.Text(LOCTEXT("ResetLandscapeSize", "Reset"))
				.ToolTipText(LOCTEXT("ResetLandscapeSizeTooltip", "Reset the landscape scale back to default settings"))
				.OnClicked_Raw(this, &FLandscapeClipDetails::OnResetLandscapeSize)
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
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		AEarthLandscapeClip* EarthLandscapeClip = Cast<AEarthLandscapeClip>(Clip);
		if (EarthLandscapeClip != nullptr)
		{
			EarthLandscapeClip->DownloadTile([](const FEarthTileDownloadStatus Status)
			{
				if (Status.IsError)
				{
					UE_LOG(LogTemp, Error, TEXT("Tile Download Error: %s"), *Status.ErrorMessage);
					
					FNotificationInfo Info(LOCTEXT("SpawnNotification_Notification", "Tile Download Error"));
					Info.ExpireDuration = 10.0f;
					Info.WidthOverride = 300.0f;
					Info.Hyperlink = FSimpleDelegate();
					Info.Hyperlink.BindLambda([]() {});
					Info.HyperlinkText = FText::FromString("Check Output Log for details");
					FSlateNotificationManager::Get().AddNotification(Info);
				}
			});
		}
	}
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnOpenMap()
{
	FPlatformProcess::LaunchURL(TEXT("https://www.gdi4k.com/terrainmagic/map"), NULL, NULL);
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnImportGeoTiff()
{
	UE_LOG(LogTemp, Warning, TEXT("Importing the GeoTiff file..."))

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	constexpr uint32 SelectionFlag = 0; //A value of 0 represents single file selection while a value of 1 represents multiple file selection
	TArray<FString> SelectedFiles;
	DesktopPlatform->OpenFileDialog(nullptr, "Select the GeoTiff File", "", "", "", SelectionFlag, SelectedFiles);

	if (SelectedFiles.Num() == 0)
	{
		return FReply::Handled();
	}
	
	const char* SelectedFileName = StringCast<ANSICHAR>(ToCStr(SelectedFiles[0])).Get();
	
	// Load the GeoTiffClip
	AGeoTiffLandscapeClip* GeoTiffLandscapeClip = nullptr;
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		GeoTiffLandscapeClip = Cast<AGeoTiffLandscapeClip>(Clip);
		if (GeoTiffLandscapeClip != nullptr)
		{
			break;
		}
	}
		
	GDALAllRegister();
	GDALDataset  *dataset = (GDALDataset *) GDALOpen(SelectedFileName, GA_ReadOnly);
	
	if( dataset == NULL ) {
		FNotificationInfo Info(LOCTEXT("Failed_To_Open_GeoTiff_File", "Failed to open the GeoTiff file."));
		Info.ExpireDuration = 10.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}
	
	// Get image metadata
	const uint32 Width = dataset->GetRasterXSize();
	const uint32 Height = dataset->GetRasterYSize();

	FGeoTiffInfo GeoTiffInfo = {};
	GeoTiffInfo.TextureResolution.X = Width;
	GeoTiffInfo.TextureResolution.Y = Height;
	
	double geoTransform[6];
	if (dataset->GetGeoTransform(geoTransform) == CE_None ) {
		GeoTiffInfo.Origin.X = geoTransform[0];
		GeoTiffInfo.Origin.Y = geoTransform[3];
		GeoTiffInfo.PixelToMetersRatio.X = geoTransform[1];
		GeoTiffInfo.PixelToMetersRatio.Y = geoTransform[5];
	} else {
		FNotificationInfo Info(LOCTEXT("Unsupported GeoTiff File", "Unsupported GeoTiff file found!"));
		Info.ExpireDuration = 10.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}
	
	// Load the Data
	GDALRasterBand  *elevationBand = dataset->GetRasterBand(1);
	const int32 TargetResolution = GeoTiffLandscapeClip->GetTargetResolution();
	const uint32 ReadResolution = TargetResolution == -1? Width : TargetResolution;
	
	TArray<float> RawHeightData;
	RawHeightData.SetNumUninitialized(ReadResolution * ReadResolution);
	elevationBand->RasterIO(GF_Read, 0, 0, Width, Height, RawHeightData.GetData(), ReadResolution, ReadResolution, GDT_Float32, 0, 0);

	// Pass the Data to the Clip
	
	GeoTiffLandscapeClip->ApplyRawHeightData(GeoTiffInfo, ReadResolution, RawHeightData);
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnUpdateLandscapeSize()
{
	// Load the GeoTiffClip
	AGeoTiffLandscapeClip* GeoTiffLandscapeClip = nullptr;
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		GeoTiffLandscapeClip = Cast<AGeoTiffLandscapeClip>(Clip);
		if (GeoTiffLandscapeClip != nullptr)
		{
			break;
		}
	}

	const FVector RealSize = GeoTiffLandscapeClip->GetUpdatedLandscapeSize();
	ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(GeoTiffLandscapeClip->GetWorld(), ALandscape::StaticClass()));
	Landscape->SetActorScale3D(RealSize);

	GeoTiffLandscapeClip->_Invalidate();
	GeoTiffLandscapeClip->_MatchLandscapeSizeDefferred(2);
	
	return FReply::Handled();
}

FReply FLandscapeClipDetails::OnResetLandscapeSize()
{
	AGeoTiffLandscapeClip* GeoTiffLandscapeClip = nullptr;
	for (ALandscapeClip* Clip: GetSelectedLandscapeClips())
	{
		GeoTiffLandscapeClip = Cast<AGeoTiffLandscapeClip>(Clip);
		if (GeoTiffLandscapeClip != nullptr)
		{
			break;
		}
	}
	
	ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(GeoTiffLandscapeClip->GetWorld(), ALandscape::StaticClass()));
	Landscape->SetActorScale3D({100.0f, 100.0f, 100.f});

	GeoTiffLandscapeClip->_Invalidate();
	GeoTiffLandscapeClip->_MatchLandscapeSizeDefferred(2);
	
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

bool FLandscapeClipDetails::IsGeoTiffLandscapeClip()
{
	const TArray<ALandscapeClip*> SelectedClips = GetSelectedLandscapeClips();
	if (SelectedClips.Num() == 0)
	{
		return false;
	}

	for (ALandscapeClip* Clip: SelectedClips)
	{
		const AGeoTiffLandscapeClip* GeoTiffLandscapeClip = Cast<AGeoTiffLandscapeClip>(Clip);
		if (GeoTiffLandscapeClip == nullptr)
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
