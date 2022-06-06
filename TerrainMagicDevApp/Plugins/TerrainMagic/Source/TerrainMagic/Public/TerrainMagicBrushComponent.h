﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "TerrainMagicBrushComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAINMAGIC_API UTerrainMagicBrushComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTerrainMagicBrushComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite)
	UMaterialInstanceDynamic* BrushMaterial;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FTransform LandscapeTransform;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FIntPoint LandscapeSize;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FIntPoint RenderTargetSize;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void Initialize(FTransform InputLandscapeTransform, FIntPoint InputLandscapeSize, FIntPoint InputRenderTargetSize);
	
	UFUNCTION(BlueprintCallable)
	void SetScalarRenderParam(FName Parameter, float Value);

	UFUNCTION(BlueprintCallable)
	void SetVectorRenderParam(FName Parameter, FVector Value);

	UFUNCTION(BlueprintCallable)
	void SetTextureRenderParam(FName Parameter, UTexture* Value);

	UFUNCTION(BlueprintCallable)
	void InitializeRenderParams(UTextureRenderTarget2D* InputHeightMap);

	UFUNCTION(BlueprintCallable)
	void SetScalarRenderParams(TMap<FName, float> Params);

	UFUNCTION(BlueprintCallable)
	void SetVectorRenderParams(TMap<FName, FVector> Params);
};
