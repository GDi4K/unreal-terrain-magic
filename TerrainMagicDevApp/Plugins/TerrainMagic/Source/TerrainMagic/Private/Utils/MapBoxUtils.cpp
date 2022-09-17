#include "Utils/MapBoxUtils.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void FMapBoxUtils::DownloadTileRaw(int32 X, int32 Y, int32 Zoom, TFunction<void(TArray<FColor>)> Callback)
{
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";
	const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);

	// UE_LOG(LogTemp, Warning, TEXT("Image URL: %s"), *ImageURL);
	// return;
	
	//const FString ImageURL = "https://planetarycomputer.microsoft.com/api/data/v1/mosaic/tiles/78bee4a24f875fdc7f333d6765996655/WebMercatorQuad/11/348/737@2x?assets=data&colormap_name=terrain&rescale=-1000,4000&collection=3dep-seamless&format=png";
	
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool HasCompleted)
	{
		const bool HasDownloaded = HasCompleted && Request.IsValid() && Response->GetContentLength() > 0;
		if (!HasDownloaded)
		{
			return;
		}

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		ImageWrapper->SetCompressed(Response->GetContent().GetData(), Response->GetContentLength());
		TArray<uint8> RawImageData;
		constexpr ERGBFormat InFormat = ERGBFormat::BGRA;
		const bool HasFetchImageData = ImageWrapper->GetRaw(InFormat, 8, RawImageData);

		if (HasFetchImageData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fetching Image Data is a Success!"))
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("Fetching Image Data has Failed!"))
		}

		TArray<FColor> Pixels;
		Pixels.SetNumUninitialized(512 * 512);
		
		for (int32 X=0; X<512; X++)
		{
			for (int32 Y=0; Y<512; Y++)
			{
				const int32 Index = Y * 512 + X;
				const int8 R = RawImageData[Index * 4 + 2];
				const int8 G = RawImageData[Index * 4 + 1];
				const int8 B = RawImageData[Index * 4 + 0];

				Pixels[Index] = FColor(R, G, B, 255);
			}
		}

		Callback(Pixels);
	});

	HttpRequest->SetURL(ImageURL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void FMapBoxUtils::DownloadTile(int32 X, int32 Y, int32 Zoom, TFunction<void(FMapBoxTileData*)> Callback)
{
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";
	//const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);

	const FString ImageURL = "https://planetarycomputer.microsoft.com/api/data/v1/mosaic/tiles/78bee4a24f875fdc7f333d6765996655/WebMercatorQuad/11/348/737@2x?assets=data&colormap_name=terrain&rescale=-1000,4000&collection=3dep-seamless&format=png";
	
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool HasCompleted)
	{
		const bool HasDownloaded = HasCompleted && Request.IsValid() && Response->GetContentLength() > 0;
		if (!HasDownloaded)
		{
			return;
		}

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		ImageWrapper->SetCompressed(Response->GetContent().GetData(), Response->GetContentLength());
		TArray<uint8> RawImageData;
		constexpr ERGBFormat InFormat = ERGBFormat::BGRA;
		const bool HasFetchImageData = ImageWrapper->GetRaw(InFormat, 8, RawImageData);

		if (HasFetchImageData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fetching Image Data is a Success!"))
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("Fetching Image Data has Failed!"))
		}
		
		TArray<float> HeightArray;
		HeightArray.SetNumZeroed(512 * 512);
		
		for (int32 X=0; X<512; X++)
		{
			for (int32 Y=0; Y<512; Y++)
			{
				const int32 Index = Y * 512 + X;
				const int8 R = RawImageData[Index * 4 + 2];
				const int8 G = RawImageData[Index * 4 + 1];
				const int8 B = RawImageData[Index * 4 + 0];
				
				const float Height = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1);
				HeightArray[Index] = Height;
			}
		}

		FMapBoxTileData* TileInfo = new FMapBoxTileData();
		TileInfo->HeightData.SetNumUninitialized(512 * 512);
		
		TileInfo->MinHeight = FMath::Min(HeightArray);
		const float MaxHeight = FMath::Max(HeightArray);
		TileInfo->HeightRange = MaxHeight - TileInfo->MinHeight;
		const int32 Max16BitValue = FMath::Pow(2, 16) -1;
		const float HeightRangeRatio = Max16BitValue / TileInfo->HeightRange;
		
		for (int32 X = 0; X<512; X++)
		{
			for (int32 Y=0; Y <512; Y++)
			{
				const int32 Index = Y * 512 + X;
				const uint16 G16Height = (HeightArray[Index] - TileInfo->MinHeight) * HeightRangeRatio;
				TileInfo->HeightData[Index] = G16Height;
			}
		}
		
		Callback(TileInfo);
	});

	HttpRequest->SetURL(ImageURL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

FIntPoint IndexToUV(int32 Index, int32 TilesPerRow)
{
	int32 X = Index % TilesPerRow;
	int32 Y = (Index - X) / TilesPerRow;

	return { X, Y };
}

FIntPoint UVToIndex(FIntPoint UV, int32 TilesPerRow)
{
	return UV.Y * TilesPerRow + UV.X;
}

void FMapBoxUtils::DownloadTileSet(int32 X, int32 Y, int32 Zoom, int32 ZoomInLevels,
	TFunction<void(FMapBoxTileData*)> Callback)
{
	const int32 TilesPerRow = FMath::Pow(2, ZoomInLevels);
	const int32 PixelsPerRow = 512 * TilesPerRow;
	const int32 NewZoom = Zoom + ZoomInLevels;

	TArray<float>* HeightData = new TArray<float>();
	HeightData->SetNumUninitialized(PixelsPerRow * PixelsPerRow);
	
	int32* TotalTilesDownloaded = new int32();
	*TotalTilesDownloaded = 0;
	UE_LOG(LogTemp, Warning, TEXT("Original X, Y, Z: %d, %d, %d"), X, Y, Zoom);
	
	for (int32 U=0; U<TilesPerRow; U++)
	{
		for (int32 V=0; V<TilesPerRow; V++)
		{
			const int32 NewX = X * TilesPerRow + U;
			const int32 NewY = Y * TilesPerRow + V;
			UE_LOG(LogTemp, Warning, TEXT("New      X, Y, Z: %d, %d, %d"), NewX, NewY, NewZoom);
			DownloadTileRaw(NewX, NewY, NewZoom, [TilesPerRow, PixelsPerRow, U, V, NewX, NewY, NewZoom, HeightData, TotalTilesDownloaded, Callback](TArray<FColor> Pixels)
			{
				UE_LOG(LogTemp, Warning, TEXT("It's here: %d, %d, %d"), NewX, NewY, NewZoom);
				for (int32 TX=0; TX<512; TX++)
				{
					for (int32 TY=0; TY<512; TY++)
					{
						const int32 Index = TY * 512 + TX;
						const FColor Pixel = Pixels[Index];

						const int32 HeightPixelX = U * 512 + TX;
						const int32 HeightPixelY = V * 512 + TY;
						const int32 HeightPixelIndex = HeightPixelY * PixelsPerRow + HeightPixelX;

						//(*HeightData)[HeightPixelIndex] = U+V*2;
						(*HeightData)[HeightPixelIndex] = -10000 + ((Pixel.R * 256 * 256 + Pixel.G * 256 + Pixel.B) * 0.1);
						if (TY == 200)
						{
							//UE_LOG(LogTemp, Warning, TEXT("UV Data: %d, %d"), U, V);
							// UE_LOG(LogTemp, Warning, TEXT("HeightData: %d"), (*HeightData)[HeightPixelIndex])
						}
					}
				}

				*TotalTilesDownloaded += 1;
				if (*TotalTilesDownloaded == TilesPerRow * TilesPerRow)
				{
					FMapBoxTileData* CombinedTileData = new FMapBoxTileData();
					CombinedTileData->HeightData.SetNumUninitialized(PixelsPerRow * PixelsPerRow);
					
					CombinedTileData->MinHeight = FMath::Min(*HeightData);
					const float MaxHeight = FMath::Max(*HeightData);
					CombinedTileData->HeightRange = MaxHeight - CombinedTileData->MinHeight;
					const int32 Max16BitValue = FMath::Pow(2, 16) -1;
					const float HeightRangeRatio = Max16BitValue / CombinedTileData->HeightRange;

					UE_LOG(LogTemp, Warning, TEXT("Min Max Range: %f, %f, %f"), CombinedTileData->MinHeight, MaxHeight, CombinedTileData->HeightRange)
					
					for (int32 HX = 0; HX<PixelsPerRow; HX++)
					{
						for (int32 HY=0; HY <PixelsPerRow; HY++)
						{
							const int32 Index = HY * PixelsPerRow + HX;
							const uint16 G16Height = ((*HeightData)[Index] - CombinedTileData->MinHeight) * HeightRangeRatio;
							
							CombinedTileData->HeightData[Index] = G16Height;
							// if (HY == 200)
							// {
							// 	UE_LOG(LogTemp, Warning, TEXT("New HeightData: %d"), CombinedTileData->HeightData[Index])
							// }
						}
					}

					Callback(CombinedTileData);
				}
			});
		}
	}
}
