// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "HeightChangeLandscapeClip.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Utils/MapBoxUtils.h"


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
AHeightChangeLandscapeClip::AHeightChangeLandscapeClip()
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_RT_HeighChange_Landscape_Clip.M_RT_HeighChange_Landscape_Clip";
}

UMaterial* AHeightChangeLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_HeightChange.M_TM_LandscapeClip_HeightChange";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AHeightChangeLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;
	
	MaterialParams.Push({"Texture", HeightMap});
	MaterialParams.Push({"BlurDistance", static_cast<float>(BlurDistance)});
	MaterialParams.Push({"BlurDistanceSteps", static_cast<float>(BlurDistanceSteps)});
	MaterialParams.Push({"BlurRadialSteps", static_cast<float>(BlurRadialSteps)});
	
	MaterialParams.Push({"HeightMultiplier", static_cast<float>(HeightMultiplier)});
	MaterialParams.Push({"SelectedBlendMode", static_cast<float>(BlendMode)});

	MaterialParams.Push({"HeightMapInputMin", HeightMapRange.InputMin});
	MaterialParams.Push({"HeightMapInputMax", HeightMapRange.InputMax});
	MaterialParams.Push({"HeightMapOutputMin", HeightMapRange.OutputMin});
	MaterialParams.Push({"HeightMapOutputMax", HeightMapRange.OutputMax});

	MaterialParams.Push({"HeightSaturation", HeightSaturation});

	MaterialParams.Push({"SelectedFadeMode", static_cast<float>(FadeMode)});
	MaterialParams.Push({"FadeSaturation", FadeSaturation});
	MaterialParams.Push({"FadeMaskSpan", FadeMaskSpan});
	
	return MaterialParams;
}

int AHeightChangeLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D AHeightChangeLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void AHeightChangeLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool AHeightChangeLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void AHeightChangeLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void AHeightChangeLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int AHeightChangeLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* AHeightChangeLandscapeClip::GetHeightMap() const
{
	return HeightMap;
}

TArray<FLandscapeClipPaintLayerSettings> AHeightChangeLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}

void AHeightChangeLandscapeClip::DownloadTexture()
{
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";

	TArray<FString> Parts;
	TileInfoString.TrimStartAndEnd().ParseIntoArray(Parts, TEXT(","), true);
	checkf(Parts.Num() == 3, TEXT("TileInfo text is invalid!"))
	
	FMapBoxTileQuery TileQuery = {};
	TileQuery.X = FCString::Atoi(*Parts[0].TrimStartAndEnd());
	TileQuery.Y = FCString::Atoi(*Parts[1].TrimStartAndEnd());
	TileQuery.Zoom = FCString::Atoi(*Parts[2].TrimStartAndEnd());
	TileQuery.ZoomInLevels = ZoomInLevel;

	FMapBoxUtils::DownloadTileSet(TileQuery, [this, TileQuery](TSharedPtr<FMapBoxTileResponse> TileData)
	{
		CurrentTileResponse = TileData;
		UE_LOG(LogTemp, Warning, TEXT("Tile Downloaded: %d"), TileData->HeightData.Num())
		const int32 TilesPerRow = FMath::Pow(2, TileQuery.ZoomInLevels);
		const int32 PixelsPerRow = 512 * TilesPerRow;
		
		// TODO: Create a Serializable Texture
		HeightMap = UTexture2D::CreateTransient(PixelsPerRow, PixelsPerRow, PF_G16);
		HeightMap->CompressionSettings = TC_VectorDisplacementmap;
		HeightMap->SRGB = 0;
		HeightMap->AddToRoot();
		HeightMap->Filter = TF_Bilinear;
		HeightMap->UpdateResource();

		const FUpdateTextureRegion2D* UpdateRegionNew = new FUpdateTextureRegion2D(0, 0, 0, 0, PixelsPerRow, PixelsPerRow);
		constexpr int32 BytesPerPixel = 2;
		const int32 BytesPerRow = PixelsPerRow * BytesPerPixel;

		uint16* SourceDataPtr = TileData->HeightData.GetData();
		uint8* SourceByteDataPtr = reinterpret_cast<uint8*>(SourceDataPtr);
		HeightMap->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), UpdateRegionNew,
								  static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), SourceByteDataPtr,
								  [this](uint8*, const FUpdateTextureRegion2D*)
								  {
									  CurrentTileResponse = nullptr;
								  });
	});
}