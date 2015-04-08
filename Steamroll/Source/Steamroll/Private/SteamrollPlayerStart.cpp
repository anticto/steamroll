// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollPlayerStart.h"


ASteamrollPlayerStart::ASteamrollPlayerStart(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	GetCapsuleComponent()->SetCapsuleSize(200.f, 100.f);
	SteamPressure = 1.f;
}

