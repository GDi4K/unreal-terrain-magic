// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Clips/NoiseLandscapeClip.h"
#include "Materials/Material.h"


// Sets default values
ANoiseLandscapeClip::ANoiseLandscapeClip()
{
}

UMaterial* ANoiseLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/LandscapeClips/M_TM_LandscapeClip_Noise.M_TM_LandscapeClip_Noise";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> ANoiseLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;

	MaterialParams.Push({"MountainNoiseScale", MountainNoiseScale});
	MaterialParams.Push({"MountainNoiseDetails", MountainNoiseDetails});
	MaterialParams.Push({"MountainNoiseContrast", MountainNoiseContrast});
	MaterialParams.Push({"MountainNoiseSharpness", MountainNoiseSharpness});
	MaterialParams.Push({"MountainNoiseSeed", MountainNoiseSeed});
	MaterialParams.Push({"MountainNoisePosition", FVector(MountainNoisePosition.X, MountainNoisePosition.Y, 0)});
	
	MaterialParams.Push({"HeightMultiplier", static_cast<float>(HeightMultiplier)});
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

void ANoiseLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int ANoiseLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* ANoiseLandscapeClip::GetHeightMap() const
{
	return nullptr;
}

TArray<FLandscapeClipPaintLayerSettings> ANoiseLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}

