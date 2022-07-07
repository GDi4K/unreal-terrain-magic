// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicBrushComponent.h"

#include "LandscapeClip.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UTerrainMagicBrushComponent::UTerrainMagicBrushComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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

	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), HeightRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), HeightRenderTarget, BrushMaterial);
	
	return HeightRenderTarget;
}

UTextureRenderTarget2D* UTerrainMagicBrushComponent::RenderWeightMap(UTextureRenderTarget2D* InputWeightMap)
{
	ATerrainMagicManager* Manager = EnsureManager();
	
	InitializeRenderParams(Manager->GetHeightMap());
	SetTextureRenderParam("WeightRT", InputWeightMap);

	UTextureRenderTarget2D* WeightRenderTarget = Manager->EnsureWeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);

	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), WeightRenderTarget);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), WeightRenderTarget, BrushMaterial);

	return WeightRenderTarget;
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
		ClipMaterial->SetTextureParameterValue("HeightRT", BufferRenderTarget);
		ClipMaterial->SetVectorParameterValue("LandscapeLocation", LandscapeTransform.GetLocation());
		ClipMaterial->SetVectorParameterValue("LandscapeScale", LandscapeTransform.GetScale3D());
		ClipMaterial->SetVectorParameterValue("LandscapeSize", FVector(LandscapeSize.X, LandscapeSize.Y, 0));
		ClipMaterial->SetVectorParameterValue("RenderTargetSize", FVector(RenderTargetSize.X, RenderTargetSize.Y, 0));

		ClipMaterial->SetTextureParameterValue("Texture", LandscapeClip->HeightMap);
		ClipMaterial->SetVectorParameterValue("TextureRoot", LandscapeClip->HeightMapRoot);
		ClipMaterial->SetVectorParameterValue("TextureSizeInCM", {LandscapeClip->HeightMapSizeInCM.X, LandscapeClip->HeightMapSizeInCM.Y, 0});
		ClipMaterial->SetVectorParameterValue("TextureRotationInDegrees", LandscapeClip->GetActorRotation().Euler());
		
		ClipMaterial->SetScalarParameterValue("HeightMultiplier", LandscapeClip->HeightMultiplier);
		ClipMaterial->SetScalarParameterValue("SelectedBlendMode", LandscapeClip->BlendMode);
		
		ClipMaterial->SetScalarParameterValue("HeightMapInputMin", LandscapeClip->HeightMapRange.InputMin);
		ClipMaterial->SetScalarParameterValue("HeightMapInputMax", LandscapeClip->HeightMapRange.InputMax);
		ClipMaterial->SetScalarParameterValue("HeightMapOutputMin", LandscapeClip->HeightMapRange.OutputMin);
		ClipMaterial->SetScalarParameterValue("HeightMapOutputMax", LandscapeClip->HeightMapRange.OutputMax);

		ClipMaterial->SetScalarParameterValue("HeightSaturation", LandscapeClip->HeightSaturation);

		// Render the Clip
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), HeightRenderTarget, ClipMaterial);

		// Copy the NewHeightMap to the Buffer
		CopyRTMaterial->SetTextureParameterValue("RenderTarget", HeightRenderTarget);
		UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), BufferRenderTarget);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), BufferRenderTarget, CopyRTMaterial);
	}
	
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
