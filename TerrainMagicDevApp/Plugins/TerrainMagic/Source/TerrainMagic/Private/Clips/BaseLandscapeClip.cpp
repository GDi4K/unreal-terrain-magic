// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Clips/BaseLandscapeClip.h"
#include "Materials/Material.h"


// Sets default values
ABaseLandscapeClip::ABaseLandscapeClip()
{
}

UMaterial* ABaseLandscapeClip::GetSourceMaterialForHeight() const
{
	const FName MaterialPath = "/TerrainMagic/Core/Materials/LandscapeClips/M_TM_LandscapeClip_Base.M_TM_LandscapeClip_Base";
	return Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialPath.ToString()));
}

TArray<FTerrainMagicMaterialParam> ABaseLandscapeClip::GetMaterialParams()
{
	TArray<FTerrainMagicMaterialParam> MaterialParams;

	MaterialParams.Push({"BaseValue", BaseValue});
	
	return MaterialParams;
}

int ABaseLandscapeClip::GetHeightMultiplier() const
{
	return 1.0;
}

FVector2D ABaseLandscapeClip::GetClipBaseSize() const
{
	return {0, 0};
}

void ABaseLandscapeClip::SetClipBaseSize(FVector2D BaseSize)
{
	
}

bool ABaseLandscapeClip::IsEnabled() const
{
	return bEnabled;
}

void ABaseLandscapeClip::SetEnabled(bool bEnabledInput)
{
	bEnabled = bEnabledInput;
}

void ABaseLandscapeClip::SetZIndex(int Index)
{
	ZIndex = Index;
}

int ABaseLandscapeClip::GetZIndex() const
{
	return ZIndex;
}

UTexture* ABaseLandscapeClip::GetHeightMap() const
{
	return nullptr;
}

TArray<FLandscapeClipPaintLayerSettings> ABaseLandscapeClip::GetPaintLayerSettings() const
{
	return {};
}

#if WITH_EDITOR
void ABaseLandscapeClip::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property->GetName() == "BaseValuePreset")
	{
		if (BaseValuePreset == BLCBV_ZERO)
		{
			BaseValue = 0.0;
		} else if (BaseValuePreset == BLCBV_ABSOLUTE_MINIMUM)
		{
			BaseValue = -32768.0;
			
		} else if (BaseValuePreset == BLCBV_ABSOLUTE_MAXIMUM)
		{
			BaseValue = 32767.0;
		}
	}
}
#endif