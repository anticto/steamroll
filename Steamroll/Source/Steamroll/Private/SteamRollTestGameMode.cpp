// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamRollTestGameMode.h"
#include "SteamRollTestBall.h"
#include "PlayerBase.h"

ASteamRollTestGameMode::ASteamRollTestGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our ball
	//DefaultPawnClass = ASteamRollTestBall::StaticClass();
	DefaultPawnClass = APlayerBase::StaticClass();
}
