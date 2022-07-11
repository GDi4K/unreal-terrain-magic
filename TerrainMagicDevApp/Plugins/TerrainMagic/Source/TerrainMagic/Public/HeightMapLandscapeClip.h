// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "GameFramework/Actor.h"
#include "HeightMapLandscapeClip.generated.h"

UCLASS()
class TERRAINMAGIC_API AHeightMapLandscapeClip : public ALandscapeClip
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHeightMapLandscapeClip();

protected:
	

public:
	virtual UMaterial* GetSourceMaterial() const override;
	virtual TArray<FTerrainMagicMaterialParam> GetMaterialParams() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	UTexture* HeightMap = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_ADD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	FTerrainMagicRemap HeightMapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float HeightSaturation = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Modifiers")
	TEnumAsByte<ELandscapeClipFadeMode> FadeMode = LCF_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Modifiers")
	float FadeMaskSpan = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Modifiers")
	float FadeSaturation = 1.0;
};
