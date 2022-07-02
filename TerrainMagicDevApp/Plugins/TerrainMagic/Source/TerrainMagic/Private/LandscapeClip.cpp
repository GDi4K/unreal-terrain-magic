// Fill out your copyright notice in the Description page of Project Settings.


#include "LandscapeClip.h"


// Sets default values
ALandscapeClip::ALandscapeClip()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
}

// Called when the game starts or when spawned
void ALandscapeClip::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALandscapeClip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HeightMapRoot = GetActorLocation();
	HeightMapSizeInCM = HeightMapBaseSize * GetActorScale3D() * 100;
}

bool ALandscapeClip::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
		return true;
	return false;
}

