// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "HeightChangeLandscapeClip.h"

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

	constexpr int TextureWidth = 2048;
	G16Texture = UTexture2D::CreateTransient(TextureWidth, TextureWidth, PF_G16);
	G16Texture->CompressionSettings = TC_VectorDisplacementmap;
	G16Texture->SRGB = 0;
	G16Texture->AddToRoot();
	G16Texture->Filter = TF_Bilinear;

#if WITH_EDITORONLY_DATA
	G16Texture->MipGenSettings = TMGS_NoMipmaps;
#endif
	
	G16Texture->UpdateResource();
	
	// Allocate Data
	constexpr  int BytesPerPixel = 2;
	constexpr int32 BufferSize = TextureWidth * TextureWidth * BytesPerPixel;
	SourceData = new uint8[BufferSize];
	uint16* SourceData16 = reinterpret_cast<uint16*>(SourceData);

	for (int X=0; X < TextureWidth; X++)
	{
		for (int Y=0; Y < TextureWidth; Y++)
		{
			double U = X / static_cast<float>(TextureWidth);
			double V = Y / static_cast<float>(TextureWidth);

			U = U - 0.5;
			V = V - 0.5;

			double distance = 1.0 - FMath::Sqrt(U*U + V*V);
			distance = FMath::Clamp(distance,0.0, 1.0);
			distance = smoothstep(0.0, 1.0, distance);
			
			const int Index = Y * TextureWidth + X;
			SourceData16[Index] = distance * 65535;
		}
	}
	
	
	WholeTextureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureWidth);
	constexpr int32 BytesPerRow = TextureWidth * BytesPerPixel;
	G16Texture->UpdateTextureRegions(static_cast<int32>(0), static_cast<uint32>(1), &WholeTextureRegion,
								  static_cast<uint32>(BytesPerRow), static_cast<uint32>(BytesPerPixel), SourceData);
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
	
	MaterialParams.Push({"Texture", G16Texture});
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
