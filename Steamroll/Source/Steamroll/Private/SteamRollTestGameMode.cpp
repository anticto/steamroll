// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamRollTestGameMode.h"
#include "SteamRollTestBall.h"
#include "PlayerBase.h"

#include "Engine.h"

ASteamRollTestGameMode::ASteamRollTestGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our ball
	//DefaultPawnClass = ASteamRollTestBall::StaticClass();
	DefaultPawnClass = APlayerBase::StaticClass();
}


AActor* ASteamRollTestGameMode::ChoosePlayerStart(AController* Player)
{
	static int32 NextPlayerStartIndex = 0;

	bool bPIEStart = false;

	if (0 == PlayerStarts.Num())
		return NULL; // No player starts

	// Spawn at the next player start point
	APlayerStart* FoundPlayerStart = PlayerStarts[NextPlayerStartIndex % PlayerStarts.Num()];

	// Search for any PIE points
	for (int32 PlayerStartIndex = 0; PlayerStartIndex < PlayerStarts.Num(); ++PlayerStartIndex)
	{
		APlayerStart* PlayerStart = PlayerStarts[PlayerStartIndex];

		if (Cast<APlayerStartPIE>(PlayerStart) != NULL)
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			bPIEStart = true;
			break;
		}
	}

	if (!bPIEStart)
	{
		NextPlayerStartIndex++;
	}

	return FoundPlayerStart;
}
