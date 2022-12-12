// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Utils/MapUtils.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void FMapUtils::DownloadTileRaw(EMapSource Source, int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapTileResponseRaw>)> Callback)
{
	if (Source != MS_MAPBOX)
	{
		const TSharedPtr<FMapTileResponseRaw> TileResponseError = MakeShared<FMapTileResponseRaw>();
		TileResponseError->IsSuccess = false;
		TileResponseError->ErrorMessage = "Unsupported Map Source.";
		Callback(TileResponseError);
		return;
	}
	
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";
	const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);
	
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool HasCompleted)
	{
		if (!HasCompleted)
		{
			const TSharedPtr<FMapTileResponseRaw> TileResponseError = MakeShared<FMapTileResponseRaw>();
			TileResponseError->IsSuccess = false;
			
			if (Response.IsValid())
			{
				TileResponseError->ErrorMessage = "HTTP Response Error: " + Response->GetContentAsString();
			} else
			{
				TileResponseError->ErrorMessage = "Invalid HTTP Request or Network Failure";
			}

			Callback(TileResponseError);
			return;
		}

		if (Response->GetResponseCode() / 100 != 2)
		{
			const TSharedPtr<FMapTileResponseRaw> TileResponseError = MakeShared<FMapTileResponseRaw>();
			TileResponseError->IsSuccess = false;
			TileResponseError->ErrorMessage = "Invalid HTTP Response Code: " + FString::FromInt(Response->GetResponseCode());
			Callback(TileResponseError);
			return;
		}

		if (Response->GetContentLength() == 0)
		{
			const TSharedPtr<FMapTileResponseRaw> TileResponseError = MakeShared<FMapTileResponseRaw>();
			TileResponseError->IsSuccess = false;
			TileResponseError->ErrorMessage = "No data found in the response";
			Callback(TileResponseError);
			return;
		}
		
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

		ImageWrapper->SetCompressed(Response->GetContent().GetData(), Response->GetContentLength());
		TArray<uint8> RawImageData;
		constexpr ERGBFormat InFormat = ERGBFormat::BGRA;
		const bool HasFetchImageData = ImageWrapper->GetRaw(InFormat, 8, RawImageData);

		if (!HasFetchImageData)
		{
			const TSharedPtr<FMapTileResponseRaw> TileResponseError = MakeShared<FMapTileResponseRaw>();
			TileResponseError->IsSuccess = false;
			TileResponseError->ErrorMessage = "Corrupted tile data found.";
			Callback(TileResponseError);
			return;
		}

		const TSharedPtr<FMapTileResponseRaw> TileResponse = MakeShared<FMapTileResponseRaw>();
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

void FMapUtils::DownloadTileSet(EMapSource Source, const FMapTileQuery TileQuery, TFunction<void(TSharedPtr<FMapTileDownloadProgress>, TSharedPtr<FMapTileResponse>)> Callback)
{
	const int32 TilesPerRow = FMath::Pow(2.0, static_cast<float>(TileQuery.ZoomInLevels));
	const int32 PixelsPerRow = 512 * TilesPerRow;
	const int32 NewZoom = TileQuery.Zoom + TileQuery.ZoomInLevels;

	TSharedPtr<TArray<float>> HeightData= MakeShared<TArray<float>>();
	HeightData->SetNumUninitialized(PixelsPerRow * PixelsPerRow);

	TSharedPtr<int32> TotalTilesDownloaded = MakeShared<int32>();
	*TotalTilesDownloaded = 0;

	const TSharedPtr<bool> RequestCompleted = MakeShared<bool>();
	*RequestCompleted = false;
	
	for (int32 U=0; U<TilesPerRow; U++)
	{
		for (int32 V=0; V<TilesPerRow; V++)
		{
			const int32 NewX = TileQuery.X * TilesPerRow + U;
			const int32 NewY = TileQuery.Y * TilesPerRow + V;
			
			DownloadTileRaw(Source, NewX, NewY, NewZoom, [TilesPerRow, PixelsPerRow, U, V, NewX, NewY, NewZoom, HeightData, TotalTilesDownloaded, Callback, RequestCompleted](TSharedPtr<FMapTileResponseRaw> TileResponseRaw)
			{
				if (*RequestCompleted)
				{
					return;
				}

				if (!TileResponseRaw->IsSuccess)
				{
					const TSharedPtr<FMapTileDownloadProgress> DownloadProgress = MakeShared<FMapTileDownloadProgress>();
					DownloadProgress->TotalTiles = TilesPerRow * TilesPerRow;
					DownloadProgress->TilesDownloaded = *TotalTilesDownloaded;

					const TSharedPtr<FMapTileResponse> ErrorResponse = MakeShared<FMapTileResponse>();
					ErrorResponse->IsSuccess = false;
					ErrorResponse->ErrorMessage = TileResponseRaw->ErrorMessage;
					
					*RequestCompleted = false;

					Callback(DownloadProgress, ErrorResponse);
					
					return;
				}
				
				for (int32 TX=0; TX<512; TX++)
				{
					for (int32 TY=0; TY<512; TY++)
					{
						const int32 Index = TY * 512 + TX;
						const FColor Pixel = TileResponseRaw->RGBHeight[Index];

						const int32 HeightPixelX = U * 512 + TX;
						const int32 HeightPixelY = V * 512 + TY;
						const int32 HeightPixelIndex = HeightPixelY * PixelsPerRow + HeightPixelX;

						(*HeightData)[HeightPixelIndex] = -10000 + ((Pixel.R * 256 * 256 + Pixel.G * 256 + Pixel.B) * 0.1);
					}
				}

				*TotalTilesDownloaded += 1;

				const TSharedPtr<FMapTileDownloadProgress> DownloadProgress = MakeShared<FMapTileDownloadProgress>();
				DownloadProgress->TotalTiles = TilesPerRow * TilesPerRow;
				DownloadProgress->TilesDownloaded = *TotalTilesDownloaded;

				Callback(DownloadProgress, nullptr);

				if (*TotalTilesDownloaded == TilesPerRow * TilesPerRow)
				{
					const TSharedPtr<FMapTileResponse> CombinedTileData = MakeShared<FMapTileResponse>();
					CombinedTileData->HeightData.SetNumUninitialized(PixelsPerRow * PixelsPerRow);
					
					CombinedTileData->MinHeight = FMath::Min(*HeightData);
					const float MaxHeight = FMath::Max(*HeightData);
					CombinedTileData->HeightRange = MaxHeight - CombinedTileData->MinHeight;
					const int32 Max16BitValue = FMath::Pow(2.0, 16.0) - 1;
					const float HeightRangeRatio = Max16BitValue / CombinedTileData->HeightRange;

					for (int32 HX = 0; HX<PixelsPerRow; HX++)
					{
						for (int32 HY=0; HY <PixelsPerRow; HY++)
						{
							const int32 Index = HY * PixelsPerRow + HX;
							const uint16 G16Height = ((*HeightData)[Index] - CombinedTileData->MinHeight) * HeightRangeRatio;
							
							CombinedTileData->HeightData[Index] = G16Height;
						}
					}

					
					Callback(DownloadProgress, CombinedTileData);
				}
			});
		}
	}
}