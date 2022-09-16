#include "Utils/MapBoxUtils.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void FMapBoxUtils::DownloadTile(int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapBoxTileData>)> Callback)
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

		const TSharedPtr<FMapBoxTileData> TileInfo = MakeShared<FMapBoxTileData>();
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
