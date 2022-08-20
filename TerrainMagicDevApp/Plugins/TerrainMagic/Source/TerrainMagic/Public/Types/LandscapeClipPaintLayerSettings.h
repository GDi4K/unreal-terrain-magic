// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "LandscapeClipPaintLayerSettings.generated.h"

UENUM(BlueprintType)
enum FLandscapeClipPaintLayerSettingsPaintMode
{
	LCPM_Original = 0 UMETA(DisplayName="Use Original"),
	LCPM_FILL = 1 UMETA(DisplayName="Fill Layer"),
	LCPM_CLEAR = 2 UMETA(DisplayName="Clear Layer"),
	LCPM_MASKS = 3 UMETA(DisplayName="Use Masks"),
};

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
struct FLandscapeClipPaintLayerSettingsHeightMask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RangeStart = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RangeEnd = 2000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Brightness = 1.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Contrast = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Invert = false;
};

USTRUCT(BlueprintType)
struct FLandscapeClipPaintLayerSettingsNormalMask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Direction = {0, 0, 1};
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Brightness = 1.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Contrast = 10.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float NormalSmoothMultiplier = 10.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Invert = false;
};

USTRUCT(BlueprintType)
struct FLandscapeClipPaintLayerSettingsLocationMask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector2D UVCenter = {0.5, 0.5};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ExpandScale = 0.2;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Brightness = 1.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Contrast = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Invert = false;
};

USTRUCT(BlueprintType)
struct FLandscapeClipPaintLayerSettingsEdgeNoiseMask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Scale = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Contrast = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RangeStart = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RangeEnd= 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Seed = {0, 0, 0};
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
	TEnumAsByte<FLandscapeClipPaintLayerSettingsPaintMode> PaintMode = LCPM_MASKS;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WeightBrightness = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WeightContrast = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="PaintMode == 3"))
	FLandscapeClipPaintLayerSettingsLocationMask LocationMask;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="PaintMode == 3"))
	FLandscapeClipPaintLayerSettingsNormalMask NormalMask;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="PaintMode == 3"))
	FLandscapeClipPaintLayerSettingsTextureMapMask TextureMapMask;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="PaintMode == 3"))
	FLandscapeClipPaintLayerSettingsHeightMask HeightMask;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="PaintMode == 3"))
	FLandscapeClipPaintLayerSettingsEdgeNoiseMask EdgeNoiseMask;
};