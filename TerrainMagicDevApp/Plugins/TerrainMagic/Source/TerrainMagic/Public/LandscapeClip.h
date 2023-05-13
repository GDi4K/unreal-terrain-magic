// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Types/FTerrainMagicRemap.h"
#include "Types/OutlineComponent.h"
#include "Types/TerrainMagicMaterialParam.h"
#include "Types/LandscapeClipPaintLayerSettings.h"
#include "Materials/Material.h"
#include "LandscapeClip.generated.h"

UENUM(BlueprintType)
enum ELandscapeClipBlendMode
{
	LCB_ADD = 0 UMETA(DisplayName="Add"),
	LCB_AVERAGE = 1 UMETA(DisplayName="Average"),
	LCB_MIN = 2 UMETA(DisplayName="Min"),
	LCB_Max = 3 UMETA(DisplayName="Max"),
	LCB_COPY = 4 UMETA(DisplayName="Copy"),
};

UENUM(BlueprintType)
enum ELandscapeClipFadeMode
{
	LCF_NONE = 0 UMETA(DisplayName="None"),
	LCF_CIRCULAR = 1 UMETA(DisplayName="Circular"),
	LCF_BOX = 2 UMETA(DisplayName="Box"),
};

UENUM(BlueprintType)
enum ELandscapeClipFadeMinimum
{
	LCFM_ZERO = 0 UMETA(DisplayName="0"),
	LCFM_REMAP_MIN = 1 UMETA(DisplayName="HeightMap Range Output Min")
};

enum ELandscapeClipSoloAction
{
	LCSA_NONE = 0,
	LCSA_SOLO = 1,
	LCSA_UNSOLO = 2,
};

USTRUCT(BlueprintType)
struct FLandscapeClipTileRepositioning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float Scale = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	FVector2D Panning = {0.0, 0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic", meta=(ToolTip="In Degrees", ClampMax=360, ClampMin=0))
	float Rotation = 0.0f;
};

UCLASS(Abstract)
class TERRAINMAGIC_API ALandscapeClip : public AActor
{
	GENERATED_BODY()

	FVector PrevScale3D = {1, 1, 1};

	UPROPERTY()
	float InitialZPosition = -12424.0;

	bool RunMatchLandscapeSizeActive = false;
	FDateTime RunMatchLandscapeSizeAt = 0;

public:
	// Sets default values for this actor's properties
	ALandscapeClip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
#if WITH_EDITOR	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif
	
	virtual TArray<FTerrainMagicMaterialParam> GetMaterialParams();
	virtual UTexture* GetHeightMap() const;
	virtual UMaterial* GetSourceMaterialForHeight() const;
	virtual int GetHeightMultiplier() const;
	virtual FVector2D GetClipBaseSize() const;
	virtual void SetClipBaseSize(FVector2D BaseSize);
	virtual void SetZIndex(int Index);
	virtual int GetZIndex() const;
	virtual bool IsEnabled() const;
	virtual void SetEnabled(bool bEnabled);
	virtual TArray<FLandscapeClipPaintLayerSettings> GetPaintLayerSettings() const;
	float GetHeightAddition() const;
	
	void _Invalidate() const;
	void _ToggleOutline();
	void _ToggleSolo();
	void _TogglePreview();
	void _MatchLandscapeSize();
	void _MatchLandscapeSizeDefferred(float Secs);

	UPROPERTY()
	bool bShowOutline = true;

	TEnumAsByte<ELandscapeClipSoloAction> SoloAction = LCSA_NONE;
	FDateTime SoloTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	UOutlineComponent* OutlineComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Actor")
	FVector LandscapeLocation = {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Actor")
	FVector LandscapeScale = {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Actor")
	FVector LandscapeSize= {};
	
	UPROPERTY()
	UMaterialInstanceDynamic* MaterialForHeight = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialForWeight = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* PreviewMaterial = nullptr;

	void ApplyMaterialParamsForHeight(TArray<FTerrainMagicMaterialParam> Array);
	void ApplyMaterialParamsForWeight(TArray<FTerrainMagicMaterialParam> Array, FLandscapeClipPaintLayerSettings PaintLayerSettings);

	FVector HeightMapRoot = {0, 0, 0};
	FVector2D HeightMapSizeInCM = {};
};
