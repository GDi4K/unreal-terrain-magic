// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "LandscapeClipPaintLayerSettings.generated.h"

USTRUCT(BlueprintType)
struct FLandscapeClipPaintLayerSettingsTextureMapMask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UTexture* Texture = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Brightness = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Contrast = 1.0;
};

USTRUCT(BlueprintType)
struct FLandscapeClipPaintLayerSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName PaintLayer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FName> AdditionalPaintLayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool FillLayer = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WeightBrightness = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WeightContrast = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLandscapeClipPaintLayerSettingsTextureMapMask TextureMapMask;
};