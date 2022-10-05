// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "GameFramework/Actor.h"
#include "WeightMapLandscapeClip.generated.h"

UCLASS()
class TERRAINMAGIC_API AWeightMapLandscapeClip : public ALandscapeClip
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeightMapLandscapeClip();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", DuplicateTransient)
	int ZIndex = -1;
	
	UPROPERTY(BlueprintReadWrite, Category="01-General|Modify Height", meta=(ToolTip="A multiplier that will use with the following HeightMap range output values. This value doesn't scale with the clip actor's scaling factors."))
	int HeightMultiplier = 32000;
	
	UPROPERTY(BlueprintReadWrite, Category="01-General|Modify Height")
	float HeightSaturation = 1.0;

	UPROPERTY(EditAnywhere, Category="Actor", meta=(ToolTip="Size of the HeightMap in meters before scaling"))
	FVector2D HeightMapBaseSize = {1000, 1000};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting|Fading")
	TEnumAsByte<ELandscapeClipFadeMode> FadeMode = LCF_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting|Fading")
	float FadeMaskSpan = 0.2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting|Fading")
	float FadeSaturation = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting")
	TArray<FLandscapeClipPaintLayerSettings> PaintLayerSettings = {};
};
