// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollPlayerStart.h"


ASteamrollPlayerStart::ASteamrollPlayerStart(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	CapsuleComponent->SetCapsuleSize(200.f, 100.f);
}


