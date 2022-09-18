// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

struct FMapBoxTileResponse
{
	bool IsSuccess = true;
	FString ErrorMessage = "";
	TArray<uint16> HeightData;
	float HeightRange;
	float MinHeight;
};

struct FMapBoxTileDownloadProgress
{
	int32 TotalTiles = 0;
	int32 TilesDownloaded = 0;
};

struct FMapBoxTileResponseRaw
{
	bool IsSuccess = true;
	FString ErrorMessage = "";
	TArray<FColor> RGBHeight;
};

struct FMapBoxTileQuery
{
	int32 X;
	int32 Y;
	int32 Zoom;
	int32 ZoomInLevels = 0;
};

class FMapBoxUtils
{
public:
	static void DownloadTileRaw(int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapBoxTileResponseRaw>)> Callback);
	static void DownloadTileSet(const FMapBoxTileQuery TileQuery, TFunction<void(TSharedPtr<FMapBoxTileDownloadProgress>, TSharedPtr<FMapBoxTileResponse>)> Callback);
	static void MakeG16Texture(int32 TextureWidth, uint16* HeightData, TFunction<void(UTexture2D*)> Callback);
};
