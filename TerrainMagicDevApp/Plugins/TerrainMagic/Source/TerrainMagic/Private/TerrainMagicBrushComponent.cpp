// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicBrushComponent.h"

#include "LandscapeClip.h"
#include "Engine/Canvas.h"
#include "Materials/Material.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"


// Sets default values for this component's properties
UTerrainMagicBrushComponent::UTerrainMagicBrushComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	const FName CopyRTMaterialPath = "/TerrainMagic/Core/Materials/M_Copy_RT.M_Copy_RT";
	UMaterial* MaterialSource = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *CopyRTMaterialPath.ToString()));
	CopyRTMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), MaterialSource);
}


// Called when the game starts
void UTerrainMagicBrushComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTerrainMagicBrushComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTerrainMagicBrushComponent::Initialize(const FTransform InputLandscapeTransform, const FIntPoint InputLandscapeSize,
                                             const FIntPoint InputRenderTargetSize)
{
	LandscapeTransform = InputLandscapeTransform;
	LandscapeSize = InputLandscapeSize;
	RenderTargetSize = InputRenderTargetSize;
}

void UTerrainMagicBrushComponent::SetScalarRenderParam(const FName Parameter, const float Value)
{
	BrushMaterial->SetScalarParameterValue(Parameter, Value);
}

void UTerrainMagicBrushComponent::SetVectorRenderParam(const FName Parameter, const FVector Value)
{
	BrushMaterial->SetVectorParameterValue(Parameter, Value);
}

void UTerrainMagicBrushComponent::SetTextureRenderParam(const FName Parameter, UTexture* Value)
{
	BrushMaterial->SetTextureParameterValue(Parameter, Value);
}

ATerrainMagicManager* UTerrainMagicBrushComponent::EnsureManager()
{
	AActor* CurrentActor = UGameplayStatics::GetActorOfClass(GetWorld(), ATerrainMagicManager::StaticClass());
	if (CurrentActor == nullptr)
	{
		AActor* SpawnedActor = GetWorld()->SpawnActor(ATerrainMagicManager::StaticClass());
		return Cast<ATerrainMagicManager>(SpawnedActor);
	}

	return Cast<ATerrainMagicManager>(CurrentActor);
}

bool UTerrainMagicBrushComponent::HasHeightMap()
{
	return EnsureManager()->GetHeightMap() != nullptr;
}

void UTerrainMagicBrushComponent::CacheHeightMap(UTextureRenderTarget2D* InputHeightMap)
{
	EnsureManager()->CacheHeightMap(InputHeightMap);
}

void UTerrainMagicBrushComponent::ResetHeightMapCache()
{
	EnsureManager()->ResetHeightMapCache();
}

int UTerrainMagicBrushComponent::GetHeightMapVersion()
{
	return EnsureManager()->GetHeightMapVersion();
}

void UTerrainMagicBrushComponent::SetScalarRenderParams(TMap<FName, float> Params)
{
	for (const auto Item : Params)
	{
		SetScalarRenderParam(Item.Key, Item.Value);
	}
}

void UTerrainMagicBrushComponent::SetVectorRenderParams(TMap<FName, FVector> Params)
{
	for (const auto Item : Params)
	{
		SetVectorRenderParam(Item.Key, Item.Value);
	}
}

void UTerrainMagicBrushComponent::InitializeRenderParams(UTextureRenderTarget2D* InputHeightMap)
{
	SetTextureRenderParam("HeightRT", InputHeightMap);
	SetVectorRenderParam("LandscapeLocation", LandscapeTransform.GetLocation());
	SetVectorRenderParam("LandscapeScale", LandscapeTransform.GetScale3D());
	SetVectorRenderParam("LandscapeSize", FVector(LandscapeSize.X, LandscapeSize.Y, 0));
	SetVectorRenderParam("RenderTargetSize", FVector(RenderTargetSize.X, RenderTargetSize.Y, 0));
}

UTextureRenderTarget2D* UTerrainMagicBrushComponent::RenderHeightMap(UTextureRenderTarget2D* InputHeightMap)
{
	InitializeRenderParams(InputHeightMap);
	ATerrainMagicManager* Manager = EnsureManager();

	UTextureRenderTarget2D* HeightRenderTarget = Manager->EnsureHeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);

	Manager->RenderHeightMap(BrushMaterial);
	
	return HeightRenderTarget;
}

UTextureRenderTarget2D* UTerrainMagicBrushComponent::RenderWeightMap(UTextureRenderTarget2D* InputWeightMap)
{
	ATerrainMagicManager* Manager = EnsureManager();
	
	InitializeRenderParams(Manager->GetHeightMap());
	SetTextureRenderParam("WeightRT", InputWeightMap);

	UTextureRenderTarget2D* WeightRenderTarget = Manager->EnsureWeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);

	Manager->RenderWeightMap(BrushMaterial);

	return WeightRenderTarget;
}

ALandscapeClip* FindSoloClip(TArray<ALandscapeClip*> LandscapeClips)
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

UTextureRenderTarget2D* UTerrainMagicBrushComponent::RenderLandscapeClips(UTextureRenderTarget2D* InputHeightMap)
{
	ATerrainMagicManager* Manager = EnsureManager();
	
	UTextureRenderTarget2D* HeightRenderTarget = Manager->EnsureHeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), HeightRenderTarget);
	
	TArray<ALandscapeClip*> LandscapeClips = Manager->GetAllLandscapeClips();

	if (LandscapeClips.Num() == 0)
	{
		return InputHeightMap;
	}

	const ALandscapeClip* SoloClip = FindSoloClip(LandscapeClips);

	if (BufferRenderTarget == nullptr)
	{
		BufferRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), RenderTargetSize.X, RenderTargetSize.Y, RTF_RGBA8);
	}

	// Copy the Input HeightMap at the beginning
	CopyRTMaterial->SetTextureParameterValue("RenderTarget", InputHeightMap);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTarget, CopyRTMaterial);
	
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
		
		// Apply Params
		TArray<FTerrainMagicMaterialParam> Params = {};
		
		Params.Push({"HeightRT", BufferRenderTarget});
		Params.Push({"LandscapeLocation", LandscapeTransform.GetLocation()});
		Params.Push({"LandscapeScale", LandscapeTransform.GetScale3D()});
		Params.Push({"LandscapeSize", FVector(LandscapeSize.X, LandscapeSize.Y, 0)});
		Params.Push({"RenderTargetSize", FVector(RenderTargetSize.X, RenderTargetSize.Y, 0)});

		Params.Push({"ClipRoot", LandscapeClip->HeightMapRoot});
		Params.Push({"ClipSizeInCM", FVector(LandscapeClip->HeightMapSizeInCM.X, LandscapeClip->HeightMapSizeInCM.Y, 0)});
		Params.Push({"ClipRotationInDegrees", LandscapeClip->GetActorRotation().Euler()});

		LandscapeClip->ApplyMaterialParams(Params);
		
		// Render the Clip
		Manager->RenderHeightMap(LandscapeClip->Material);

		// Copy the NewHeightMap to the Buffer
		CopyRTMaterial->SetTextureParameterValue("RenderTarget", HeightRenderTarget);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTarget);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTarget, CopyRTMaterial);
	}

	// This will prevent us from a memory leak where holding a render target causes some issues.
	// It happens when reloading the map inside the Editor
	CopyRTMaterial->ClearParameterValues();
	
	return HeightRenderTarget;
}

bool UTerrainMagicBrushComponent::HasInvalidatedLandscapeClips()
{
	bool bNeedsInvalidation = false;
	const ATerrainMagicManager* Manager = EnsureManager();

	for (ALandscapeClip* LandscapeClip: Manager->GetAllLandscapeClips())
	{
		if (LandscapeClip->bNeedsInvalidation)
		{
			bNeedsInvalidation = true;
		}
		LandscapeClip->bNeedsInvalidation = false;
	}

	return bNeedsInvalidation;
}
