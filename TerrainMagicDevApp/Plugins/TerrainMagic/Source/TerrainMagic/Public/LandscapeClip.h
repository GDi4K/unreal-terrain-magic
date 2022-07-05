// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	FVector HeightMapBaseSize = {1000, 1000, 1000};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_ADD;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float HeightMapOutputMin = -1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float HeightMapOutputMax = 1.0;
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="01-General")
	void Invalidate();
	
	FVector HeightMapRoot = {0, 0, 0};
	FVector HeightMapSizeInCM = HeightMapBaseSize * 100;
};
