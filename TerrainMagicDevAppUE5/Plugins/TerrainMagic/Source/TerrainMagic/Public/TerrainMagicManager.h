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
	bool bClipsAreDirty = false;
	bool bNeedToInvalidateClips = false;

	FTerrainMagicPaintLayerResult FindPaintLayer(FVector Location);
	void PopulateLastZIndex();
	bool CanRenderPreviewMesh() const;

	UPROPERTY()
	int LastZIndex = -2002;

	UPROPERTY()
	bool bShowPreviewMesh = false;

	UPROPERTY()
	FVector LandscapeLocationBeforePreview{};

	UPROPERTY()
	UMaterialInstanceDynamic* PreviewMaterial = nullptr;

	// Here we use this buffer render target to save a copy of a render target
	UPROPERTY()
	UTextureRenderTarget2D* BufferRenderTargetForHeight = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* BufferRenderTargetForWeight = nullptr;

	// This is the initial render target & we need to keep a copy for the later use
	UPROPERTY()
	UTextureRenderTarget2D* BaseRenderTargetForHeight = nullptr;

public:
	// Sets default values for this actor's properties
	ATerrainMagicManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void Initialize(const FTransform InputLandscapeTransform, const FIntPoint InputLandscapeSize,
											 const FIntPoint InputRenderTargetSize);
	UTextureRenderTarget2D* RenderLandscapeClipsHeightMap(UTextureRenderTarget2D* InputHeightMap);
	UTextureRenderTarget2D* RenderLandscapeClipsWeightMap(FName LayerName, UTextureRenderTarget2D* InputWeightMap);
	void ClipsAreDirty();
	void InvalidateClips();
	bool NeedToInvalidateClips();

	TArray<ALandscapeClip*> GetAllLandscapeClips() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic|General")
	UStaticMeshComponent* PreviewMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic|General")
	FTransform LandscapeTransform;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic|General")
	FIntPoint LandscapeSize;

	UPROPERTY(VisibleAnywhere, Category="TerrainMagic|General")
	FIntPoint RenderTargetSize;

	UPROPERTY(BlueprintReadWrite, Category="TerrainMagic|General")
	UMaterialInstanceDynamic* CopyRTMaterial = nullptr;

	UPROPERTY()
	TArray<uint8> PaintLayerData;

	UPROPERTY()
	TArray<FName> PaintLayerNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic|General")
	UTextureRenderTarget2D* HeightRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic|General")
	UTextureRenderTarget2D* WeightRenderTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TerrainMagic|General")
	UTextureRenderTarget2D* CachedHeightMap = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic|PreviewMesh")
	FLinearColor PreviewBaseColor = {0.15, 0.06, 0.04};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic|PreviewMesh")
	float PreviewRoughness = 0.5;

	UFUNCTION(BlueprintCallable, Category="TerrainMagic|General")
	UTextureRenderTarget2D* GetHeightMap();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="TerrainMagic|General")
	void ShowClipOutlines() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="TerrainMagic|General")
	void HideClipOutlines() const;

	UFUNCTION(BlueprintCallable, Category="TerrainMagic|General")
	static FTerrainMagicPaintLayerResult FindLandscapePaintLayer(FVector Location);

	UFUNCTION(BlueprintCallable, CallInEditor, Category="TerrainMagic|General")
	void TogglePreview();
	
	void CacheHeightMap(UTextureRenderTarget2D* HeightMap);
	void ResetHeightMapCache();

	void RenderHeightMap(UMaterialInterface* Material);
	void RenderWeightMap(FName LayerName, UMaterialInterface* Material) const;

	void ResetPaintLayerData(const float ActivationThreshold);
	void ProcessPaintLayerData(FName LayerName, UTextureRenderTarget2D* RenderTarget);
	
	UTextureRenderTarget2D* EnsureHeightRenderTarget(const int Width, const int Height);
	UTextureRenderTarget2D* EnsureWeightRenderTarget(const int Width, const int Height);

	int GetHeightMapVersion() const;
	int GetNextLandscapeClipZIndex();

	static ATerrainMagicManager* EnsureManager(UWorld* World);
};
