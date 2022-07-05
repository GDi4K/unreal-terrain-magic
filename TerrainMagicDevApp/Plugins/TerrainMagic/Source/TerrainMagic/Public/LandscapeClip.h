// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/FTerrainMagicRemap.h"
#include "LandscapeClip.generated.h"

UENUM(BlueprintType)
enum ELandscapeClipBlendMode
{
	LCB_ADD = 0 UMETA(DisplayName="Add"),
	LCB_AVERAGE = 1 UMETA(DisplayName="Average"),
	LCB_MIN = 2 UMETA(DisplayName="Min"),
	LCB_Max = 3 UMETA(DisplayName="Max"),
};

UCLASS()
class TERRAINMAGIC_API ALandscapeClip : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALandscapeClip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	bool bNeedsInvalidation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	class USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	UTexture* HeightMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Size of the HeightMap in meters before scaling"),  Category="01-General")
	FVector2D HeightMapBaseSize = {1000, 1000};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_ADD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", meta=(ToolTip="A multiplier that will use with the following HeightMap range output values. This value doesn't scale with the clip actor's scaling factors."))
	int HeightMultiplier = 32000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	FTerrainMagicRemap HeightMapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float HeightSaturation = 1.0;
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="01-General")
	void Invalidate();
	
	FVector HeightMapRoot = {0, 0, 0};
	FVector2D HeightMapSizeInCM = HeightMapBaseSize * 100;
};
