// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TerrainMagicManager.generated.h"

struct FPaintLayerItem
{
	uint8 PaintLayerIndex;
	float Coverage;
};

USTRUCT(BlueprintType)
struct FTerrainMagicPaintLayerResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="TerrainMagic")
	bool FoundPaintLayer = false;

	UPROPERTY(BlueprintReadOnly, Category="TerrainMagic")
	FName PaintLayer;
};

UCLASS()
class TERRAINMAGIC_API ATerrainMagicManager : public AActor
{
	GENERATED_BODY()

	int HeightMapVersion = 0;
	FDateTime LastPaintLayerResetTime = 0;
	float PaintLayerActivationThreshold = 0;

	FTerrainMagicPaintLayerResult FindPaintLayer(FVector Location);

public:
	// Sets default values for this actor's properties
	ATerrainMagicManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Initialize(const FTransform InputLandscapeTransform, const FIntPoint InputLandscapeSize,
											 const FIntPoint InputRenderTargetSize);

	TArray<ALandscapeClip*> GetAllLandscapeClips() const;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic")
	FTransform LandscapeTransform;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic")
	FIntPoint LandscapeSize;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic")
	FIntPoint RenderTargetSize;

	UPROPERTY()
	TArray<uint8> PaintLayerData;

	UPROPERTY()
	TArray<FName> PaintLayerNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic")
	UTextureRenderTarget2D* HeightRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic")
	UTextureRenderTarget2D* WeightRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic")
	UTextureRenderTarget2D* CachedHeightMap = nullptr;

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	UTextureRenderTarget2D* GetHeightMap();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="TerrainMagic")
	void ShowClipOutlines() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="TerrainMagic")
	void HideClipOutlines() const;

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	static FTerrainMagicPaintLayerResult FindLandscapePaintLayer(FVector Location);
	
	void CacheHeightMap(UTextureRenderTarget2D* HeightMap);
	void ResetHeightMapCache();

	void RenderHeightMap(UMaterialInterface* Material);
	void RenderWeightMap(FName LayerName, UMaterialInterface* Material) const;

	void ResetPaintLayerData(const float ActivationThreshold);
	void ProcessPaintLayerData(FName LayerName, UTextureRenderTarget2D* RenderTarget);
	
	UTextureRenderTarget2D* EnsureHeightRenderTarget(const int Width, const int Height);
	UTextureRenderTarget2D* EnsureWeightRenderTarget(const int Width, const int Height);

	int GetHeightMapVersion() const;
};
