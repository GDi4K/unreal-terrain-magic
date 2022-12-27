// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicManager.h"

#include "Clips/BaseLandscapeClip.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine.h"
#include "Landscape.h"
#include "Kismet/KismetMaterialLibrary.h"

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
		// This conversion need for the 5.1 support
		(int32)FMath::FloorToInt(RelativeLocationAfterScaleDown.X),
		(int32)FMath::FloorToInt(RelativeLocationAfterScaleDown.Y),
	};

	const size_t PixelIndex = RelativeLocationToPixels.Y * RenderTargetSize.X + RelativeLocationToPixels.X;

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

void ATerrainMagicManager::TogglePreview()
{
	bShowPreviewMesh = !bShowPreviewMesh;
	ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(GetWorld(), ALandscape::StaticClass()));
	
	if (bShowPreviewMesh)
	{
		LandscapeLocationBeforePreview = Landscape->GetActorLocation();
		Landscape->SetActorLocation({
			LandscapeLocationBeforePreview.X,
			LandscapeLocationBeforePreview.Y,
			-100000
		});
	} else
	{
		Landscape->SetActorLocation(LandscapeLocationBeforePreview);
	}
	
	InvalidateClips();
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

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	
	// Add Preview Mesh Component
	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	PreviewMeshComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Assign the PreviewMaterial
	const FName PlaneMeshLocation = "/TerrainMagic/Core/Meshes/SM_TM_PreviewMesh_1000.SM_TM_PreviewMesh_1000";
	UStaticMesh* PlaneMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *PlaneMeshLocation.ToString()));
	PreviewMeshComponent->SetStaticMesh(PlaneMesh);

	// Create the CopyRTMaterial
	const FName CopyRTMaterialPath = "/TerrainMagic/Core/Materials/Utils/M_Copy_RT.M_Copy_RT";
	UMaterial* MaterialSource = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *CopyRTMaterialPath.ToString()));
	CopyRTMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), MaterialSource);
}

// Called when the game starts or when spawned
void ATerrainMagicManager::BeginPlay()
{
	Super::BeginPlay();
}

bool ATerrainMagicManager::CanRenderPreviewMesh() const
{
	if (!bShowPreviewMesh)
	{
		return false;
	}

	if (bClipsAreDirty)
	{
		return true;
	}

	return false;
}

// Called every frame
void ATerrainMagicManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PreviewMaterial == nullptr)
	{
		const FName PreviewMaterialPath = "/TerrainMagic/Core/Materials/M_TerrainMagicPreview.M_TerrainMagicPreview";
		UMaterial* PreviewMaterialSource = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *PreviewMaterialPath.ToString()));
		PreviewMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), PreviewMaterialSource);
		PreviewMeshComponent->SetMaterial(0, PreviewMaterial);
	}

	if (CanRenderPreviewMesh())
	{
		bClipsAreDirty = false;
		RenderLandscapeClipsHeightMap(BaseRenderTargetForHeight);
	}

	if (bShowPreviewMesh)
	{
		const FVector LandscapeScale = LandscapeTransform.GetScale3D();
		const FVector2D LandscapeSizeInCm = {
			LandscapeSize.X * LandscapeScale.X,
			LandscapeSize.Y * LandscapeScale.Y
		};
		const FVector2D Landscape2DLocation = FVector2D(LandscapeLocationBeforePreview);
		const FVector2D LandscapeMid = Landscape2DLocation + LandscapeSizeInCm/2.0;
		
		PreviewMeshComponent->SetRelativeScale3D(FVector(
			LandscapeSizeInCm.X / 1000.0f,
			LandscapeSizeInCm.Y / 1000.0f,
			0.1
		));
		PreviewMeshComponent->SetRelativeLocation({
			LandscapeMid.X,
			LandscapeMid.Y,
			LandscapeLocationBeforePreview.Z
		});
		PreviewMeshComponent->SetVisibility(true);
	} else
	{
		PreviewMeshComponent->SetVisibility(false);
	}

}

bool ATerrainMagicManager::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
		return true;
	return false;
}

#if WITH_EDITOR
void ATerrainMagicManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ClipsAreDirty();
}
#endif

void ATerrainMagicManager::Initialize(const FTransform InputLandscapeTransform, const FIntPoint InputLandscapeSize,
                                      const FIntPoint InputRenderTargetSize)
{
	LandscapeTransform = InputLandscapeTransform;
	LandscapeSize = InputLandscapeSize;
	RenderTargetSize = InputRenderTargetSize;
}

ALandscapeClip* HandleLandscapeClipSoloProcess(TArray<ALandscapeClip*> LandscapeClips)
{
	ALandscapeClip* SoloClip = nullptr;
	for (ALandscapeClip* Clip: LandscapeClips)
	{
		if (Clip->SoloAction != LCSA_NONE)
		{
			if (SoloClip == nullptr)
			{
				SoloClip = Clip;
				continue;
			}

			if (Clip->SoloTime > SoloClip->SoloTime)
			{
				SoloClip = Clip;
			}
		}
	}

	if (SoloClip != nullptr)
	{
		const TEnumAsByte<ELandscapeClipSoloAction> CurrentSoloAction = SoloClip->SoloAction;
		for (ALandscapeClip* Clip: LandscapeClips)
		{
			if (CurrentSoloAction == LCSA_UNSOLO)
			{
				Clip->SetEnabled(true);
				Clip->SoloAction = LCSA_NONE;
				Clip->SoloTime = 0;
			} else if (CurrentSoloAction == LCSA_SOLO)
			{
				Clip->SetEnabled(SoloClip==Clip);
				if (SoloClip != Clip)
				{
					Clip->SoloAction = LCSA_NONE;
					Clip->SoloTime = 0;
				}
			}
		}
	}

	return SoloClip;
}


UTextureRenderTarget2D* ATerrainMagicManager::RenderLandscapeClipsHeightMap(UTextureRenderTarget2D* InputHeightMap)
{
	if (!bShowPreviewMesh)
	{
		// Copy a version of the BaseHeightMap where we use it in the preview mode to invoke this function
		if (BaseRenderTargetForHeight == nullptr)
		{
			BaseRenderTargetForHeight = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RenderTargetSize.X, RenderTargetSize.Y, RTF_RGBA8);
		}
		
		CopyRTMaterial->SetTextureParameterValue("RenderTarget", InputHeightMap);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BaseRenderTargetForHeight);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BaseRenderTargetForHeight, CopyRTMaterial);
	}
	
	EnsureHeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), HeightRenderTarget);
	
	TArray<ALandscapeClip*> LandscapeClips = GetAllLandscapeClips();

	if (LandscapeClips.Num() == 0)
	{
		return InputHeightMap;
	}

	HandleLandscapeClipSoloProcess(LandscapeClips);

	if (BufferRenderTargetForHeight == nullptr)
	{
		BufferRenderTargetForHeight = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RenderTargetSize.X, RenderTargetSize.Y, RTF_RGBA8);
	}

	// Copy the Input HeightMap at the beginning
	CopyRTMaterial->SetTextureParameterValue("RenderTarget", InputHeightMap);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTargetForHeight);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTargetForHeight, CopyRTMaterial);
	
	for (ALandscapeClip* LandscapeClip: LandscapeClips)
	{
		// Set landscape information where actor can use to position it relative to the landscape
		LandscapeClip->LandscapeLocation = bShowPreviewMesh? LandscapeLocationBeforePreview : LandscapeTransform.GetLocation();
		LandscapeClip->LandscapeScale = LandscapeTransform.GetScale3D();
		LandscapeClip->LandscapeSize = FVector(LandscapeSize.X, LandscapeSize.Y, 0);
		
		if (!LandscapeClip->IsEnabled())
		{
			continue;
		}
		
		// Apply Params
		TArray<FTerrainMagicMaterialParam> Params = {};
		
		Params.Push({"HeightRT", BufferRenderTargetForHeight});
		Params.Push({"LandscapeLocation", LandscapeTransform.GetLocation()});
		Params.Push({"LandscapeScale", LandscapeTransform.GetScale3D()});
		Params.Push({"LandscapeSize", FVector(LandscapeSize.X, LandscapeSize.Y, 0)});
		Params.Push({"RenderTargetSize", FVector(RenderTargetSize.X, RenderTargetSize.Y, 0)});

		Params.Push({"ClipRoot", LandscapeClip->HeightMapRoot});
		Params.Push({"ClipSizeInCM", FVector(LandscapeClip->HeightMapSizeInCM.X, LandscapeClip->HeightMapSizeInCM.Y, 0)});
		Params.Push({"ClipRotationInDegrees", LandscapeClip->GetActorRotation().Euler()});

		LandscapeClip->ApplyMaterialParamsForHeight(Params);
		
		// Render the Clip
		RenderHeightMap(LandscapeClip->MaterialForHeight);

		// Copy the NewHeightMap to the Buffer
		CopyRTMaterial->SetTextureParameterValue("RenderTarget", HeightRenderTarget);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTargetForHeight);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTargetForHeight, CopyRTMaterial);
	}

	// This will prevent us from a memory leak where holding a render target causes some issues.
	// It happens when reloading the map inside the Editor
	CopyRTMaterial->ClearParameterValues();
	
	return HeightRenderTarget;
}

UTextureRenderTarget2D* ATerrainMagicManager::RenderLandscapeClipsWeightMap(FName LayerName,
	UTextureRenderTarget2D* InputWeightMap)
{
	EnsureWeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), WeightRenderTarget);
	
	TArray<ALandscapeClip*> LandscapeClips = GetAllLandscapeClips();

	if (LandscapeClips.Num() == 0)
	{
		return InputWeightMap;
	}

	HandleLandscapeClipSoloProcess(LandscapeClips);

	if (BufferRenderTargetForWeight == nullptr)
	{
		BufferRenderTargetForWeight = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RenderTargetSize.X, RenderTargetSize.Y, RTF_RGBA8);
	}

	// Copy the Input HeightMap at the beginning
	CopyRTMaterial->SetTextureParameterValue("RenderTarget", InputWeightMap);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTargetForWeight);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTargetForWeight, CopyRTMaterial);
	
	for (ALandscapeClip* LandscapeClip: LandscapeClips)
	{
		// Set landscape information where actor can use to position it relative to the landscape
		LandscapeClip->LandscapeLocation = LandscapeTransform.GetLocation();
		LandscapeClip->LandscapeScale = LandscapeTransform.GetScale3D();
		LandscapeClip->LandscapeSize = FVector(LandscapeSize.X, LandscapeSize.Y, 0);
		
		if (!LandscapeClip->IsEnabled())
		{
			continue;
		}
	
		// Check whether this clip needs to run for this paint layer
		FLandscapeClipPaintLayerSettings RelatedPaintLayerSettings;
		bool bFoundPaintLayer = false;
		for (FLandscapeClipPaintLayerSettings PaintLayerSettings: LandscapeClip->GetPaintLayerSettings())
		{
			if (PaintLayerSettings.PaintLayer == LayerName)
			{
				bFoundPaintLayer = true;
				RelatedPaintLayerSettings = PaintLayerSettings;
				break;
			}
	
			for (FName PaintLayer: PaintLayerSettings.AdditionalPaintLayers)
			{
				if (PaintLayer == LayerName)
				{
					bFoundPaintLayer = true;
					RelatedPaintLayerSettings = PaintLayerSettings;
					break;
				}
			}
	
			if (bFoundPaintLayer)
			{
				break;
			}
		}
	
		if (!bFoundPaintLayer)
		{
			continue;
		}
		
		// Apply Params
		TArray<FTerrainMagicMaterialParam> Params = {};
		
		Params.Push({"HeightRT", GetHeightMap()});
		Params.Push({"LandscapeLocation", LandscapeTransform.GetLocation()});
		Params.Push({"LandscapeScale", LandscapeTransform.GetScale3D()});
		Params.Push({"LandscapeSize", FVector(LandscapeSize.X, LandscapeSize.Y, 0)});
		Params.Push({"RenderTargetSize", FVector(RenderTargetSize.X, RenderTargetSize.Y, 0)});
	
		Params.Push({"ClipRoot", LandscapeClip->HeightMapRoot});
		Params.Push({"ClipSizeInCM", FVector(LandscapeClip->HeightMapSizeInCM.X, LandscapeClip->HeightMapSizeInCM.Y, 0)});
		Params.Push({"ClipRotationInDegrees", LandscapeClip->GetActorRotation().Euler()});
	
		Params.Push({"WeightRT", BufferRenderTargetForWeight});
	
		LandscapeClip->ApplyMaterialParamsForWeight(Params, RelatedPaintLayerSettings);
		
		// Render the Clip
		RenderWeightMap(LayerName, LandscapeClip->MaterialForWeight);
	
		// Copy the NewHeightMap to the Buffer
		CopyRTMaterial->SetTextureParameterValue("RenderTarget", WeightRenderTarget);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTargetForWeight);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTargetForWeight, CopyRTMaterial);
	}

	// This will prevent us from a memory leak where holding a render target causes some issues.
	// It happens when reloading the map inside the Editor
	CopyRTMaterial->ClearParameterValues();
	
	return WeightRenderTarget;
}

void ATerrainMagicManager::ClipsAreDirty()
{
	bClipsAreDirty = true;
}

void ATerrainMagicManager::InvalidateClips()
{
	bNeedToInvalidateClips = true;
}

bool ATerrainMagicManager::NeedToInvalidateClips()
{
	if (bNeedToInvalidateClips)
	{
		bNeedToInvalidateClips = false;
		return true;
	}

	return false;
}

TArray<ALandscapeClip*> ATerrainMagicManager::GetAllLandscapeClips() const
{
	TArray<AActor*> Actors;
	TArray<ALandscapeClip*> LandscapeClips;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscapeClip::StaticClass(), Actors);

	ALandscapeClip* BaseLandscapeClip = nullptr;

	for (AActor* ClipActor: Actors)
	{
		ALandscapeClip* LandscapeClip = Cast<ALandscapeClip>(ClipActor);
		check(LandscapeClip)
		ABaseLandscapeClip* LandscapeClipAsBaseClip = Cast<ABaseLandscapeClip>(LandscapeClip);
		if (IsValid(LandscapeClipAsBaseClip))
		{
			BaseLandscapeClip = LandscapeClipAsBaseClip;
		} else
		{
			LandscapeClips.Add(LandscapeClip);
		}
	}

	LandscapeClips.Sort([](const ALandscapeClip& ip1, const ALandscapeClip& ip2) {
		return  ip1.GetZIndex() < ip2.GetZIndex();
	});

	if(IsValid(BaseLandscapeClip))
	{
		LandscapeClips.Insert(BaseLandscapeClip, 0);
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
	if (PreviewMaterial != nullptr)
	{
		PreviewMaterial->SetTextureParameterValue("HeightRT", HeightRenderTarget);
		PreviewMaterial->SetVectorParameterValue("LandscapeLocation", LandscapeLocationBeforePreview);
		PreviewMaterial->SetVectorParameterValue("LandscapeScale", LandscapeTransform.GetScale3D());
		PreviewMaterial->SetVectorParameterValue("LandscapeSize", FVector(LandscapeSize.X, LandscapeSize.Y, 0));
		PreviewMaterial->SetVectorParameterValue("RenderTargetSize", FVector(RenderTargetSize.X, RenderTargetSize.Y, 0));
		PreviewMaterial->SetScalarParameterValue("Roughness", PreviewRoughness);
		PreviewMaterial->SetVectorParameterValue("BaseColor", PreviewBaseColor);
	}
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

