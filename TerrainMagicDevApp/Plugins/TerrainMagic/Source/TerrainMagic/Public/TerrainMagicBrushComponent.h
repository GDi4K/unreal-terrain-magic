// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TerrainMagicManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/ActorComponent.h"

#include "TerrainMagicBrushComponent.generated.h"

USTRUCT(BlueprintType)
struct FLandscapeClipsInvalidationResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="TerrainMagic")
	bool bHasInvalidated = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="TerrainMagic")
	TArray<FName> AffectedPaintLayers = {};
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAINMAGIC_API UTerrainMagicBrushComponent : public UActorComponent
{
	GENERATED_BODY()

	// Here we use this buffer render target to save a copy of a render target
	UPROPERTY()
	UTextureRenderTarget2D* BufferRenderTargetForHeight = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* BufferRenderTargetForWeight = nullptr;

public:
	// Sets default values for this component's properties
	UTerrainMagicBrushComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, Category="TerrainMagic")
	UMaterialInstanceDynamic* BrushMaterial;

	UPROPERTY(BlueprintReadWrite, Category="TerrainMagic")
	UMaterialInstanceDynamic* CopyRTMaterial;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="TerrainMagic")
	FTransform LandscapeTransform;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="TerrainMagic")
	FIntPoint LandscapeSize;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="TerrainMagic")
	FIntPoint RenderTargetSize;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void Initialize(FTransform InputLandscapeTransform, FIntPoint InputLandscapeSize, FIntPoint InputRenderTargetSize);
	
	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void SetScalarRenderParam(FName Parameter, float Value);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void SetVectorRenderParam(FName Parameter, FVector Value);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void SetTextureRenderParam(FName Parameter, UTexture* Value);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void SetScalarRenderParams(TMap<FName, float> Params);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void SetVectorRenderParams(TMap<FName, FVector> Params);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void InitializeRenderParams(UTextureRenderTarget2D* InputHeightMap);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	UTextureRenderTarget2D* RenderHeightMap(UTextureRenderTarget2D* InputHeightMap);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	UTextureRenderTarget2D* RenderWeightMap(FName LayerName, UTextureRenderTarget2D* InputWeightMap);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	UTextureRenderTarget2D* RenderLandscapeClips(UTextureRenderTarget2D* InputHeightMap);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	UTextureRenderTarget2D* PaintLandscapeClips(FName LayerName, UTextureRenderTarget2D* InputWeightMap);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	FLandscapeClipsInvalidationResponse HasInvalidatedLandscapeClips();
	
	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	ATerrainMagicManager* EnsureManager();

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	bool HasHeightMap();

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void CacheHeightMap(UTextureRenderTarget2D* InputHeightMap);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void ResetHeightMapCache();

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	int GetHeightMapVersion();

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void ResetPaintLayerData(const float ActivationThreshold);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void ProcessPaintLayerData(FName LayerName, UTextureRenderTarget2D* RenderTarget);
};
