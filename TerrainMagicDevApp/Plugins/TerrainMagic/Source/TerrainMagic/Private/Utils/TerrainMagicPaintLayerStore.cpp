// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "Utils/TerrainMagicPaintLayerStore.h"

void FTerrainMagicPaintLayerStore::ProcessPaintLayer(FName LayerName, UTextureRenderTarget2D* RenderTarget)
{
	UE_LOG(LogTemp, Warning, TEXT("Layer Received: %s"), *LayerName.ToString())
}
