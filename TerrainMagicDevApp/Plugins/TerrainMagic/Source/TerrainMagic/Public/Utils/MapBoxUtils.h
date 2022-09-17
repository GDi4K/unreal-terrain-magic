#pragma once

struct FMapBoxTileData
{
	TArray<uint16> HeightData;
	float HeightRange;
	float MinHeight;
};

class FMapBoxUtils
{
public:
	static void DownloadTileRaw(int32 X, int32 Y, int32 Zoom, TFunction<void(TArray<FColor>)> Callback);
	static void DownloadTile(int32 X, int32 Y, int32 Zoom, TFunction<void(FMapBoxTileData*)> Callback);
	static void DownloadTileSet(int32 X, int32 Y, int32 Zoom, int32 ZoomInLevels, TFunction<void(FMapBoxTileData*)> Callback);
};
