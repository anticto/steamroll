// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"

float ASteamrollBall::StartDraggingTime = 3.f;

ASteamrollBall::ASteamrollBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Sphere = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("Sphere"));
	RootComponent = Sphere;

	Activated = false;
	Age = 0.f;
	Dragging = false;

	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;
}

void ASteamrollBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Age += DeltaSeconds;

	if (!Dragging && Age > StartDraggingTime)
	{
		Dragging = true;
		Sphere->SetLinearDamping(2.f);
		Sphere->SetAngularDamping(2.f);
	}
}

void ASteamrollBall::ActivateBall()
{
	Activated = true;
}

