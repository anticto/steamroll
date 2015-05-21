// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "JetStream.h"


// Sets default values
AJetStream::AJetStream()
{
	Power = 1000.f;
	MaxSpeed = 5000.f;
	TurnFactor = 10.f;

	StreamActive.SetNumZeroed(4);
	StreamActive[0] = true;
}

