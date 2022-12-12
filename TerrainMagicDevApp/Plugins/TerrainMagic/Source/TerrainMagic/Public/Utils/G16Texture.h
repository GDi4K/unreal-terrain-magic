// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "G16Texture.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMAGIC_API UG16Texture : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	int32 TextureWidth;

	UPROPERTY()
	FString Directory;

	UPROPERTY()
	FString Filename;

	UPROPERTY()
	UTexture2D* RuntimeTexture = nullptr;

	TArray<uint8>* TextureData = nullptr;
	
	FUpdateTextureRegion2D UpdateRegion;
	
	void Init();

public:
	UG16Texture();
	~UG16Texture();
	
	int32 GetTextureWidth() const;
	void UpdateOnly(uint16* HeightData, TFunction<void(UTexture2D*)> Callback);
	void CacheToDisk() const;
	void UpdateAndCache(uint16* HeightData, TFunction<void(UTexture2D*)> Callback);
	UTexture2D* LoadCachedTexture() const;

	static UG16Texture* Create(UObject* Outer, int32 TextureWidth, FString Directory, FString Filename);
};