// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicManager.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"


void ATerrainMagicManager::ProcessPaintLayerData(FName LayerName, UTextureRenderTarget2D* RenderTarget)
{
	FTerrainMagicPaintLayer* PaintLayer = FindOrGetPaintLayer(LayerName);
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	const FIntRect SampleRect = {0, 0, RenderTargetSize.X, RenderTargetSize.Y};
	
	const FReadSurfaceDataFlags ReadSurfaceDataFlags;
	
	// Read the render target surface data back.	
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		FTerrainMagicPaintLayer* PaintLayer;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
		TFunction<void()> Callback;
	};
	
	FReadSurfaceContext Context =
	{
		RenderTargetResource,
		PaintLayer,
		SampleRect,
		ReadSurfaceDataFlags,
	};
	
	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[Context, this](FRHICommandListImmediate& RHICmdList)
		{
			Context.PaintLayer->LayerData.Reset();
			RHICmdList.ReadSurfaceData(
				Context.SrcRenderTarget->GetRenderTargetTexture(),
				Context.Rect,
				Context.PaintLayer->LayerData,
				Context.Flags
			);
		});

	FlushRenderingCommands();
}

FTerrainMagicPaintLayer* ATerrainMagicManager::FindOrGetPaintLayer(FName LayerName)
{
	for (int Index=0; Index<PaintLayers.Num(); Index++)
	{
		FTerrainMagicPaintLayer* PaintLayer = &PaintLayers[Index];
		if (PaintLayer->LayerName == LayerName)
		{
			return PaintLayer;
		}
	}

	PaintLayers.Push({LayerName, {}});
	FTerrainMagicPaintLayer* PaintLayer = &PaintLayers[PaintLayers.Num() - 1];
	UE_LOG(LogTemp, Warning, TEXT("Adding Paint Layer: %s"), *PaintLayer->LayerName.ToString())

	return PaintLayer;
}

FTerrainMagicPaintLayer* ATerrainMagicManager::FindPaintLayer(FVector Location)
{
	FTerrainMagicPaintLayer* PaintLayer = nullptr;
	for (int Index=0; Index<PaintLayers.Num(); Index++)
	{
		PaintLayer = &PaintLayers[Index];
		UE_LOG(LogTemp, Warning, TEXT("Paint Layer Found: %s"), *PaintLayer->LayerName.ToString())
	}
	
	return PaintLayer;
}

// Sets default values
ATerrainMagicManager::ATerrainMagicManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	#if ENGINE_MAJOR_VERSION == 5
		bIsSpatiallyLoaded = false;
	#endif
#endif
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

void ATerrainMagicManager::Initialize(const FTransform InputLandscapeTransform, const FIntPoint InputLandscapeSize,
	const FIntPoint InputRenderTargetSize)
{
	LandscapeTransform = InputLandscapeTransform;
	LandscapeSize = InputLandscapeSize;
	RenderTargetSize = InputRenderTargetSize;
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

FName ATerrainMagicManager::FindLandscapePaintLayer(FVector Location)
{
	AActor* CurrentActor = UGameplayStatics::GetActorOfClass(GEngine->GetCurrentPlayWorld(), StaticClass());
	if (CurrentActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Actor Found"))
		return FName("No Actor Found");
	}

	ATerrainMagicManager* Manager = Cast<ATerrainMagicManager>(CurrentActor);
	
	const FTerrainMagicPaintLayer* PaintLayer = Manager->FindPaintLayer(Location);
	if (PaintLayer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Paint Layer"))
		return  FName("No Paint Layer");
	}

	UE_LOG(LogTemp, Warning, TEXT("Found Something"))
	return  PaintLayer->LayerName;
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

void ATerrainMagicManager::RenderWeightMap(FName LayerName, UMaterialInterface* Material)
{
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), WeightRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), WeightRenderTarget, Material);
	ProcessPaintLayerData(LayerName, WeightRenderTarget);
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

