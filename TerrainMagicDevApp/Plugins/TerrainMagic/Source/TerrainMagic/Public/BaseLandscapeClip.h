// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "BaseLandscapeClip.generated.h"

UENUM(BlueprintType)
enum EBaseLandscapeClipBaseValue
{
	BLCBV_ZERO = 0 UMETA(DisplayName="0"),
	BLCBV_ABSOLUTE_MINIMUM = 1 UMETA(DisplayName="Absolute Minimum"),
	BLCBV_ABSOLUTE_MAXIMUM = 2 UMETA(DisplayName="Absolute Maximum"),
};

UCLASS()
class TERRAINMAGIC_API ABaseLandscapeClip : public ALandscapeClip
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseLandscapeClip();

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

#if WITH_EDITOR 
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", DuplicateTransient)
	int ZIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<EBaseLandscapeClipBaseValue> BaseValuePreset = BLCBV_ABSOLUTE_MINIMUM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float BaseValue = -32768.0;
};
