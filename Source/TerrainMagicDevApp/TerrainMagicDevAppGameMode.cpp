// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainMagicDevAppGameMode.h"
#include "TerrainMagicDevAppCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATerrainMagicDevAppGameMode::ATerrainMagicDevAppGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
