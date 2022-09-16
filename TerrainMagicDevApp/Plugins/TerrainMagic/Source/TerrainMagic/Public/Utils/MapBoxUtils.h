#pragma once

class FMapBoxUtils
{
public:
	static void DownloadTile(int32 X, int32 Y, int32 Zoom, TFunction<void(TArray<uint16>)> Callback);
};
