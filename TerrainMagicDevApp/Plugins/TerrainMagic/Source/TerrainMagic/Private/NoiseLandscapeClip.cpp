// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "NoiseLandscapeClip.h"
#include "Materials/Material.h"


// Sets default values
ANoiseLandscapeClip::ANoiseLandscapeClip()
{
}

UMaterial* ANoiseLandscapeClip::GetSourceMaterial() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_Noise.M_TM_LandscapeClip_Noise";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> ANoiseLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;
	
	MaterialParams.Push({"Texture", HeightMap});
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

int ANoiseLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D ANoiseLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void ANoiseLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool ANoiseLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void ANoiseLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

UTexture* ANoiseLandscapeClip::GetHeightMap() const
{
	return HeightMap;
}

