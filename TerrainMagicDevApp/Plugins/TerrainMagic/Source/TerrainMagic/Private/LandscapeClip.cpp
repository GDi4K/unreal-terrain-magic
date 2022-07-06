// Fill out your copyright notice in the Description page of Project Settings.


#include "LandscapeClip.h"


// Sets default values
ALandscapeClip::ALandscapeClip()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("OutlineComponent"));
	OutlineComponent->SetLineThickness(500.0);
	OutlineComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepWorldTransform);
	
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
	HeightMapSizeInCM = HeightMapBaseSize * FVector2D(GetActorScale3D()) * 100;

	OutlineComponent->SetBoxExtent({
		HeightMapSizeInCM.X/2,
		HeightMapSizeInCM.Y/2,
		static_cast<float>(HeightMultiplier/2.0)
	});
}

bool ALandscapeClip::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
		return true;
	return false;
}

void ALandscapeClip::Invalidate()
{
	bNeedsInvalidation = true;
}

void ALandscapeClip::ToggleOutline()
{
	bShowOutline = !bShowOutline;
	OutlineComponent->SetVisibility(bShowOutline);
}

