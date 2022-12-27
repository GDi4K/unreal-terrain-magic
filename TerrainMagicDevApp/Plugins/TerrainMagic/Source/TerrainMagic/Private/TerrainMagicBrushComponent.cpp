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
	
	const FName CopyRTMaterialPath = "/TerrainMagic/Core/Materials/Utils/M_Copy_RT.M_Copy_RT";
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

	EnsureManager()->Initialize(LandscapeTransform, LandscapeSize, RenderTargetSize);
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
	return ATerrainMagicManager::EnsureManager(GetWorld());
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

void UTerrainMagicBrushComponent::ResetPaintLayerData(const float ActivationThreshold)
{
	EnsureManager()->ResetPaintLayerData(ActivationThreshold);
}

void UTerrainMagicBrushComponent::ProcessPaintLayerData(FName LayerName, UTextureRenderTarget2D* RenderTarget)
{
	// We need to keep this since, this allow us to process paint layers
	// which haven't use inside any weight brush
	EnsureManager()->ProcessPaintLayerData(LayerName, RenderTarget);
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

UTextureRenderTarget2D* UTerrainMagicBrushComponent::RenderWeightMap(FName LayerName, UTextureRenderTarget2D* InputWeightMap)
{
	ATerrainMagicManager* Manager = EnsureManager();
	// TODO: Remove this after sometime after introducing accessing paint layer info from Blueprints
	// This is already doing in the Initialize method o fthis component
	// But we add this make sure existing projects created without the method of the Manager
	// In that case, we need to update it like this
	Manager->Initialize(LandscapeTransform, LandscapeSize, RenderTargetSize);
	
	InitializeRenderParams(Manager->GetHeightMap());
	SetTextureRenderParam("WeightRT", InputWeightMap);

	UTextureRenderTarget2D* WeightRenderTarget = Manager->EnsureWeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);

	Manager->RenderWeightMap(LayerName, BrushMaterial);

	// We still need to process these here.
	// Otherwise if the related WeightBrush used below the InfoBrush, then
	// info brush won't see these changes.
	Manager->ProcessPaintLayerData(LayerName, WeightRenderTarget);

	return WeightRenderTarget;
}

UTextureRenderTarget2D* UTerrainMagicBrushComponent::RenderLandscapeClips(UTextureRenderTarget2D* InputHeightMap)
{
	ATerrainMagicManager* Manager = EnsureManager();
	return Manager->RenderLandscapeClipsHeightMap(InputHeightMap);
}

UTextureRenderTarget2D* UTerrainMagicBrushComponent::PaintLandscapeClips(FName LayerName,
	UTextureRenderTarget2D* InputWeightMap)
{
	ATerrainMagicManager* Manager = EnsureManager();
	return Manager->RenderLandscapeClipsWeightMap(LayerName, InputWeightMap);
}

FLandscapeClipsInvalidationResponse UTerrainMagicBrushComponent::HasInvalidatedLandscapeClips()
{
	FLandscapeClipsInvalidationResponse Response;
	ATerrainMagicManager* Manager = EnsureManager();
	Response.bHasInvalidated = Manager->NeedToInvalidateClips();

	TSet<FName> PaintLayers;
	for (ALandscapeClip* LandscapeClip: Manager->GetAllLandscapeClips())
	{
		// Finding Paint Layers
		for(const FLandscapeClipPaintLayerSettings PaintLayerSettings: LandscapeClip->GetPaintLayerSettings())
		{
			PaintLayers.Add(PaintLayerSettings.PaintLayer);
			for(const FName AdditionalPaintLayer: PaintLayerSettings.AdditionalPaintLayers)
			{
				PaintLayers.Add(AdditionalPaintLayer);
			}
		}
	}

	Response.AffectedPaintLayers = PaintLayers.Array();
	
	return Response;
}
