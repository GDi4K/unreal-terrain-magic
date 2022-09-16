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
	static void DownloadTile(int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapBoxTileData>)> Callback);
};
