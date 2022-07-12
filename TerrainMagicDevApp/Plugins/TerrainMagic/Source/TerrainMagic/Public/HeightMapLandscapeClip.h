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
	virtual int GetHeightMultiplier() const override;
	virtual FVector2D GetClipBaseSize() const override;

	UFUNCTION(CallInEditor, BlueprintCallable, Category="01-General")
	void Invalidate() { _Invalidate(); }

	UFUNCTION(CallInEditor, BlueprintCallable, Category="01-General")
	void ToggleOutline() { _ToggleOutline(); }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	UTexture* HeightMap = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_ADD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	FTerrainMagicRemap HeightMapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", meta=(ToolTip="A multiplier that will use with the following HeightMap range output values. This value doesn't scale with the clip actor's scaling factors."))
	int HeightMultiplier = 32000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float HeightSaturation = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Size of the HeightMap in meters before scaling"),  Category="01-General")
	FVector2D HeightMapBaseSize = {1000, 1000};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", meta=(DisplayName="----------------------------------------", EditCondition="false"))
	bool __Divider = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipFadeMode> FadeMode = LCF_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float FadeMaskSpan = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float FadeSaturation = 1.0;
};
