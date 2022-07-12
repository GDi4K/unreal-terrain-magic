// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicManager.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"


// Sets default values
ATerrainMagicManager::ATerrainMagicManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATerrainMagicManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATerrainMagicManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<ALandscapeClip*> ATerrainMagicManager::GetAllLandscapeClips() const
{
	TArray<AActor*> Actors;
	TArray<ALandscapeClip*> LandscapeClips;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscapeClip::StaticClass(), Actors);

	for (AActor* ClipActor: Actors)
	{
		ALandscapeClip* LandscapeClip = Cast<ALandscapeClip>(ClipActor);
		check(LandscapeClip)
		LandscapeClips.Add(LandscapeClip);
	}

	return LandscapeClips;
}

UTextureRenderTarget2D* ATerrainMagicManager::GetHeightMap()
{
	if (CachedHeightMap != nullptr)
	{
		return CachedHeightMap;
	}
	
	return HeightRenderTarget;
}

void ATerrainMagicManager::ShowClipOutlines() const
{
	for(ALandscapeClip* Clip: GetAllLandscapeClips())
	{
		Clip->bShowOutline = true;
	}
}

void ATerrainMagicManager::HideClipOutlines() const
{
	for(ALandscapeClip* Clip: GetAllLandscapeClips())
	{
		Clip->bShowOutline = false;
	}
}

void ATerrainMagicManager::CacheHeightMap(UTextureRenderTarget2D* HeightMap)
{
	CachedHeightMap = HeightMap;
}

void ATerrainMagicManager::ResetHeightMapCache()
{
	HeightRenderTarget = nullptr;
	CachedHeightMap = nullptr;
}

void ATerrainMagicManager::RenderHeightMap(UMaterialInterface* Material)
{
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), HeightRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), HeightRenderTarget, Material);
	HeightMapVersion += 1;
}

void ATerrainMagicManager::RenderWeightMap(UMaterialInterface* Material) const
{
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), WeightRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), WeightRenderTarget, Material);
}

UTextureRenderTarget2D* ATerrainMagicManager::EnsureHeightRenderTarget(const int Width, const int Height)
{
	if (HeightRenderTarget == nullptr)
	{
		HeightRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), Width, Height, RTF_RGBA8);
	}

	CachedHeightMap = nullptr;
	return HeightRenderTarget;
}

UTextureRenderTarget2D* ATerrainMagicManager::EnsureWeightRenderTarget(const int Width, const int Height)
{
	if (WeightRenderTarget == nullptr)
	{
		WeightRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), Width, Height, RTF_R8);
	}

	return WeightRenderTarget;
}

int ATerrainMagicManager::GetHeightMapVersion() const
{
	return HeightMapVersion;
}

