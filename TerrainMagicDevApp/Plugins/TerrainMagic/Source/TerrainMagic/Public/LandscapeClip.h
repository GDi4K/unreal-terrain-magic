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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere)
	UTexture* HeightMap;

	UPROPERTY(EditAnywhere)
	FVector HeightMapRoot = {0, 0, 0};

	UPROPERTY(EditAnywhere)
	FVector2D HeightMapSize = {500, 500};
};
