// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollGameMode.h"
#include "SteamrollBall.h"

ASteamrollGameMode::ASteamrollGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our ball
	DefaultPawnClass = ASteamrollBall::StaticClass();
}
