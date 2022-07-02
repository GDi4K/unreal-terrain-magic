// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LandscapeClip.generated.h"

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

	UFUNCTION(BlueprintCallable, CallInEditor, Category="01-General")
	void Invalidate();
	
	FVector HeightMapRoot = {0, 0, 0};
	FVector HeightMapSizeInCM = HeightMapBaseSize * 100;
};
