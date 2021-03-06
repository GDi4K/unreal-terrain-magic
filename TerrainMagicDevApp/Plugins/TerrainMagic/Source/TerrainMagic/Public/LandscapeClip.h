// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Types/FTerrainMagicRemap.h"
#include "Types/OutlineComponent.h"
#include "Types/TerrainMagicMaterialParam.h"
#include "LandscapeClip.generated.h"

UENUM(BlueprintType)
enum ELandscapeClipBlendMode
{
	LCB_ADD = 0 UMETA(DisplayName="Add"),
	LCB_AVERAGE = 1 UMETA(DisplayName="Average"),
	LCB_MIN = 2 UMETA(DisplayName="Min"),
	LCB_Max = 3 UMETA(DisplayName="Max"),
};

UENUM(BlueprintType)
enum ELandscapeClipFadeMode
{
	LCF_NONE = 0 UMETA(DisplayName="None"),
	LCF_CIRCULAR = 1 UMETA(DisplayName="Circular"),
	LCF_BOX = 2 UMETA(DisplayName="Box"),
};

enum ELandscapeClipSoloAction
{
	LCSA_NONE = 0,
	LCSA_SOLO = 1,
	LCSA_UNSOLO = 2,
};

UCLASS(Abstract)
class TERRAINMAGIC_API ALandscapeClip : public AActor
{
	GENERATED_BODY()

	FVector PrevScale3D = {1, 1, 1};

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
	
	virtual TArray<FTerrainMagicMaterialParam> GetMaterialParams();
	virtual UTexture* GetHeightMap() const;
	virtual UMaterial* GetSourceMaterial() const;
	virtual int GetHeightMultiplier() const;
	virtual FVector2D GetClipBaseSize() const;
	virtual void SetClipBaseSize(FVector2D BaseSize);
	virtual bool IsEnabled() const;
	virtual void SetEnabled(bool bEnabled);
	void _Invalidate();
	void _ToggleOutline();
	void _ToggleSolo();
	void _TogglePreview();
	void _MatchLandscapeSize();

	bool bNeedsInvalidation = false;

	UPROPERTY()
	bool bShowOutline = true;
	
	bool bShowPreview = false;

	TEnumAsByte<ELandscapeClipSoloAction> SoloAction = LCSA_NONE;
	FDateTime SoloTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	UOutlineComponent* OutlineComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Actor")
	FVector LandscapeLocation = {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Actor")
	FVector LandscapeScale = {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Actor")
	FVector LandscapeSize= {};
	
	UPROPERTY()
	UMaterialInstanceDynamic* Material = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* PreviewMaterial = nullptr;

	void ApplyMaterialParams(TArray<FTerrainMagicMaterialParam> Array);
	

	FVector HeightMapRoot = {0, 0, 0};
	FVector2D HeightMapSizeInCM = {};
};
