// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Utils/G16Texture.h"

#include "IImageWrapper.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UG16Texture::Init()
{
	TextureData = new TArray<uint8>();
	TextureData->SetNumUninitialized(TextureWidth * TextureWidth * 2);
	
	RuntimeTexture = UTexture2D::CreateTransient(TextureWidth, TextureWidth, PF_G16);
	RuntimeTexture->CompressionSettings = TC_VectorDisplacementmap;
	RuntimeTexture->SRGB = 0;
	RuntimeTexture->AddToRoot();
	RuntimeTexture->Filter = TF_Bilinear;
#if WITH_EDITORONLY_DATA
	RuntimeTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
	
	RuntimeTexture->UpdateResource();
}

UG16Texture::UG16Texture()
{
	
}

UG16Texture::~UG16Texture()
{
	if (TextureData != nullptr)
	{
		TextureData->Empty();
		delete TextureData;
	}
}

int32 UG16Texture::GetTextureWidth() const
{
	return TextureWidth;
}

void UG16Texture::UpdateOnly(uint16* HeightData, TFunction<void(UTexture2D*)> Callback)
{
	if (RuntimeTexture == nullptr)
	{
		Init();
	}

	FMemory::Memcpy( TextureData->GetData(), HeightData, TextureWidth * TextureWidth * 2);
	
	constexpr int32 BytesPerPixel = 2;
	const int32 BytesPerRow = TextureWidth * BytesPerPixel;
	
	UpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureWidth);
	RuntimeTexture->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), &UpdateRegion,
							  static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), TextureData->GetData(),
							  [Callback, this](uint8*, const FUpdateTextureRegion2D*)
							  {
								  Callback(RuntimeTexture);
							  });
}

void UG16Texture::UpdateAndCache(uint16* HeightData, TFunction<void(UTexture2D*)> Callback)
{
	UpdateOnly(HeightData, Callback);
	CacheToDisk();
}

void UG16Texture::CacheToDisk() const
{
	if (TextureData == nullptr)
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	const FString PackageName = Directory + Filename;
	UPackage* CacheTexturePackage = CreatePackage(*PackageName);
	CacheTexturePackage->FullyLoad();
	
	UTexture2D* CacheTexture = NewObject<UTexture2D>(CacheTexturePackage, *Filename, RF_Public | RF_Standalone);
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
	
	CacheTexture->MipGenSettings = TMGS_NoMipmaps;
	
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
		CacheTexture->Source.Init(TextureWidth, TextureWidth, 1, 1, ETextureSourceFormat::TSF_G16, TextureData->GetData());
		CacheTexture->UpdateResource();
		CacheTexturePackage->MarkPackageDirty();
	
		FAssetRegistryModule::AssetCreated(CacheTexture);
		CacheTexture->MarkPackageDirty();
		CacheTexture->PostEditChange();
		CacheTexturePackage->SetDirtyFlag(true);
	
		FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(CacheTexturePackage, CacheTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
	}
#endif	
}

UTexture2D* UG16Texture::LoadCachedTexture() const
{
	const FString TexturePath = Directory + Filename + "." + Filename;
	return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));
}

UG16Texture* UG16Texture::Create(UObject* Outer, int32 TextureWidth, FString Directory, FString Filename)
{
	UG16Texture* Texture = NewObject<UG16Texture>(Outer);
	Texture->TextureWidth = TextureWidth;
	Texture->Directory = Directory;
	Texture->Filename = Filename;

	return Texture;
}