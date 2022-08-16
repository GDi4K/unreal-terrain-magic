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

ALandscapeClip* HandleSoloClipLogic(TArray<ALandscapeClip*> LandscapeClips)
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

	HandleSoloClipLogic(LandscapeClips);

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
		LandscapeClip->LandscapeLocation = LandscapeTransform.GetLocation();
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
		Manager->RenderHeightMap(LandscapeClip->MaterialForHeight);

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

UTextureRenderTarget2D* UTerrainMagicBrushComponent::PaintLandscapeClips(FName LayerName,
	UTextureRenderTarget2D* InputWeightMap)
{
	UE_LOG(LogTemp, Warning, TEXT("Paintling Landscape Clips for: %s"), *LayerName.ToString())

	ATerrainMagicManager* Manager = EnsureManager();
	
	UTextureRenderTarget2D* WeightRenderTarget = Manager->EnsureWeightRenderTarget(RenderTargetSize.X, RenderTargetSize.Y);
	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), WeightRenderTarget);
	
	TArray<ALandscapeClip*> LandscapeClips = Manager->GetAllLandscapeClips();

	if (LandscapeClips.Num() == 0)
	{
		return InputWeightMap;
	}

	HandleSoloClipLogic(LandscapeClips);

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
		
		Params.Push({"HeightRT", Manager->GetHeightMap()});
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
		Manager->RenderWeightMap(LayerName, LandscapeClip->MaterialForWeight);
		UE_LOG(LogTemp, Warning, TEXT("Applying Paintlayer settings, %s"), *LandscapeClip->MaterialForWeight->GetName())
	
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

FLandscapeClipsInvalidationResponse UTerrainMagicBrushComponent::HasInvalidatedLandscapeClips()
{
	FLandscapeClipsInvalidationResponse Response;
	Response.bHasInvalidated = false;
	const ATerrainMagicManager* Manager = EnsureManager();

	TSet<FName> PaintLayers;
	for (ALandscapeClip* LandscapeClip: Manager->GetAllLandscapeClips())
	{
		if (LandscapeClip->bNeedsInvalidation)
		{
			Response.bHasInvalidated  = true;
		}
		LandscapeClip->bNeedsInvalidation = false;

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
