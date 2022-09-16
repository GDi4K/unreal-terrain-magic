// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "HeightChangeLandscapeClip.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/Material.h"
#include "Kismet/KismetRenderingLibrary.h"


float smoothstep (float edge0, float edge1, float x)
{
	if (x < edge0)
		return 0;

	if (x >= edge1)
		return 1;

	// Scale/bias into [0..1] range
	x = (x - edge0) / (edge1 - edge0);

	return x * x * (3 - 2 * x);
}

// Sets default values
AHeightChangeLandscapeClip::AHeightChangeLandscapeClip()
{
	RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), 2048, 2048, RTF_RGBA8);
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_RT_HeighChange_Landscape_Clip.M_RT_HeighChange_Landscape_Clip";
	UMaterial* SourceMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
	RenderTargetMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), SourceMaterial);
}

UMaterial* AHeightChangeLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_HeightChange.M_TM_LandscapeClip_HeightChange";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> AHeightChangeLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;

	UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), Cast<UTextureRenderTarget2D>(RenderTarget));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), Cast<UTextureRenderTarget2D>(RenderTarget), RenderTargetMaterial);
	
	MaterialParams.Push({"Texture", Texture});
	MaterialParams.Push({"HeightMultiplier", static_cast<float>(HeightMultiplier)});
	MaterialParams.Push({"SelectedBlendMode", static_cast<float>(BlendMode)});

	MaterialParams.Push({"HeightMapInputMin", HeightMapRange.InputMin});
	MaterialParams.Push({"HeightMapInputMax", HeightMapRange.InputMax});
	MaterialParams.Push({"HeightMapOutputMin", HeightMapRange.OutputMin});
	MaterialParams.Push({"HeightMapOutputMax", HeightMapRange.OutputMax});

	MaterialParams.Push({"HeightSaturation", HeightSaturation});

	MaterialParams.Push({"SelectedFadeMode", static_cast<float>(FadeMode)});
	MaterialParams.Push({"FadeSaturation", FadeSaturation});
	MaterialParams.Push({"FadeMaskSpan", FadeMaskSpan});
	
	return MaterialParams;
}

int AHeightChangeLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D AHeightChangeLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void AHeightChangeLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool AHeightChangeLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void AHeightChangeLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void AHeightChangeLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int AHeightChangeLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* AHeightChangeLandscapeClip::GetHeightMap() const
{
	return RenderTarget;
}

TArray<FLandscapeClipPaintLayerSettings> AHeightChangeLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}

void AHeightChangeLandscapeClip::DownloadTexture()
{
	const FString AccessToken = "pk.eyJ1IjoiYXJ1bm9kYSIsImEiOiJjbDgxNm0wM3QwNGN0M3VudW5pbHJzcHFoIn0.S9PCT354lP_MKHrWFqEbxQ";

	TArray<FString> Parts;
	TileInfoString.TrimStartAndEnd().ParseIntoArray(Parts, TEXT(","), true);
	checkf(Parts.Num() == 3, TEXT("TileInfo text is invalid!"))
	
	const int32 X = FCString::Atoi(*Parts[0].TrimStartAndEnd());;
	const int32 Y = FCString::Atoi(*Parts[1].TrimStartAndEnd());;
	const int32 Zoom = FCString::Atoi(*Parts[2].TrimStartAndEnd());;
	const FString ImageURL = FString::Printf(TEXT("https://api.mapbox.com/v4/mapbox.terrain-rgb/%d/%d/%d@2x.pngraw?access_token=%s"), Zoom, X, Y, *AccessToken);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	const int32 TileId = 1;
	HttpRequest->OnProcessRequestComplete().BindWeakLambda(this, [this, TileId](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handling TileId: %d"), TileId);
		HandleDownloadedImage(Request, Response, bWasSuccessful);
	});

	HttpRequest->SetURL(ImageURL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void AHeightChangeLandscapeClip::HandleDownloadedImage(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	const bool HasDownloaded = bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0;
	if (!HasDownloaded)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Download Completed!"));
	
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	ImageWrapper->SetCompressed(HttpResponse->GetContent().GetData(), HttpResponse->GetContentLength());
	TArray<uint8>* RawImageData = new TArray<uint8>();
	const ERGBFormat InFormat = ERGBFormat::BGRA;
	const bool HasFetchImageData = ImageWrapper->GetRaw(InFormat, 8, *RawImageData);

	if (HasFetchImageData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fetching Image Data is a Success!"))
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fetching Image Data has Failed!"))
	}

	SourceTexture = UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8);
	SourceTexture->CompressionSettings = TC_VectorDisplacementmap;
	SourceTexture->SRGB = 0;
	SourceTexture->AddToRoot();
	SourceTexture->Filter = TF_Bilinear;
	SourceTexture->UpdateResource();

	const TSharedPtr<TArray<FColor>> NewImageData = MakeShared<TArray<FColor>>();
	NewImageData->SetNumUninitialized(512 * 512 * 4);

	for (int32 X=0; X<512; X++)
	{
		for (int32 Y=0; Y<512; Y++)
		{
			const int32 Index = Y * 512 + X;
			const int8 R = (*RawImageData)[Index * 4 + 2];
			const int8 G = (*RawImageData)[Index * 4 + 1];
			const int8 B = (*RawImageData)[Index * 4 + 0];
			const int8 A = (*RawImageData)[Index * 4 + 3];

			const FColor Pixel = FColor(R, G, B, A);
			(*NewImageData)[Index] = Pixel;
		}
	}

	const TSharedPtr<FUpdateTextureRegion2D> UpdateRegionNew = MakeShared<FUpdateTextureRegion2D>(0, 0, 0, 0, 512, 512);
	constexpr int32 BytesPerPixel = 4;
	constexpr int32 BytesPerRow = 512 * BytesPerPixel;

	FColor* SourceDataPtr = NewImageData->GetData();
	uint8* SourceByteDataPtr = reinterpret_cast<uint8*>(SourceDataPtr);
	SourceTexture->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), UpdateRegionNew.Get(),
								  static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), SourceByteDataPtr);
}