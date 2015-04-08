// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "DeploymentSpot.h"


ADeploymentSpot::ADeploymentSpot(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	AimingStep = 22.5f;
	NumSlots = 4;
	NumBalls = 1000;
	LaunchPowerFactor = 1.f;
}

void ADeploymentSpot::NextCamera()
{
	CameraListActiveIndex++;

	if (CameraListActiveIndex >= CameraList.Num())
	{
		CameraListActiveIndex = 0;
	}
}

