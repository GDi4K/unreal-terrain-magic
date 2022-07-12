// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "LandscapeClip.h"

#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"


// Sets default values
ALandscapeClip::ALandscapeClip()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("OutlineComponent"));
	OutlineComponent->SetLineThickness(1000.0);
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

	if (GetWorld() == nullptr)
	{
		return;
	}

	
	if (GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
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

void ALandscapeClip::ApplyMaterialParams(TArray<FTerrainMagicMaterialParam> Params)
{
	if (Material == nullptr)
	{
		Material = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), GetSourceMaterial());
	}
	
	// Set Input Params
	for (FTerrainMagicMaterialParam Param: Params)
	{
		if (Param.Type == TMMP_SCALAR)
		{
			Material->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			Material->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			Material->SetTextureParameterValue(Param.Name, Param.TextureValue);
		}
	}
	
	// Set ChildClass Params
	for (FTerrainMagicMaterialParam Param: GetMaterialParams())
	{
		if (Param.Type == TMMP_SCALAR)
		{
			Material->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			Material->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			Material->SetTextureParameterValue(Param.Name, Param.TextureValue);
		}
	}
}

TArray<FTerrainMagicMaterialParam> ALandscapeClip::GetMaterialParams()
{
	return {};
}

UMaterial* ALandscapeClip::GetSourceMaterial() const
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

