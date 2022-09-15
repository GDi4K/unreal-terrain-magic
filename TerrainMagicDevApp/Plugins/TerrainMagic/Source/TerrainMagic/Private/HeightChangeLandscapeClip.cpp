// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "HeightChangeLandscapeClip.h"

#include "AssetRegistry/AssetRegistryModule.h"
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

	const FName MapBoxTexturePath = "/TerrainMagic/HeightMaps/T_TM_height2_debris.T_TM_height2_debris";
	UTexture2D* MapBoxTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *MapBoxTexturePath.ToString()));
	SourceTexture = MapBoxTexture;
	SourceTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	SourceTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	SourceTexture->SRGB = false;
	SourceTexture->Filter = TF_Bilinear;
	SourceTexture->UpdateResource();

	const int32 TextureWidth = SourceTexture->GetSizeX();
	G16Texture = UG16Texture::MakeSerializable(TextureWidth, "Hello Texture");
	Texture = G16Texture->GetTexture();

	const FTexture2DMipMap& SourceTextureMip = SourceTexture->PlatformData->Mips[0];
	const uint8* SourceTextureData = static_cast<const uint8*>(SourceTextureMip.BulkData.LockReadOnly());
	const int32 Max16BitValue = FMath::Pow(2, 16) - 1;

	 for (int32 X = 0; X<SourceTexture->GetSizeX(); X++)
	 {
	 	for (int32 Y=0; Y <SourceTexture->GetSizeY(); Y++)
	 	{
	 		const int32 Index = Y * SourceTexture->GetSizeX() + X;
	 		const uint8 Height = SourceTextureData[Index];
	 		const double HeightNormalized = Height / 255.0;
	
	 		G16Texture->WritePixel(X, Y, HeightNormalized * Max16BitValue);
	 	}
	 }
	
	 SourceTextureMip.BulkData.Unlock();
	 G16Texture->UpdateTexture();

	// for (int X=0; X < TextureWidth; X++)
	// {
	// 	for (int Y=0; Y < TextureWidth; Y++)
	// 	{
	// 		double U = X / static_cast<float>(TextureWidth);
	// 		double V = Y / static_cast<float>(TextureWidth);
	//
	// 		U = U - 0.5;
	// 		V = V - 0.5;
	//
	// 		double Distance = 1.0 - FMath::Sqrt(U*U + V*V);
	// 		Distance = FMath::Clamp(Distance,0.0, 1.0);
	// 		Distance = (FMath::Sin(Distance * 40) + 1) / 2.0;
	// 		//Distance = smoothstep(0.0, 1.0, Distance);
	//
	// 		G16Texture->WritePixel(X, Y, Distance * 65535);
	// 	}
	// }
	//
	// G16Texture->UpdateTexture();
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

void AHeightChangeLandscapeClip::UpdateTexture()
{
	
}
