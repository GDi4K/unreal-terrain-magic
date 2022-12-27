// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "TerrainMagicCommands.h"

#include "Clips/EarthLandscapeClip.h"
#include "InputCoreTypes.h"
#include "LandscapeClip.h"
#include "TerrainMagicManager.h"
#include "Engine/Selection.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Notifications/SNotificationList.h"
#define LOCTEXT_NAMESPACE "FTerrainMagicCommands"

void FTerrainMagicCommands::ShowSelectAnActorNotification()
{
	FNotificationInfo Info(LOCTEXT("SelectAnActor_Notification", "Select an actor & follow the action again."));
	Info.ExpireDuration = 10.0f;
	Info.WidthOverride = 300.0f;
	Info.Hyperlink = FSimpleDelegate();
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FTerrainMagicCommands::RegisterCommands()
{
	UI_COMMAND(InvalidateLandscapeClipsAction, "Invalidate", "Invalidate Landscape Clips", EUserInterfaceActionType::Button, FInputChord(EKeys::Q, false, false, true, false));
	CommandsList->MapAction(
		InvalidateLandscapeClipsAction,
		FExecuteAction::CreateRaw(this, &FTerrainMagicCommands::OnInvalidateLandscapeClips),
		FCanExecuteAction());

	UI_COMMAND(TogglePreviewLandscapeClipsAction, "Toogle Preview", "Toggle Landscape Clips Preview Mode", EUserInterfaceActionType::Button, FInputChord(EKeys::W, false, false, true, false));
	CommandsList->MapAction(
		TogglePreviewLandscapeClipsAction,
		FExecuteAction::CreateRaw(this, &FTerrainMagicCommands::OnTogglePreviewLandscapeClips),
		FCanExecuteAction());

	UI_COMMAND(DownloadTileAction, "Downlod Tile", "Download Tile", EUserInterfaceActionType::Button, FInputChord(EKeys::D, false, false, true, false));
	CommandsList->MapAction(
		DownloadTileAction,
		FExecuteAction::CreateRaw(this, &FTerrainMagicCommands::OnDownloadTile),
		FCanExecuteAction());
}

void FTerrainMagicCommands::OnInvalidateLandscapeClips() const
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	if(GEditor->GetSelectedActorCount() == 0)
	{
		ShowSelectAnActorNotification();
		return;
	}
	
	for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
	{
		const AActor* Actor = Cast<AActor>(*Iter);
		if (Actor)
		{
			ATerrainMagicManager* Manager = Cast<ATerrainMagicManager>(UGameplayStatics::GetActorOfClass(Actor->GetWorld(), ATerrainMagicManager::StaticClass()));
			Manager->InvalidateClips();
			break;
		}
	}
}

void FTerrainMagicCommands::OnTogglePreviewLandscapeClips() const
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	if(GEditor->GetSelectedActorCount() == 0)
	{
		ShowSelectAnActorNotification();
		return;
	}
	
	for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
	{
		const AActor* Actor = Cast<AActor>(*Iter);
		if (Actor)
		{
			ATerrainMagicManager* Manager = Cast<ATerrainMagicManager>(UGameplayStatics::GetActorOfClass(Actor->GetWorld(), ATerrainMagicManager::StaticClass()));
			Manager->TogglePreview();
			break;
		}
	}
}

void FTerrainMagicCommands::OnDownloadTile() const
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);
	
	for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
	{
		AEarthLandscapeClip* Clip = Cast<AEarthLandscapeClip>(*Iter);
		if (Clip)
		{
			Clip->DownloadTile([](const FEarthTileDownloadStatus Status)
			{
				if (Status.IsError)
				{
					UE_LOG(LogTemp, Error, TEXT("Tile Download Error: %s"), *Status.ErrorMessage);
					
					FNotificationInfo Info(LOCTEXT("SpawnNotification_Notification", "Tile Download Error"));
					Info.ExpireDuration = 10.0f;
					Info.WidthOverride = 300.0f;
					Info.Hyperlink = FSimpleDelegate();
					Info.Hyperlink.BindLambda([]() {});
					Info.HyperlinkText = FText::FromString("Check Output Log for details");
					FSlateNotificationManager::Get().AddNotification(Info);
				}
			});
			UE_LOG(LogTemp, Display, TEXT("Downloding Tile: %s"), *Clip->GetName())
		}
	}
}

#undef LOCTEXT_NAMESPACE
