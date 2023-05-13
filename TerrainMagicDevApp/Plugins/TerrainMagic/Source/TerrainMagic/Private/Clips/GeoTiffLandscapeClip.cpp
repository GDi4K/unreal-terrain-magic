// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Clips/GeoTiffLandscapeClip.h"
#include "Materials/Material.h"
#include "Utils/TerrainMagicThreading.h"
#include "Engine/World.h"

void AGeoTiffLandscapeClip::ReloadTextureIfNeeded()
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

// Sets default values
AGeoTiffLandscapeClip::AGeoTiffLandscapeClip()
{
}

UMaterial* AGeoTiffLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/LandscapeClips/M_TM_LandscapeClip_Generic.M_TM_LandscapeClip_Generic";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AGeoTiffLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;
	
	MaterialParams.Push({"Texture", HeightMap});
	
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

	MaterialParams.Push({"TileRepositionZoomScale", TileRepositioning.Scale});
	MaterialParams.Push({"TileRepositionPanning", FVector(TileRepositioning.Panning, 0.0)});
	MaterialParams.Push({"TileRepositionRotation", TileRepositioning.Rotation});

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
	
	return MaterialParams;
}

int AGeoTiffLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D AGeoTiffLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void AGeoTiffLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool AGeoTiffLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void AGeoTiffLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void AGeoTiffLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int AGeoTiffLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* AGeoTiffLandscapeClip::GetHeightMap() const
{
	return HeightMap;
}

TArray<FLandscapeClipPaintLayerSettings> AGeoTiffLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}

void AGeoTiffLandscapeClip::ApplyRawHeightData(FGeoTiffInfo SourceGeoTiffInfo, uint32 TextureWidth, TArray<float> HeightData)
{
	if (!IsValid(G16Texture) || G16Texture->GetTextureWidth() != TextureWidth)
	{
		G16Texture = UG16Texture::Create(this, TextureWidth, "/Game/TerrainMagic/HeightMaps/GeoTiff/", GetName());
	}

	const float MinValue = FMath::Min(HeightData);
	const float MaxValue = FMath::Max(HeightData);
	const float Range = MaxValue - MinValue;
	const int32 Max16BitValue = FMath::Pow(2.0, 16.0) - 1;
	const float HeightRangeRatio = Max16BitValue / Range;

	SourceGeoTiffInfo.Range = Range;

	TArray<uint16> G16HeightData;
	G16HeightData.SetNumUninitialized(TextureWidth * TextureWidth);

	for (size_t Index = 0; Index < HeightData.Num(); Index++)
	{
		G16HeightData[Index] = (HeightData[Index] - MinValue) * HeightRangeRatio;
	}

	G16Texture->UpdateAndCache(G16HeightData.GetData(), [this, SourceGeoTiffInfo](UTexture2D* Texture)
	{
		FTerrainMagicThreading::RunOnGameThread([this, SourceGeoTiffInfo, Texture]()
		{
			GeoTiffInfo = SourceGeoTiffInfo;
			HeightMap = Texture;
			_Invalidate();
		});
	});
}

void AGeoTiffLandscapeClip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}

	ReloadTextureIfNeeded();
}

int32 AGeoTiffLandscapeClip::GetTargetResolution() const
{
	switch (TargetResolution)
	{
	case GTRES_SOURCE:
		return -1;

	case GTRES_1024:
		return 1024;

	case GTRES_2048:
		return 2048;

	case GTRES_4096:
		return 4096;
		
	case GTRES_8192:
		return 8192;
		
	default:
		return -1;
	}
}

FVector AGeoTiffLandscapeClip::GetUpdatedLandscapeSize() const
{
	const float GeoTiffWidth = GeoTiffInfo.TextureResolution.X * GeoTiffInfo.PixelToMetersRatio.X;
	float WidthScale = GeoTiffWidth / LandscapeSize.X * 100;
	float HeightScale = GeoTiffInfo.Range / 512 * 100;
	return {WidthScale, WidthScale, HeightScale};
}
