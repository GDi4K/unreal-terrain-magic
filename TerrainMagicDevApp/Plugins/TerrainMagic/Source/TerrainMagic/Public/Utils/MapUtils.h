// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once
#include "MapUtils.generated.h"

UENUM(BlueprintType)
enum EMapSource
{
	MS_MAPBOX = 0 UMETA(DisplayName="Maxbox"),
	MS_PLANETARY_COMPUTER = 1 UMETA(DisplayName="Planetary Computer"),
};


struct FMapTileResponse
{
	bool IsSuccess = true;
	FString ErrorMessage = "";
	TArray<uint16> HeightData;
	float HeightRange;
	float MinHeight;
};

struct FMapTileDownloadProgress
{
	int32 TotalTiles = 0;
	int32 TilesDownloaded = 0;
};

struct FMapTileResponseRaw
{
	bool IsSuccess = true;
	FString ErrorMessage = "";
	TArray<FColor> RGBHeight;
};

struct FMapTileQuery
{
	int32 X;
	int32 Y;
	int32 Zoom;
	int32 ZoomInLevels = 0;
};

class FMapUtils
{
public:
	static void DownloadTileRaw(EMapSource Source, int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapTileResponseRaw>)> Callback);
	static void DownloadTileSet(EMapSource Source, const FMapTileQuery TileQuery, TFunction<void(TSharedPtr<FMapTileDownloadProgress>, TSharedPtr<FMapTileResponse>)> Callback);
	static void MakeG16Texture(int32 TextureWidth, uint16* HeightData, FString CacheTextureName, TFunction<void(UTexture2D*)> Callback);
	static UTexture2D* LoadCachedTexture(FString CacheTextureName);
};
