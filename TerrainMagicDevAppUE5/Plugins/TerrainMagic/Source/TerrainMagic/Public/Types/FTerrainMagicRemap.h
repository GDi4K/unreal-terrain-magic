// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once
#include "FTerrainMagicRemap.generated.h"

USTRUCT(BlueprintType)
struct FTerrainMagicRemap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float InputMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float InputMax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float OutputMin = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TerrainMagic")
	float OutputMax = 1;
};
