// Copyright (c) 2022 GDi4K. All Rights Reserved.

#include "TerrainMagicStyles.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

#if ENGINE_MAJOR_VERSION == 5
	#include "Styling/SlateStyleMacros.h"
#endif

#define RootToContentDir Style->RootToContentDir

#if ENGINE_MAJOR_VERSION == 4
	#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#endif

TSharedPtr<FSlateStyleSet> FTerrainMagicStyles::StyleInstance = nullptr;

void FTerrainMagicStyles::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FTerrainMagicStyles::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FTerrainMagicStyles::GetStyleSetName()
{
	static FName StyleSetName(TEXT("TerrainMagicStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FTerrainMagicStyles::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("TerrainMagic")->GetBaseDir() / TEXT("Resources"));

#if ENGINE_MAJOR_VERSION == 5
	Style->Set("TerrainMagicCommands.InvalidateLandscapeClipsAction", new IMAGE_BRUSH(TEXT("Icon_Invalidate_20"), Icon20x20));
	Style->Set("TerrainMagicCommands.TogglePreviewLandscapeClipsAction", new IMAGE_BRUSH(TEXT("Icon_TogglePreview_20"), Icon20x20));
#else
	Style->Set("TerrainMagicCommands.InvalidateLandscapeClipsAction", new IMAGE_BRUSH(TEXT("Icon_Invalidate_40"), Icon40x40));
	Style->Set("TerrainMagicCommands.TogglePreviewLandscapeClipsAction", new IMAGE_BRUSH(TEXT("Icon_TogglePreview_40"), Icon40x40));
#endif
	
	return Style;
}

void FTerrainMagicStyles::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FTerrainMagicStyles::Get()
{
	return *StyleInstance;
}

