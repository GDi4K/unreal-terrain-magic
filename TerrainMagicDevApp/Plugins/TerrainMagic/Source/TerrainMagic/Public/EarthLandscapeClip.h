// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "Utils/MapBoxUtils.h"
#include "EarthLandscapeClip.generated.h"

UENUM()
enum EHeightMapTileWidth
{
	HMW_512 = 0 UMETA(DisplayName="512"),
	HMW_1024 = 1 UMETA(DisplayName="1024"),
	HMW_2048 = 2 UMETA(DisplayName="2048"),
	HMW_4096 = 3 UMETA(DisplayName="4096"),
};

UCLASS()
class TERRAINMAGIC_API AEarthLandscapeClip : public ALandscapeClip
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint16> CurrentHeightData;
	
	TSharedPtr<FMapBoxTileResponse> CurrentTileResponse = nullptr;
	bool HasTextureReloaded = false;

	void ReloadTextureIfNeeded();
	
public:
	// Sets default values for this actor's properties
	AEarthLandscapeClip();

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
	virtual void Tick(float DeltaSeconds) override;
	virtual UTexture* GetHeightMap() const override;
	virtual TArray<FLandscapeClipPaintLayerSettings> GetPaintLayerSettings() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", DuplicateTransient)
	int ZIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_COPY;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="01-General")
	UTexture2D* HeightMap = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	FString TileInfoString = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<EHeightMapTileWidth> TileResolution = HMW_512;
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="01-General")
	void DownloadTexture();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Radial Blur")
	int32 BlurDistance = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Radial Blur")
	int32 BlurDistanceSteps = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Radial Blur")
	int32 BlurRadialSteps = 8;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General|Modify Height")
	FTerrainMagicRemap HeightMapRange = {0, 1, 0, 1};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting")
	TArray<FLandscapeClipPaintLayerSettings> PaintLayerSettings = {};
};
