// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Types/FTerrainMagicRemap.h"
#include "Types/OutlineComponent.h"
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

UCLASS()
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

	bool bNeedsInvalidation = false;
	bool bShowOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	UOutlineComponent* OutlineComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	UTexture* HeightMap = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Size of the HeightMap in meters before scaling"),  Category="01-General")
	FVector2D HeightMapBaseSize = {1000, 1000};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	TEnumAsByte<ELandscapeClipBlendMode> BlendMode = LCB_ADD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General", meta=(ToolTip="A multiplier that will use with the following HeightMap range output values. This value doesn't scale with the clip actor's scaling factors."))
	int HeightMultiplier = 32000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	FTerrainMagicRemap HeightMapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="01-General")
	float HeightSaturation = 1.0;
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="01-General")
	void Invalidate();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="01-General")
	void ToggleOutline();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Modifiers")
	TEnumAsByte<ELandscapeClipFadeMode> FadeMode = LCF_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Modifiers")
	float FadeSaturation = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="02-Modifiers")
	float FadeMaskSpan = 1.0;
	
	FVector HeightMapRoot = {0, 0, 0};
	FVector2D HeightMapSizeInCM = HeightMapBaseSize * 100;
};
