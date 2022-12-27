// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Clips/WeightMapLandscapeClip.h"
#include "Materials/Material.h"


// Sets default values
AWeightMapLandscapeClip::AWeightMapLandscapeClip()
{
}

UMaterial* AWeightMapLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/LandscapeClips/M_TM_LandscapeClip_HeightPassthrough.M_TM_LandscapeClip_HeightPassthrough";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AWeightMapLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;
	
	MaterialParams.Push({"SelectedFadeMode", static_cast<float>(FadeMode)});
	MaterialParams.Push({"FadeSaturation", FadeSaturation});
	MaterialParams.Push({"FadeMaskSpan", FadeMaskSpan});
	
	return MaterialParams;
}

int AWeightMapLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D AWeightMapLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void AWeightMapLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool AWeightMapLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void AWeightMapLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void AWeightMapLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int AWeightMapLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* AWeightMapLandscapeClip::GetHeightMap() const
{
	return nullptr;
}

TArray<FLandscapeClipPaintLayerSettings> AWeightMapLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}