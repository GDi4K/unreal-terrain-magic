// Fill out your copyright notice in the Description page of Project Settings.


#include "HeightMapLandscapeClip.h"


// Sets default values
AHeightMapLandscapeClip::AHeightMapLandscapeClip()
{
	
}

UMaterial* AHeightMapLandscapeClip::GetSourceMaterial() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_Generic.M_TM_LandscapeClip_Generic";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AHeightMapLandscapeClip::GetMaterialParams()
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

