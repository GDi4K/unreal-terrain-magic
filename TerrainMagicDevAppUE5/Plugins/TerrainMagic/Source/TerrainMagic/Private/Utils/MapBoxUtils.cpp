// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Utils/MapBoxUtils.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void FMapBoxUtils::DownloadTileRaw(int32 X, int32 Y, int32 Zoom, TFunction<void(TSharedPtr<FMapBoxTileResponseRaw>)> Callback)
{
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";
	const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);
	
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool HasCompleted)
	{
		if (!HasCompleted)
		{
			const TSharedPtr<FMapBoxTileResponseRaw> TileResponseError = MakeShared<FMapBoxTileResponseRaw>();
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
			const TSharedPtr<FMapBoxTileResponseRaw> TileResponseError = MakeShared<FMapBoxTileResponseRaw>();
			TileResponseError->IsSuccess = false;
			TileResponseError->ErrorMessage = "Invalid HTTP Response Code: " + FString::FromInt(Response->GetResponseCode());
			Callback(TileResponseError);
			return;
		}

		if (Response->GetContentLength() == 0)
		{
			const TSharedPtr<FMapBoxTileResponseRaw> TileResponseError = MakeShared<FMapBoxTileResponseRaw>();
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
			const TSharedPtr<FMapBoxTileResponseRaw> TileResponseError = MakeShared<FMapBoxTileResponseRaw>();
			TileResponseError->IsSuccess = false;
			TileResponseError->ErrorMessage = "Corrupted tile data found.";
			Callback(TileResponseError);
			return;
		}

		const TSharedPtr<FMapBoxTileResponseRaw> TileResponse = MakeShared<FMapBoxTileResponseRaw>();
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

void FMapBoxUtils::DownloadTileSet(const FMapBoxTileQuery TileQuery, TFunction<void(TSharedPtr<FMapBoxTileDownloadProgress>, TSharedPtr<FMapBoxTileResponse>)> Callback)
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
			
			DownloadTileRaw(NewX, NewY, NewZoom, [TilesPerRow, PixelsPerRow, U, V, NewX, NewY, NewZoom, HeightData, TotalTilesDownloaded, Callback, RequestCompleted](TSharedPtr<FMapBoxTileResponseRaw> TileResponseRaw)
			{
				if (*RequestCompleted)
				{
					return;
				}

				if (!TileResponseRaw->IsSuccess)
				{
					const TSharedPtr<FMapBoxTileDownloadProgress> DownloadProgress = MakeShared<FMapBoxTileDownloadProgress>();
					DownloadProgress->TotalTiles = TilesPerRow * TilesPerRow;
					DownloadProgress->TilesDownloaded = *TotalTilesDownloaded;

					const TSharedPtr<FMapBoxTileResponse> ErrorResponse = MakeShared<FMapBoxTileResponse>();
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

				const TSharedPtr<FMapBoxTileDownloadProgress> DownloadProgress = MakeShared<FMapBoxTileDownloadProgress>();
				DownloadProgress->TotalTiles = TilesPerRow * TilesPerRow;
				DownloadProgress->TilesDownloaded = *TotalTilesDownloaded;

				Callback(DownloadProgress, nullptr);

				if (*TotalTilesDownloaded == TilesPerRow * TilesPerRow)
				{
					const TSharedPtr<FMapBoxTileResponse> CombinedTileData = MakeShared<FMapBoxTileResponse>();
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

void FMapBoxUtils::MakeG16Texture(int32 TextureWidth, uint16* HeightData, FString CacheTextureName, TFunction<void(UTexture2D*)> Callback)
{
#if WITH_EDITORONLY_DATA
	uint8* HeightDataBytes = reinterpret_cast<uint8*>(HeightData);

	const FString PackageName = TEXT("/Game/TerrainMagic/HeightMaps/") + CacheTextureName;
	UPackage* CacheTexturePackage = CreatePackage(*PackageName);
	CacheTexturePackage->FullyLoad();
	
	UTexture2D* CacheTexture = NewObject<UTexture2D>(CacheTexturePackage, *CacheTextureName, RF_Public | RF_Standalone);
	CacheTexture->CompressionSettings = TC_Grayscale;

#if ENGINE_MAJOR_VERSION == 5
	FTexturePlatformData* PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = TextureWidth;
	PlatformData->SizeY = TextureWidth;
	PlatformData->PixelFormat = EPixelFormat::PF_G16;
	CacheTexture->SetPlatformData(PlatformData);
#else
	CacheTexture->PlatformData = new FTexturePlatformData();
	CacheTexture->PlatformData->SizeX = TextureWidth;
	CacheTexture->PlatformData->SizeY = TextureWidth;
	CacheTexture->PlatformData->PixelFormat = EPixelFormat::PF_G16;
#endif
	
	#if WITH_EDITORONLY_DATA
		CacheTexture->MipGenSettings = TMGS_NoMipmaps;
	#endif
	
	if (CacheTexture != NULL)
	{
		CacheTexture->Filter = TextureFilter::TF_Bilinear;
		// Allocate first mipmap.
		const int32 NumBlocksX = TextureWidth / GPixelFormats[EPixelFormat::PF_G16].BlockSizeX;
		const int32 NumBlocksY = TextureWidth / GPixelFormats[EPixelFormat::PF_G16].BlockSizeY;
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		Mip->SizeX = TextureWidth;
		Mip->SizeY = TextureWidth;
	
		// Lock the texture so it can be modified
		Mip->BulkData.Lock(LOCK_READ_WRITE);
		Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[EPixelFormat::PF_G16].BlockBytes);
		Mip->BulkData.Unlock();
	
		// Save Asset;
		CacheTexture->AddToRoot();
		CacheTexture->Source.Init(TextureWidth, TextureWidth, 1, 1, ETextureSourceFormat::TSF_G16, HeightDataBytes);
		CacheTexture->UpdateResource();
		CacheTexturePackage->MarkPackageDirty();
	
		FAssetRegistryModule::AssetCreated(CacheTexture);
		CacheTexture->MarkPackageDirty();
		CacheTexture->PostEditChange();
		CacheTexturePackage->SetDirtyFlag(true);
	
		FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(CacheTexturePackage, CacheTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
	}
	
	// Create the Runtime Texture
	UTexture2D* Texture = UTexture2D::CreateTransient(TextureWidth, TextureWidth, PF_G16);
	Texture->CompressionSettings = TC_VectorDisplacementmap;
	Texture->SRGB = 0;
	Texture->AddToRoot();
	Texture->Filter = TF_Bilinear;
	
	Texture->MipGenSettings = TMGS_NoMipmaps;
	
	Texture->UpdateResource();

	const FUpdateTextureRegion2D* UpdateRegionNew = new FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureWidth);
	constexpr int32 BytesPerPixel = 2;
	const int32 BytesPerRow = TextureWidth * BytesPerPixel;

	Texture->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), UpdateRegionNew,
							  static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), HeightDataBytes,
							  [Callback, Texture](uint8*, const FUpdateTextureRegion2D*)
							  {
								  Callback(Texture);
							  });
#endif
}

UTexture2D* FMapBoxUtils::LoadCachedTexture(FString CacheTextureName)
{
	const FString TexturePath = "/Game/TerrainMagic/HeightMaps/" + CacheTextureName + "." + CacheTextureName;
	return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));
}
