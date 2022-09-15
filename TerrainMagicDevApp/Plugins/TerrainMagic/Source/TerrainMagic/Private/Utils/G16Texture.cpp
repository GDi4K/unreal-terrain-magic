// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Utils/G16Texture.h"

#include "AssetRegistry/AssetRegistryModule.h"

void UG16Texture::InitTexture() const
{
	Texture->PlatformData = new FTexturePlatformData();
	Texture->PlatformData->SizeX = TextureWidth;
	Texture->PlatformData->SizeY = TextureWidth;
	Texture->PlatformData->PixelFormat = PF_G16;
	
	const int32 NumBlocksX = TextureWidth / GPixelFormats[PF_G16].BlockSizeX;
	const int32 NumBlocksY = TextureWidth / GPixelFormats[PF_G16].BlockSizeY;
	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Texture->PlatformData->Mips.Add(Mip);
	Mip->SizeX = TextureWidth;
	Mip->SizeY = TextureWidth;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_G16].BlockBytes);
	Mip->BulkData.Unlock();
	
	Texture->CompressionSettings = TC_Displacementmap;
	Texture->SRGB = 0;
	Texture->AddToRoot();
	Texture->Filter = TF_Bilinear;

#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif
	
	Texture->UpdateResource();
}

void UG16Texture::Init(const int32 InputTextureWidth)
{
	checkf(SourceData.Num() == 0, TEXT("Already initialized"))
	
	TextureWidth = InputTextureWidth;
	SourceData.SetNumZeroed(TextureWidth * TextureWidth);
}

UG16Texture* UG16Texture::MakeTransient(int32 InputTextureWidth)
{
	UG16Texture* G16Texture = NewObject<UG16Texture>();
	G16Texture->Init(InputTextureWidth);

	G16Texture->TextureName = NAME_None;
	G16Texture->Package = NewObject<UPackage>(nullptr, TEXT("/Engine/Transient"), RF_Transient);
	G16Texture->Package->AddToRoot();
	
	G16Texture->Texture = NewObject<UTexture2D>(G16Texture->Package, G16Texture->TextureName, RF_Transient);
	G16Texture->InitTexture();
	
	return G16Texture;
}

UG16Texture* UG16Texture::MakeSerializable(int32 InputTextureWidth, const FName InputTextureName)
{
	UG16Texture* G16Texture = NewObject<UG16Texture>();
	G16Texture->Init(InputTextureWidth);

	G16Texture->TextureName = InputTextureName;
	G16Texture->Package = NewObject<UPackage>(nullptr, TEXT("/Engine/Transient"), RF_Transient);
	G16Texture->Package->AddToRoot();
	
	G16Texture->Texture = NewObject<UTexture2D>(G16Texture->Package, G16Texture->TextureName, RF_Transient);
	G16Texture->InitTexture();
	
	return G16Texture;
}

UG16Texture* UG16Texture::MakePersistable(int32 InputTextureWidth, FName InputTextureName, FName TextureDirectory)
{
	UG16Texture* G16Texture = NewObject<UG16Texture>();
	G16Texture->Init(InputTextureWidth);

	G16Texture->TextureName = InputTextureName;
	G16Texture->PackageFilePath = TextureDirectory.ToString() + G16Texture->TextureName.ToString();
	G16Texture->Package = CreatePackage(*G16Texture->PackageFilePath);
	G16Texture->Package->FullyLoad();

	G16Texture->Texture = NewObject<UTexture2D>(G16Texture->Package, G16Texture->TextureName, RF_Public | RF_Standalone);
	G16Texture->InitTexture();

	G16Texture->Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(G16Texture->Texture);
	G16Texture->Texture->MarkPackageDirty();
	G16Texture->Texture->PostEditChange();
	G16Texture->Package->SetDirtyFlag(true);

	return G16Texture;
}

int32 UG16Texture::GetTextureWidth()
{
	return TextureWidth;
}

UTexture2D* UG16Texture::GetTexture()
{
	return Texture;
}

int32 UG16Texture::ReadPixel(const int32 X, const int32 Y)
{
	const int32 Index = Y * TextureWidth + X;
	return SourceData[Index];
}

void UG16Texture::WritePixel(const int32 X, const int32 Y, const int32 Value)
{
	checkf(Value < 65536, TEXT("Trying to write a value(%d) bigger than 16bit grayscale max"), Value);
	
	const int32 Index = Y * TextureWidth + X;
	SourceData[Index] = Value;
}

void UG16Texture::UpdateTexture()
{
	UpdateRegion = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureWidth);
	constexpr int32 BytesPerPixel = 2;
	const int32 BytesPerRow = TextureWidth * BytesPerPixel;

	uint16* SourceDataPtr = SourceData.GetData();
	uint8* SourceByteDataPtr = reinterpret_cast<uint8*>(SourceDataPtr);
	Texture->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), &UpdateRegion,
								  static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), SourceByteDataPtr);
}

