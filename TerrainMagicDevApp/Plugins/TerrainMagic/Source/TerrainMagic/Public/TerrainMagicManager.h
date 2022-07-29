// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeClip.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"
#include "Utils/TerrainMagicPaintLayerStore.h"
#include "TerrainMagicManager.generated.h"

UCLASS()
class TERRAINMAGIC_API ATerrainMagicManager : public AActor
{
	GENERATED_BODY()

	int HeightMapVersion = 0;
	FTerrainMagicPaintLayerStore PaintLayerStore;

public:
	// Sets default values for this actor's properties
	ATerrainMagicManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	FTransform LandscapeTransform;
	FIntPoint LandscapeSize;
	FIntPoint RenderTargetSize;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Initialize(const FTransform InputLandscapeTransform, const FIntPoint InputLandscapeSize,
											 const FIntPoint InputRenderTargetSize);

	TArray<ALandscapeClip*> GetAllLandscapeClips() const;

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
	
	void CacheHeightMap(UTextureRenderTarget2D* HeightMap);
	void ResetHeightMapCache();

	void RenderHeightMap(UMaterialInterface* Material);
	void RenderWeightMap(UMaterialInterface* Material) const;
	
	UTextureRenderTarget2D* EnsureHeightRenderTarget(const int Width, const int Height);
	UTextureRenderTarget2D* EnsureWeightRenderTarget(const int Width, const int Height);

	int GetHeightMapVersion() const;
};
