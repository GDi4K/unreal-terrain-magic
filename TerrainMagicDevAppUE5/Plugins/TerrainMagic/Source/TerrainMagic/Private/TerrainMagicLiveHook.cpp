// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicLiveHook.h"


// Sets default values
ATerrainMagicLiveHook::ATerrainMagicLiveHook()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATerrainMagicLiveHook::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATerrainMagicLiveHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

