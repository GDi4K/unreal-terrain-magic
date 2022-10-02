// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainMagicLiveHookUse.h"


// Sets default values
ATerrainMagicLiveHookUse::ATerrainMagicLiveHookUse()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Hook = NewObject<ATerrainMagicLiveHook>();
}

// Called when the game starts or when spawned
void ATerrainMagicLiveHookUse::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATerrainMagicLiveHookUse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

