#include "Utils/MapBoxUtils.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void FMapBoxUtils::DownloadTileRaw(int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapBoxTileResponseRaw>)> Callback)
{
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";
	const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);
	
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

		TSharedPtr<FMapBoxTileResponseRaw> TileResponse = MakeShared<FMapBoxTileResponseRaw>();
		TileResponse->RGBHeight.SetNumUninitialized(512 * 512);
		
		for (int32 X=0; X<512; X++)
		{
			for (int32 Y=0; Y<512; Y++)
			{
				const int32 Index = Y * 512 + X;
				const int8 R = RawImageData[Index * 4 + 2];
				const int8 G = RawImageData[Index * 4 + 1];
				const int8 B = RawImageData[Index * 4 + 0];

				TileResponse->RGBHeight[Index] = FColor(R, G, B, 255);
			}
		}

		Callback(TileResponse);
	});

	HttpRequest->SetURL(ImageURL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void FMapBoxUtils::DownloadTileSet(const FMapBoxTileQuery TileQuery, TFunction<void(TSharedPtr<FMapBoxTileResponse>)> Callback)
{
	const int32 TilesPerRow = FMath::Pow(2, TileQuery.ZoomInLevels);
	const int32 PixelsPerRow = 512 * TilesPerRow;
	const int32 NewZoom = TileQuery.Zoom + TileQuery.ZoomInLevels;

	TSharedPtr<TArray<float>> HeightData= MakeShared<TArray<float>>();
	HeightData->SetNumUninitialized(PixelsPerRow * PixelsPerRow);

	TSharedPtr<int32> TotalTilesDownloaded = MakeShared<int32>();
	*TotalTilesDownloaded = 0;
	
	for (int32 U=0; U<TilesPerRow; U++)
	{
		for (int32 V=0; V<TilesPerRow; V++)
		{
			const int32 NewX = TileQuery.X * TilesPerRow + U;
			const int32 NewY = TileQuery.Y * TilesPerRow + V;
			
			DownloadTileRaw(NewX, NewY, NewZoom, [TilesPerRow, PixelsPerRow, U, V, NewX, NewY, NewZoom, HeightData, TotalTilesDownloaded, Callback](TSharedPtr<FMapBoxTileResponseRaw> TileResponseRaw)
			{
				UE_LOG(LogTemp, Warning, TEXT("It's here: %d, %d, %d"), NewX, NewY, NewZoom);
				for (int32 TX=0; TX<512; TX++)
				{
					for (int32 TY=0; TY<512; TY++)
					{
						const int32 Index = TY * 512 + TX;
						const FColor Pixel = TileResponseRaw->RGBHeight[Index];

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
					const TSharedPtr<FMapBoxTileResponse> CombinedTileData = MakeShared<FMapBoxTileResponse>();
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
