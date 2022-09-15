// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "G16Texture.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class TERRAINMAGIC_API UG16Texture : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	FString PackageFilePath;

	UPROPERTY()
	FName TextureName;

	UPROPERTY()
	UPackage* Package = nullptr;

	UPROPERTY()
	int32 TextureWidth;

	UPROPERTY()
	UTexture2D* Texture = nullptr;

	UPROPERTY()
	TArray<uint16> SourceData;
	
	FUpdateTextureRegion2D UpdateRegion;

	void InitTexture() const;

public:
	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void Init(const int32 InputTextureWidth);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	static UG16Texture* MakeTransient(const int32 InputTextureWidth);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	static UG16Texture* MakeSerializable(int32 InputTextureWidth, FName InputTextureName);
	
	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	static UG16Texture* MakePersistable(int32 InputTextureWidth, FName InputTextureName, FName TextureDirectory);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	int32 GetTextureWidth();

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	UTexture2D* GetTexture();

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	int32 ReadPixel(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void WritePixel(int32 X, int32 Y, int32 Value);

	UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	void UpdateTexture();

	// UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	// void UpdateTextureRegion(FIntPoint Offset, FIntPoint Size);
	//
	// UFUNCTION(BlueprintCallable, Category="TerrainMagic")
	// void SaveTexture();
};
