// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainMagicLiveHook.h"
#include "TerrainMagicLiveHookUse.generated.h"

UCLASS()
class TERRAINMAGICDEVAPP_API ATerrainMagicLiveHookUse : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrainMagicLiveHookUse();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ATerrainMagicLiveHook* Hook;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
