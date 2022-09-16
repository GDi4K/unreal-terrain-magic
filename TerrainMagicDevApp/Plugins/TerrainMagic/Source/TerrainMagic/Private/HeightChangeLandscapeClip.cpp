// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "HeightChangeLandscapeClip.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Kismet/KismetRenderingLibrary.h"


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
	RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), 2048, 2048, RTF_RGBA8);
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_RT_HeighChange_Landscape_Clip.M_RT_HeighChange_Landscape_Clip";
	UMaterial* SourceMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
	RenderTargetMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), SourceMaterial);
}

UMaterial* AHeightChangeLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_HeightChange.M_TM_LandscapeClip_HeightChange";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AHeightChangeLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;

	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), Cast<UTextureRenderTarget2D>(RenderTarget));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), Cast<UTextureRenderTarget2D>(RenderTarget), RenderTargetMaterial);
	
	MaterialParams.Push({"Texture", Texture});
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
	return RenderTarget;
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
	
	const int32 X = FCString::Atoi(*Parts[0].TrimStartAndEnd());;
	const int32 Y = FCString::Atoi(*Parts[1].TrimStartAndEnd());;
	const int32 Zoom = FCString::Atoi(*Parts[2].TrimStartAndEnd());;
	const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);

	UE_LOG(LogTemp, Warning, TEXT("URL: %s"), *ImageURL)
}
