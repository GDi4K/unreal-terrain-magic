// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "SplineLandscapeClip.h"
#include "Materials/Material.h"


void ASplineLandscapeClip::ReloadTextureIfNeeded()
{
	if (HasTextureReloaded)
	{
		return;
	}
	HasTextureReloaded = true;

	if (IsValid(G16Texture))
	{
		HeightMap = G16Texture->LoadCachedTexture();
	}
}

// Sets default values
ASplineLandscapeClip::ASplineLandscapeClip()
{
}

UMaterial* ASplineLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/M_TM_LandscapeClip_Generic.M_TM_LandscapeClip_Generic";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> ASplineLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;
	
	MaterialParams.Push({"Texture", HeightMap});
	
	float HeightMultiplierInput = (HeightMap == nullptr)? 0.0f : HeightMultiplier;
	MaterialParams.Push({"HeightMultiplier", HeightMultiplierInput});
	MaterialParams.Push({"SelectedBlendMode", static_cast<float>(BlendMode)});

	MaterialParams.Push({"HeightMapInputMin", HeightMapRange.InputMin});
	MaterialParams.Push({"HeightMapInputMax", HeightMapRange.InputMax});
	MaterialParams.Push({"HeightMapOutputMin", HeightMapRange.OutputMin});
	MaterialParams.Push({"HeightMapOutputMax", HeightMapRange.OutputMax});
	MaterialParams.Push({"HeightSaturation", HeightSaturation});
	MaterialParams.Push({"HeightAddition", GetHeightAddition()});

	MaterialParams.Push({"SelectedFadeMode", static_cast<float>(FadeMode)});
	MaterialParams.Push({"FadeSaturation", FadeSaturation});
	MaterialParams.Push({"FadeMaskSpan", FadeMaskSpan});
	MaterialParams.Push({"FadeUseRemapMin", static_cast<float>(FadeMinimum)});
	
	return MaterialParams;
}

int ASplineLandscapeClip::GetHeightMultiplier() const
{
	return HeightMultiplier;
}

FVector2D ASplineLandscapeClip::GetClipBaseSize() const
{
	return HeightMapBaseSize;
}

void ASplineLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	HeightMapBaseSize = BaseSize;
}

bool ASplineLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void ASplineLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void ASplineLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int ASplineLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* ASplineLandscapeClip::GetHeightMap() const
{
	return HeightMap;
}

TArray<FLandscapeClipPaintLayerSettings> ASplineLandscapeClip::GetPaintLayerSettings() const
{
	return PaintLayerSettings;
}

void ASplineLandscapeClip::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GetWorld()->WorldType != EWorldType::Editor)
	{
		return;
	}

	ReloadTextureIfNeeded();
}

void ASplineLandscapeClip::Draw()
{
	if(G16Texture == nullptr)
	{
		// TODO: Change the TextureWidth as needed
		G16Texture = UG16Texture::Create(this, 2048, "/Game/TerrainMagic/HeightMaps/Spline/", GetName());
	}
	
	TArray<uint16>* HeightData = new TArray<uint16>();
	HeightData->SetNumZeroed(2048 * 2048);

	G16Texture->Update(HeightData->GetData(), [this, HeightData](UTexture2D* Texture)
	{
		HeightMap = Texture;
		delete HeightData;
	});
}
