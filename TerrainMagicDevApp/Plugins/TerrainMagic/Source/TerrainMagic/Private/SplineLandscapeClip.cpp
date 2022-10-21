// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "SplineLandscapeClip.h"

#include "TerrainMagicManager.h"
#include "Materials/Material.h"
#include "Utils/TerrainMagicThreading.h"

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

void ASplineLandscapeClip::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property->GetName() == "Center")
	{
		Draw();
	}
}

void ASplineLandscapeClip::Draw()
{
	// TODO: Change the TextureWidth as needed
	constexpr int32 TextureWidth = 2048;
	
	if(G16Texture == nullptr || TextureWidth != G16Texture->GetTextureWidth())
	{
		G16Texture = UG16Texture::Create(this, TextureWidth, "/Game/TerrainMagic/HeightMaps/Spline/", GetName());
	}

	if (DrawCounter.GetValue() > 0)
	{
		bNeedsToDrawAgain = true;
		return;
	}

	DrawCounter.Increment();

	FTerrainMagicThreading::RunOnAnyBackgroundThread([this, TextureWidth]()
	{
		TArray<uint16>* HeightData = new TArray<uint16>();
		HeightData->SetNumZeroed(TextureWidth * TextureWidth);
		const int32 Max16Bit = FMath::Pow(2, 16) - 1;

		for (int32 X=0; X<TextureWidth; X++)
		{
			for (int32 Y=0; Y<TextureWidth; Y++)
			{
				FVector2D UV = {X/static_cast<float>(TextureWidth), Y/static_cast<float>(TextureWidth)};
				const int32 Index = (Y * TextureWidth) + X;

				float Distance = 1.0f - FMath::Clamp((UV - Center).Size() * 2.0f, 0.0f, 1.0f);
				Distance = smoothstep(0.0f, 1.0f, Distance);
				(*HeightData)[Index] = Distance * Max16Bit;
			}
		}

		FTerrainMagicThreading::RunOnGameThread([this, HeightData]()
		{
			G16Texture->UpdateOnly(HeightData->GetData(), [this, HeightData](UTexture2D* Texture)
			{
				HeightMap = Texture;
				delete HeightData;
				FTerrainMagicThreading::RunOnGameThread([this]()
				{
					DrawCounter.Decrement();
					ATerrainMagicManager::EnsureManager(GetWorld())->ClipsAreDirty();
					if (bNeedsToDrawAgain)
					{
						bNeedsToDrawAgain = false;
						Draw();
					}
				});
			});
		});
	});
	
	

	
}
