// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Clips/EarthLandscapeClip.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Utils/MapUtils.h"
#include "Utils/TerrainMagicThreading.h"
#include "Engine/World.h"


float smoothstep (float edge0, float edge1, float x)
{
	if (x < edge0)
		return 0;

	if (x >= edge1)
		return 1;

	// Scale/bias into [0..1] range
	x = (x - edge0) / (edge1 - edge0);

	return x * x * (3 - 2 * x);
}

// Sets default values
AEarthLandscapeClip::AEarthLandscapeClip()
{
}

UMaterial* AEarthLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "Material'/TerrainMagic/Core/Materials/LandscapeClips/M_TM_LandscapeClip_Earth.M_TM_LandscapeClip_Earth'";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AEarthLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;
	
	MaterialParams.Push({"Texture", HeightMap});
	MaterialParams.Push({"BlurDistance", static_cast<float>(BlurDistance)});
	MaterialParams.Push({"BlurDistanceSteps", static_cast<float>(BlurDistanceSteps)});
	MaterialParams.Push({"BlurRadialSteps", static_cast<float>(BlurRadialSteps)});

	MaterialParams.Push({"NoiseInfluence", NoiseInfluence});
	MaterialParams.Push({"MountainNoiseScale", MountainNoiseScale});
	MaterialParams.Push({"MountainNoiseDetails", MountainNoiseDetails});
	MaterialParams.Push({"MountainNoiseContrast", MountainNoiseContrast});
	MaterialParams.Push({"MountainNoiseSharpness", MountainNoiseSharpness});
	MaterialParams.Push({"MountainNoiseSeed", MountainNoiseSeed});
	MaterialParams.Push({"MountainNoisePosition", FVector(MountainNoisePosition.X, MountainNoisePosition.Y, 0)});


	MaterialParams.Push({"TileRepositionZoomScale", TileRepositioning.Scale});
	MaterialParams.Push({"TileRepositionPanning", FVector(TileRepositioning.Panning, 0.0)});
	MaterialParams.Push({"TileRepositionRotation", TileRepositioning.Rotation});

	float HeightMultiplierInput = (HeightMap == nullptr)? 0.0f : HeightMultiplier;
	MaterialParams.Push({"HeightMultiplier", HeightMultiplierInput});
	MaterialParams.Push({"SelectedBlendMode", static_cast<float>(BlendMode)});

	MaterialParams.Push({"HeightMapInputMin", HeightMapRange.InputMin});
	MaterialParams.Push({"HeightMapInputMax", HeightMapRange.InputMax});
	MaterialParams.Push({"HeightMapOutputMin", HeightMapRange.OutputMin});
	MaterialParams.Push({"HeightMapOutputMax", HeightMapRange.OutputMax});
	MaterialParams.Push({"HeightSaturation", HeightSaturation});
	MaterialParams.Push({"HeightAddition", GetHeightAddition()});
	
	MaterialParams.Push({"SelectedFadeMode", static_cast<float>(FadeMode)});
	MaterialParams.Push({"FadeSaturation", FadeSaturation});
	MaterialParams.Push({"FadeMaskSpan", FadeMaskSpan});
	MaterialParams.Push({"FadeUseRemapMin", static_cast<float>(FadeMinimum)});
	
	return MaterialParams;
}

int AEarthLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D AEarthLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void AEarthLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool AEarthLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void AEarthLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void AEarthLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int AEarthLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

void AEarthLandscapeClip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}

	ReloadTextureIfNeeded();
}

UTexture* AEarthLandscapeClip::GetHeightMap() const
{
	return HeightMap;
}

TArray<FLandscapeClipPaintLayerSettings> AEarthLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}

void AEarthLandscapeClip::ReloadTextureIfNeeded()
{
	if (HasTextureReloaded)
	{
		return;
	}
	HasTextureReloaded = true;

	if (IsValid(G16Texture))
	{
		HeightMap = G16Texture->LoadCachedTexture();
	}
}

void AEarthLandscapeClip::DownloadTile(TFunction<void(FEarthTileDownloadStatus)> StatusCallback)
{
	TArray<FString> Parts;
	TileInfoString.TrimStartAndEnd().ParseIntoArray(Parts, TEXT(","), true);
	if (Parts.Num() != 3)
	{
		FEarthTileDownloadStatus Status;
		Status.IsError = true;
		Status.ErrorMessage = "Invalid TileInfo String";
			
		StatusCallback(Status);
		return;
	}
	
	FMapTileQuery TileQuery = {};
	TileQuery.X = FCString::Atoi(*Parts[0].TrimStartAndEnd());
	TileQuery.Y = FCString::Atoi(*Parts[1].TrimStartAndEnd());
	TileQuery.Zoom = FCString::Atoi(*Parts[2].TrimStartAndEnd());
	TileQuery.ZoomInLevels = TileResolution;

	TileDownloadProgress = "Start downloading tiles";
	FMapUtils::DownloadTileSet(MapSource, TileQuery, [this, TileQuery, StatusCallback](TSharedPtr<FMapTileDownloadProgress> DownloadProgress, TSharedPtr<FMapTileResponse> TileResponseData)
	{
		TileDownloadProgress = FString::Printf(TEXT("Completed: %d/%d"), DownloadProgress->TilesDownloaded, DownloadProgress->TotalTiles);

		if (TileResponseData == nullptr)
		{
			return;
		}

		if (!TileResponseData->IsSuccess)
		{
			FEarthTileDownloadStatus Status;
			Status.IsError = true;
			Status.ErrorMessage = TileResponseData->ErrorMessage;
			
			StatusCallback(Status);
			return;
		}
		
		const int32 TilesPerRow = FMath::Pow(2.0, static_cast<float>(TileQuery.ZoomInLevels));
		const int32 PixelsPerRow = 512 * TilesPerRow;
	
		// This is important, otherwise the TileData will be garbage collected
		CurrentTileResponse = TileResponseData;
		UE_LOG(LogTemp, Warning, TEXT("Full Name: %s, Name: %s"), *GetFullName(), *GetName())

		if (G16Texture == nullptr || G16Texture->GetTextureWidth() != PixelsPerRow)
		{
			G16Texture = UG16Texture::Create(this, PixelsPerRow, "/Game/TerrainMagic/HeightMaps/Earth/", GetName());
		}
		
		G16Texture->UpdateAndCache(TileResponseData->HeightData.GetData(), [this, StatusCallback](UTexture2D* Texture)
		{
			FTerrainMagicThreading::RunOnGameThread([this, Texture, StatusCallback]()
			{
				HeightMap = Texture;
				CurrentTileResponse = nullptr;
				_Invalidate();

				if (StatusCallback != nullptr)
				{
					FEarthTileDownloadStatus Status;
					Status.IsError = false;
					
					StatusCallback(Status);
				}
			});
		});
	});
}

#if WITH_EDITOR
void AEarthLandscapeClip::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property->GetName() == "HeightRange")
	{
		if (HeightRange == HMHR_POSITIVE)
		{
			HeightMapRange.OutputMin = 0;
			HeightMapRange.OutputMax = 1;
		} else if (HeightRange == HMHR_POSITIVE_NEGATIVE)
		{
			HeightMapRange.OutputMin = -1;
			HeightMapRange.OutputMax = 1;
		}
	}
}
#endif
