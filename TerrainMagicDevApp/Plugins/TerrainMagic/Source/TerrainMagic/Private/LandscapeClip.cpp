// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "LandscapeClip.h"

#include "TerrainMagicManager.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

// Sets default values
ALandscapeClip::ALandscapeClip()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	// Add Outline Component
	OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("OutlineComponent"));
	OutlineComponent->SetLineThickness(2000.0);
	OutlineComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);

	// Add Mesh Component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);

	const FName PlaneMeshLocation = "/Engine/BasicShapes/Plane.Plane";
	UStaticMesh* PlaneMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *PlaneMeshLocation.ToString()));
	MeshComponent->SetStaticMesh(PlaneMesh);

	const FName PreviewMaterialLocation = "/TerrainMagic/Core/Materials/M_LandscapeClip_Preview.M_LandscapeClip_Preview";
	UMaterial* PreviewMaterialSource = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *PreviewMaterialLocation.ToString()));
	PreviewMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), PreviewMaterialSource);
	MeshComponent->SetMaterial(0, PreviewMaterial);
	
	SetRootComponent(SceneComponent);

#if WITH_EDITORONLY_DATA
	#if ENGINE_MAJOR_VERSION == 5
		bIsSpatiallyLoaded = false;
	#endif
#endif
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

	if (GetWorld() == nullptr)
	{
		return;
	}

	
	if (GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}

	// Initialize the Z-Index
	if (GetZIndex() < 0)
	{
		SetZIndex(ATerrainMagicManager::EnsureManager(GetWorld())->GetNextLandscapeClipZIndex());
	}
	
	// We only support Z axis based rotation only
	FVector CurrentRotation = GetActorRotation().Euler();
	CurrentRotation.X = 0.0;
	CurrentRotation.Y = 0.0;
	SetActorRotation(FRotator::MakeFromEuler(CurrentRotation));

	// We only support uniform scaling
	const FVector CurrentScale3D = GetActorScale3D();
	FVector NewScale3D = CurrentScale3D;
	if (CurrentScale3D.X != PrevScale3D.X)
	{
		NewScale3D.X = CurrentScale3D.X;
		NewScale3D.Y = CurrentScale3D.X;
	} else if (CurrentScale3D.Y != PrevScale3D.Y)
	{
		NewScale3D.X = CurrentScale3D.Y;
		NewScale3D.Y = CurrentScale3D.Y;
	}
	NewScale3D.Z = 1.6;

	PrevScale3D = CurrentScale3D;
	SetActorScale3D(NewScale3D);

	// Set HeightMapSize
	HeightMapRoot = GetActorLocation();
	HeightMapSizeInCM = GetClipBaseSize() * FVector2D(GetActorScale3D()) * 100;

	// Render Outline
	OutlineComponent->SetVisibility(bShowOutline);
	if (bShowOutline)
	{
		OutlineComponent->SetBoxExtent({
			GetClipBaseSize().X/2 * 100,
			GetClipBaseSize().Y/2 * 100,
			static_cast<float>(GetHeightMultiplier()/2.0)
		});
	}

	// Render Preview
	MeshComponent->SetVisibility(bShowPreview);
	
	if (bShowPreview)
	{
		// Set the MeshComponent Scale
		// The plane mesh we set has size of 100cm x 100cm
		// That's why we divide here by 100
		const FVector2D ClipBaseSizeInCM = GetClipBaseSize() * 100;
		MeshComponent->SetRelativeScale3D(FVector(
			(ClipBaseSizeInCM/100).X,
			(ClipBaseSizeInCM/100).Y,
			1
		));
		MeshComponent->SetRelativeLocation({
			0,
			0,
			static_cast<float>(GetHeightMultiplier()/2.0)
		});

		PreviewMaterial->SetTextureParameterValue("HeightMap", GetHeightMap());
	}
}

bool ALandscapeClip::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
		return true;
	return false;
}

void ALandscapeClip::_Invalidate()
{
	bNeedsInvalidation = true;
}

void ALandscapeClip::_ToggleOutline()
{
	bShowOutline = !bShowOutline;
}

void ALandscapeClip::_ToggleSolo()
{
	SoloTime = FDateTime::Now();
	if (SoloAction == LCSA_SOLO)
	{
		SoloAction = LCSA_UNSOLO;
	} else
	{
		SoloAction = LCSA_SOLO;
	}
	
	_Invalidate();
}

void ALandscapeClip::_TogglePreview()
{
	bShowPreview = !bShowPreview;
}

void ALandscapeClip::_MatchLandscapeSize()
{
	const FVector CenterLocation = LandscapeLocation + ((LandscapeSize / 2) * LandscapeScale);
	const FVector ScaleFactorFromLandscapeScale = LandscapeScale / 100.0;
	SetActorLocation(CenterLocation);
	SetActorScale3D(ScaleFactorFromLandscapeScale);
	SetActorRotation(FRotator::MakeFromEuler(FVector(0, 0 , 0)));
	SetClipBaseSize(FVector2D(LandscapeSize.X, LandscapeSize.Y));
	_Invalidate();
}

void ALandscapeClip::ApplyMaterialParamsForHeight(TArray<FTerrainMagicMaterialParam> Params)
{
	if (MaterialForHeight == nullptr)
	{
		MaterialForHeight = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), GetSourceMaterialForHeight());
	}
	
	// Set Input Params
	for (FTerrainMagicMaterialParam Param: Params)
	{
		if (Param.Type == TMMP_SCALAR)
		{
			MaterialForHeight->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			MaterialForHeight->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			MaterialForHeight->SetTextureParameterValue(Param.Name, Param.TextureValue);
		}
	}
	
	// Set ChildClass Params
	for (FTerrainMagicMaterialParam Param: GetMaterialParams())
	{
		if (Param.Type == TMMP_SCALAR)
		{
			MaterialForHeight->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			MaterialForHeight->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			MaterialForHeight->SetTextureParameterValue(Param.Name, Param.TextureValue);
		}
	}
}

void ALandscapeClip::ApplyMaterialParamsForWeight(TArray<FTerrainMagicMaterialParam> Params, FLandscapeClipPaintLayerSettings PaintLayerSettings)
{
	if (MaterialForWeight == nullptr)
	{
		const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_Weight.M_TM_LandscapeClip_Weight";
		UMaterial* SourceMaterial =  Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
		MaterialForWeight = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), SourceMaterial);
	}
	
	// Set Input Params
	for (FTerrainMagicMaterialParam Param: Params)
	{
		if (Param.Type == TMMP_SCALAR)
		{
			MaterialForHeight->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			MaterialForHeight->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			MaterialForHeight->SetTextureParameterValue(Param.Name, Param.TextureValue);
		}
	}
}

TArray<FTerrainMagicMaterialParam> ALandscapeClip::GetMaterialParams()
{
	return {};
}

UTexture* ALandscapeClip::GetHeightMap() const
{
	return nullptr;
}

UMaterial* ALandscapeClip::GetSourceMaterialForHeight() const
{
	return nullptr;
}

int ALandscapeClip::GetHeightMultiplier() const
{
	return 0;
}

FVector2D ALandscapeClip::GetClipBaseSize() const
{
	return {};
}

void ALandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
}

void ALandscapeClip::SetZIndex(int Index)
{
}

int ALandscapeClip::GetZIndex() const
{
	return 0;
}

bool ALandscapeClip::IsEnabled() const
{
	return true;
}

void ALandscapeClip::SetEnabled(bool bEnabled)
{
	
}

TArray<FLandscapeClipPaintLayerSettings> ALandscapeClip::GetPaintLayerSettings() const
{
	return {};
}

