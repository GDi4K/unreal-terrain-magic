// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "GeoTiffLandscapeClip.h"
#include "Materials/Material.h"
#include "Utils/TerrainMagicThreading.h"


// Sets default values
AGeoTiffLandscapeClip::AGeoTiffLandscapeClip()
{
}

UMaterial* AGeoTiffLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_Generic.M_TM_LandscapeClip_Generic";
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

void AGeoTiffLandscapeClip::ApplyRawHeightData(uint32 TextureWidth, TArray<float> HeightData)
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

	TArray<uint16> G16HeightData;
	G16HeightData.SetNumUninitialized(TextureWidth * TextureWidth);

	for (size_t Index = 0; Index < HeightData.Num(); Index++)
	{
		G16HeightData[Index] = (HeightData[Index] - MinValue) * HeightRangeRatio;
	}

	G16Texture->UpdateOnly(G16HeightData.GetData(), [this](UTexture2D* Texture)
	{
		FTerrainMagicThreading::RunOnGameThread([this, Texture]()
		{
			HeightMap = Texture;
			_Invalidate();
		});
	});
}
