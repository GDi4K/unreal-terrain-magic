// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once
#include "Engine/Texture.h"
#include "TerrainMagicMaterialParam.generated.h"

UENUM(BlueprintType)
enum ETerrainMagicMaterialParamType
{
	TMMP_SCALAR = 0 UMETA(DisplayName="Scalar"),
	TMMP_VECTOR = 1 UMETA(DisplayName="Vector"),
	TMMP_TEXTURE = 2 UMETA(DisplayName="Texture"),
};

USTRUCT(BlueprintType)
struct FTerrainMagicMaterialParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	TEnumAsByte<ETerrainMagicMaterialParamType> Type = TMMP_SCALAR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float ScalarValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	FVector VectorValue = {0, 0, 0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	UTexture* TextureValue = nullptr;
	

	FTerrainMagicMaterialParam() {}
	
	FTerrainMagicMaterialParam(const FName ParamName, float Value)
	{
		Name = ParamName;
		Type = TMMP_SCALAR;
		ScalarValue = Value;
	}

	FTerrainMagicMaterialParam(const FName ParamName, FVector Value)
	{
		Name = ParamName;
		Type = TMMP_VECTOR;
		VectorValue = Value;
	}

	FTerrainMagicMaterialParam(const FName ParamName, UTexture* Value)
	{
		Name = ParamName;
		Type = TMMP_TEXTURE;
		TextureValue = Value;
	}
};
