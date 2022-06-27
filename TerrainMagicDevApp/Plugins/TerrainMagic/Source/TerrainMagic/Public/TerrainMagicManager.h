// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TerrainMagicManager.generated.h"

UCLASS()
class TERRAINMAGIC_API ATerrainMagicManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrainMagicManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic")
	UTextureRenderTarget2D* HeightRenderTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic")
	UTextureRenderTarget2D* WeightRenderTarget;
	
	UTextureRenderTarget2D* EnsureHeightRenderTarget(const int Width, const int Height);
	UTextureRenderTarget2D* EnsureWeightRenderTarget(const int Width, const int Height);
};
