﻿// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "GameFramework/Actor.h"
#include "Utils/G16Texture.h"
#include "SplineLandscapeClip.generated.h"

UCLASS()
class TERRAINMAGIC_API ASplineLandscapeClip : public ALandscapeClip
{
	GENERATED_BODY()

	UPROPERTY()
	UG16Texture* G16Texture = nullptr;

	bool HasTextureReloaded = false;

	void ReloadTextureIfNeeded();

public:
	// Sets default values for this actor's properties
	ASplineLandscapeClip();

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
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(CallInEditor, Category="01-General")
	void Draw();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", DuplicateTransient)
	int ZIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_COPY;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="01-General")
	UTexture* HeightMap = nullptr;
	
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
	TEnumAsByte<ELandscapeClipFadeMinimum> FadeMinimum = LCFM_ZERO;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Layer Painting")
	TArray<FLandscapeClipPaintLayerSettings> PaintLayerSettings = {};
};