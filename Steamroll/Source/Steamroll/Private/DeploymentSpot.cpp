// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "DeploymentSpot.h"


ADeploymentSpot::ADeploymentSpot(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	AimingStep = 22.5f;
	NumSlots = 4;
}

void ADeploymentSpot::NextCamera()
{
	CameraListActiveIndex++;

	if (CameraListActiveIndex >= CameraList.Num())
	{
		CameraListActiveIndex = 0;
	}
}

