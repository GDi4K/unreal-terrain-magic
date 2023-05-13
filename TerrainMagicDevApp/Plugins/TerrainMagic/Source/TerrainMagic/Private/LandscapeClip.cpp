// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "LandscapeClip.h"

#include "TerrainMagicManager.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

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
	
	SetRootComponent(SceneComponent);

#if WITH_EDITORONLY_DATA
	#if ENGINE_MAJOR_VERSION == 5
		bIsSpatiallyLoaded = false;
	#endif
	bIsEditorOnlyActor = true;
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

	if (RunMatchLandscapeSizeActive && RunMatchLandscapeSizeAt < FDateTime::Now())
	{
		_MatchLandscapeSize();
		RunMatchLandscapeSizeActive = false;
	}

	// Initialize the Z-Index
	if (GetZIndex() < 0)
	{
		SetZIndex(ATerrainMagicManager::EnsureManager(GetWorld())->GetNextLandscapeClipZIndex());
	}

	// This happens when we add the Landscape Clip to the actor the very first time
	// In this case, make sure trigger for invalidate
	// Then we will get those data eventually.
	if (LandscapeScale.X == 0 && LandscapeScale.Y == 0)
	{
		_Invalidate();
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
	
	// Set Initial Z Position
	if (InitialZPosition == -12424.0)
	{
		InitialZPosition = GetActorLocation().Z;
	}
}

bool ALandscapeClip::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
		return true;
	return false;
}

#if WITH_EDITOR
void ALandscapeClip::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ATerrainMagicManager::EnsureManager(GetWorld())->ClipsAreDirty();
}

void ALandscapeClip::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	ATerrainMagicManager::EnsureManager(GetWorld())->ClipsAreDirty();
}
#endif

void ALandscapeClip::_Invalidate() const
{
	ATerrainMagicManager::EnsureManager(GetWorld())->InvalidateClips();
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
	_Invalidate();
	ATerrainMagicManager::EnsureManager(GetWorld())->TogglePreview();
}

void ALandscapeClip::_MatchLandscapeSize()
{
	const FVector CenterLocation = LandscapeLocation + ((LandscapeSize / 2) * LandscapeScale);
	InitialZPosition = CenterLocation.Z;

	const FVector ScaleFactorFromLandscapeScale = LandscapeScale / 100.0;
	SetActorLocation(CenterLocation);
	SetActorScale3D(ScaleFactorFromLandscapeScale);
	SetActorRotation(FRotator::MakeFromEuler(FVector(0, 0 , 0)));
	SetClipBaseSize(FVector2D(LandscapeSize.X, LandscapeSize.Y));
	_Invalidate();
}

void ALandscapeClip::_MatchLandscapeSizeDefferred(float Secs)
{
	RunMatchLandscapeSizeActive = true;
	RunMatchLandscapeSizeAt = FDateTime::Now() + FTimespan::FromSeconds(Secs);
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
		const FName MaterialPath = "/TerrainMagic/Core/Materials/LandscapeClips/M_TM_LandscapeClip_Weight.M_TM_LandscapeClip_Weight";
		UMaterial* SourceMaterial =  Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
		MaterialForWeight = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), SourceMaterial);
	}

	Params.Add({"WeightBrightness", PaintLayerSettings.WeightBrightness});
	Params.Add({"WeightContrast", PaintLayerSettings.WeightContrast});
	Params.Add({"PaintMode", static_cast<float>(PaintLayerSettings.PaintMode)});

	Params.Add({"TextureMapMaskEnabled", PaintLayerSettings.TextureMapMask.Enabled? 1.0f : 0.0f});
	Params.Add({"TextureMapMaskBrightness", PaintLayerSettings.TextureMapMask.Brightness});
	Params.Add({"TextureMapMaskContrast", PaintLayerSettings.TextureMapMask.Contrast});
	Params.Add({"TextureMapMaskTexture", PaintLayerSettings.TextureMapMask.Texture});

	Params.Add({"HeightMaskEnabled", PaintLayerSettings.HeightMask.Enabled? 1.0f : 0.0f});
	Params.Add({"HeightMaskRangeStart", PaintLayerSettings.HeightMask.RangeStart});
	Params.Add({"HeightMaskRangeEnd", PaintLayerSettings.HeightMask.RangeEnd});
	Params.Add({"HeightMaskContrast", PaintLayerSettings.HeightMask.Contrast});
	Params.Add({"HeightMaskBrightness", PaintLayerSettings.HeightMask.Brightness});
	Params.Add({"HeightMaskInvert", PaintLayerSettings.HeightMask.Invert? 1.0f : 0.0f});

	Params.Add({"NormalMaskEnabled", PaintLayerSettings.NormalMask.Enabled? 1.0f : 0.0f});
	Params.Add({"NormalMaskDirection", PaintLayerSettings.NormalMask.Direction});
	Params.Add({"NormalMaskContrast", PaintLayerSettings.NormalMask.Contrast});
	Params.Add({"NormalMaskBrightness", PaintLayerSettings.NormalMask.Brightness});
	Params.Add({"NormalSmoothingMultiplier", PaintLayerSettings.NormalMask.NormalSmoothMultiplier});
	Params.Add({"NormalMaskInvert", PaintLayerSettings.NormalMask.Invert? 1.0f : 0.0f});

	Params.Add({"LocationMaskEnabled", PaintLayerSettings.LocationMask.Enabled? 1.0f : 0.0f});
	Params.Add({"LocationMaskUVCenter", FVector(PaintLayerSettings.LocationMask.UVCenter, 0)});
	Params.Add({"LocationMaskExpandScale", PaintLayerSettings.LocationMask.ExpandScale});
	Params.Add({"LocationMaskContrast", PaintLayerSettings.LocationMask.Contrast});
	Params.Add({"LocationMaskBrightness", PaintLayerSettings.LocationMask.Brightness});
	Params.Add({"LocationMaskInvert", PaintLayerSettings.LocationMask.Invert? 1.0f : 0.0f});

	Params.Add({"EdgeNoiseMaskEnabled", PaintLayerSettings.EdgeNoiseMask.Enabled? 1.0f : 0.0f});
	Params.Add({"EdgeNoiseMaskScale", PaintLayerSettings.EdgeNoiseMask.Scale});
	Params.Add({"EdgeNoiseMaskContrast", PaintLayerSettings.EdgeNoiseMask.Contrast});
	Params.Add({"EdgeNoiseMaskRangeStart", PaintLayerSettings.EdgeNoiseMask.RangeStart});
	Params.Add({"EdgeNoiseMaskRangeEnd", PaintLayerSettings.EdgeNoiseMask.RangeEnd});
	Params.Add({"EdgeNoiseMaskSeed", PaintLayerSettings.EdgeNoiseMask.Seed});

	Params.Add({"AreaNoiseMaskEnabled", PaintLayerSettings.AreaNoiseMask.Enabled? 1.0f : 0.0f});
	Params.Add({"AreaNoiseMaskScale", PaintLayerSettings.AreaNoiseMask.Scale});
	Params.Add({"AreaNoiseMaskRangeStart", PaintLayerSettings.AreaNoiseMask.RangeStart});
	Params.Add({"AreaNoiseMaskRangeEnd", PaintLayerSettings.AreaNoiseMask.RangeEnd});
	Params.Add({"AreaNoiseMaskSeed", PaintLayerSettings.AreaNoiseMask.Seed});

	// Set Input Params
	for (FTerrainMagicMaterialParam Param: Params)
	{
		if (Param.Type == TMMP_SCALAR)
		{
			MaterialForWeight->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			MaterialForWeight->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			MaterialForWeight->SetTextureParameterValue(Param.Name, Param.TextureValue);
		}
	}

	// Set ChildClass Params
	// With this, we can get fade params, etc.
	// So, even for the weight material, this is useful
	
	for (FTerrainMagicMaterialParam Param: GetMaterialParams())
	{
		if (Param.Type == TMMP_SCALAR)
		{
			MaterialForWeight->SetScalarParameterValue(Param.Name, Param.ScalarValue);
		} else if (Param.Type == TMMP_VECTOR)
		{
			MaterialForWeight->SetVectorParameterValue(Param.Name, Param.VectorValue);
		} else if (Param.Type == TMMP_TEXTURE)
		{
			MaterialForWeight->SetTextureParameterValue(Param.Name, Param.TextureValue);
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

float ALandscapeClip::GetHeightAddition() const
{
	return GetActorLocation().Z - InitialZPosition;
}

