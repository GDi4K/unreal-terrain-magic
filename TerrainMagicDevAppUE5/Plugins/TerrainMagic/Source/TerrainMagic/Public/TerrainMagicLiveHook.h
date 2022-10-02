// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainMagicLiveHook.generated.h"

UCLASS()
class TERRAINMAGIC_API ATerrainMagicLiveHook : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrainMagicLiveHook();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
