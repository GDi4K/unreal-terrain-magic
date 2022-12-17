// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "Utils/G16Texture.h"
#include "GeoTiffLandscapeClip.generated.h"

UENUM()
enum EGeoTiffTargetTextureResolution
{
	GTRES_SOURCE = 0 UMETA(DisplayName="Source Resolution"),
	GTRES_1024 = 1 UMETA(DisplayName="1024"),
	GTRES_2048 = 2 UMETA(DisplayName="2048"),
	GTRES_4096 = 3 UMETA(DisplayName="4096"),
	GTRES_8192 = 4 UMETA(DisplayName="8192"),
};

USTRUCT(BlueprintType)
struct FGeoTiffInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	FIntPoint TextureResolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	FVector2D Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	FVector2D PixelToMetersRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float Range;
};

UCLASS()
class TERRAINMAGIC_API AGeoTiffLandscapeClip : public ALandscapeClip
{
	GENERATED_BODY()

	UPROPERTY()
	UG16Texture* G16Texture = nullptr;
	
	UPROPERTY()
	FGeoTiffInfo GeoTiffInfo;

	bool HasTextureReloaded = false;
	void ReloadTextureIfNeeded();

public:
	// Sets default values for this actor's properties
	AGeoTiffLandscapeClip();

protected:
	

public:
	virtual UMaterial* GetSourceMaterialForHeight() const override;
	virtual TArray<FTerrainMagicMaterialParam> GetMaterialParams() override;
	virtual int GetHeightMultiplier() const override;
	virtual FVector2D GetClipBaseSize() const override;
	virtual void SetClipBaseSize(FVector2D BaseSize) override;
	virtual bool IsEnabled() const override;
	virtual void SetEnabled(bool bEnabledInput) override;
	virtual void SetZIndex(int Index) override;
	virtual int GetZIndex() const override;
	virtual UTexture* GetHeightMap() const override;
	virtual TArray<FLandscapeClipPaintLayerSettings> GetPaintLayerSettings() const override;
	void ApplyRawHeightData(FGeoTiffInfo GeoTiffInfo, uint32 TextureWidth, TArray<float> HeightData);
	virtual void Tick(float DeltaSeconds) override;
	int32 GetTargetResolution() const;
	FVector GetUpdatedLandscapeSize() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", DuplicateTransient)
	int ZIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_COPY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<EGeoTiffTargetTextureResolution> TargetResolution = GTRES_SOURCE;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="01-General")
	UTexture* HeightMap = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	FLandscapeClipTileRepositioning TileRepositioning;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Modify Height")
	FTerrainMagicRemap HeightMapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Modify Height", meta=(ToolTip="A multiplier that will use with the following HeightMap range output values. This value doesn't scale with the clip actor's scaling factors."))
	int HeightMultiplier = 32000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Modify Height")
	float HeightSaturation = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Modify Height", meta=(ToolTip="Size of the HeightMap in meters before scaling"))
	FVector2D HeightMapBaseSize = {1000, 1000};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Fading")
	TEnumAsByte<ELandscapeClipFadeMode> FadeMode = LCF_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Fading")
	float FadeMaskSpan = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Fading")
	float FadeSaturation = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Fading")
	TEnumAsByte<ELandscapeClipFadeMinimum> FadeMinimum = LCFM_REMAP_MIN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Radial Blur")
	int32 BlurDistance = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Radial Blur", meta=(ClampMin=0, ClampMax=100))
	int32 BlurDistanceSteps = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Radial Blur", meta=(ClampMin=0, ClampMax=100))
	int32 BlurRadialSteps = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise", meta=(ClampMin=0, ClampMax=1))
	float NoiseInfluence = 0.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise")
	float MountainNoiseScale = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise")
	float MountainNoiseDetails = 1.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise")
	float MountainNoiseContrast = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise")
	float MountainNoiseSharpness = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise")
	float MountainNoiseSeed = 0.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Mountain Noise")
	FVector2D MountainNoisePosition = {0, 0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting")
	TArray<FLandscapeClipPaintLayerSettings> PaintLayerSettings = {};
};
