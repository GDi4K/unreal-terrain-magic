// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicManager.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine.h"

void ATerrainMagicManager::ProcessPaintLayerData(FName LayerName, UTextureRenderTarget2D* RenderTarget)
{
	const FTimespan TimeDiffFromLastReset = FDateTime::Now() - LastPaintLayerResetTime;
	if (TimeDiffFromLastReset.GetTotalSeconds() > 5.0)
	{
		return;
	}
	
	int PaintLayerIndex = PaintLayerNames.Find(LayerName);
	if (PaintLayerIndex == INDEX_NONE)
	{
		PaintLayerNames.Push(LayerName);
		PaintLayerIndex = PaintLayerNames.Num() - 1;
	}
	
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	const FIntRect SampleRect = {0, 0, RenderTargetSize.X, RenderTargetSize.Y};
	
	const FReadSurfaceDataFlags ReadSurfaceDataFlags;
	
	// Read the render target surface data back.	
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* Data;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
		TFunction<void()> Callback;
	};

	TArray<FColor> RenderTargetData;
	
	FReadSurfaceContext Context =
	{
		RenderTargetResource,
		&RenderTargetData,
		SampleRect,
		ReadSurfaceDataFlags,
	};
	
	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[Context, this](FRHICommandListImmediate& RHICmdList)
		{
			Context.Data->Reset();
			RHICmdList.ReadSurfaceData(
				Context.SrcRenderTarget->GetRenderTargetTexture(),
				Context.Rect,
				*Context.Data,
				Context.Flags
			);
		});
	
	FlushRenderingCommands();
	
	for (int Index =0; Index<RenderTargetData.Num(); Index++)
	{
		const FColor Pixel = RenderTargetData[Index];
		const float Coverage = Pixel.R / 255.0;
		if (Coverage >= PaintLayerActivationThreshold)
		{
			PaintLayerData[Index] = PaintLayerIndex + 1;
		}
	}
}

FTerrainMagicPaintLayerResult ATerrainMagicManager::FindPaintLayer(FVector Location)
{
	const FVector RelativeLocation = Location - LandscapeTransform.GetLocation();
	const FVector RelativeLocationAfterScaleDown = RelativeLocation / LandscapeTransform.GetScale3D();
	const FIntPoint RelativeLocationToPixels = {
		FMath::FloorToInt(RelativeLocationAfterScaleDown.X),
		FMath::FloorToInt(RelativeLocationAfterScaleDown.Y),
	};

	const int PixelIndex = RelativeLocationToPixels.Y * RenderTargetSize.X + RelativeLocationToPixels.X;

	if (PixelIndex >= PaintLayerData.Num())
	{
		return { false };
	}
	
	const int ActualPaintLayer = PaintLayerData[PixelIndex] - 1;

	if (ActualPaintLayer == -1)
	{
		return {false};
	}
	
	return { true, PaintLayerNames[ActualPaintLayer] };
}

void ATerrainMagicManager::PopulateLastZIndex()
{
	const TArray<ALandscapeClip*> LandscapeClips = GetAllLandscapeClips();
	if (LandscapeClips.Num() == 0)
	{
		LastZIndex = -1;
	} else
	{
		LastZIndex = LandscapeClips[LandscapeClips.Num() -1]->GetZIndex();
	}
}

void ATerrainMagicManager::HandleInvalidateKeyEvent()
{
	UE_LOG(LogTemp, Warning, TEXT("It's Invalidated!!!"))
}

void ATerrainMagicManager::SetupInputHandling()
{
	// Initialize our component
	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();
	if (InputComponent)
	{
		InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ATerrainMagicManager::HandleInvalidateKeyEvent);
		// Bind inputs here
		// InputComponent->BindAction("Jump", IE_Pressed, this, &AUnrealisticPawn::Jump);
		// etc...

		// Now hook up our InputComponent to one in a Player
		// Controller, so that input flows down to us
		EnableInput(GetWorld()->GetFirstPlayerController());
		UE_LOG(LogTemp, Warning, TEXT("TerrainMagic Input Handling is done!"))
	}    
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

	LandscapeClips.Sort([](const ALandscapeClip& ip1, const ALandscapeClip& ip2) {
		return  ip1.GetZIndex() < ip2.GetZIndex();
	});

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

FTerrainMagicPaintLayerResult ATerrainMagicManager::FindLandscapePaintLayer(FVector Location)
{
	AActor* CurrentActor = UGameplayStatics::GetActorOfClass(GEngine->GetCurrentPlayWorld(), StaticClass());
	if (CurrentActor == nullptr)
	{
		return {false};
	}

	ATerrainMagicManager* Manager = Cast<ATerrainMagicManager>(CurrentActor);
	return Manager->FindPaintLayer(Location);
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

void ATerrainMagicManager::RenderWeightMap(FName LayerName, UMaterialInterface* Material) const
{
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), WeightRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), WeightRenderTarget, Material);
}

void ATerrainMagicManager::ResetPaintLayerData(const float ActivationThreshold)
{
	PaintLayerActivationThreshold = ActivationThreshold;
	PaintLayerNames = {};
	PaintLayerData = {};
	PaintLayerData.SetNumZeroed(RenderTargetSize.X * RenderTargetSize.Y);
	LastPaintLayerResetTime = FDateTime::Now();
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

int ATerrainMagicManager::GetNextLandscapeClipZIndex()
{
	// Initialize ZIndex for the first time
	if (LastZIndex == -2002)
	{
		PopulateLastZIndex();
	}
	
	LastZIndex += 1;
	return LastZIndex;
}

ATerrainMagicManager* ATerrainMagicManager::EnsureManager(UWorld* World)
{
	AActor* CurrentActor = UGameplayStatics::GetActorOfClass(World, StaticClass());
	if (CurrentActor == nullptr)
	{
		AActor* SpawnedActor = World->SpawnActor(StaticClass());
		return Cast<ATerrainMagicManager>(SpawnedActor);
	}

	return Cast<ATerrainMagicManager>(CurrentActor);
}

