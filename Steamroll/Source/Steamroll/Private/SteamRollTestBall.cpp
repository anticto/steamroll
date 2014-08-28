// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamRollTestBall.h"

ASteamRollTestBall::ASteamRollTestBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Meshes/BallMesh.BallMesh"));

	// Create mesh component for the ball
	Ball = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Ball0"));
	Ball->SetStaticMesh(BallMesh.Object);
	Ball->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetAngularDamping(0.1f);
	Ball->SetLinearDamping(0.1f);
	Ball->BodyInstance.MassScale = 10.f;
	Ball->SetNotifyRigidBodyCollision(true);
	RootComponent = Ball;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> SteamParticleSystem(TEXT("/Game/Particles/P_Steam_Lit.P_Steam_Lit"));

	// Create steam particle system
	Steam = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Steam0"));
	Steam->SetTemplate(SteamParticleSystem.Object);
	Steam->bAutoActivate = false;
	Steam->AttachTo(RootComponent);
	//Steam->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));
	Steam->bAbsoluteRotation = true;

	Activated = false;

	PrimaryActorTick.bCanEverTick = true;
}


void ASteamRollTestBall::Tick(float DeltaSeconds)
{
	if (!Activated && GetVelocity().SizeSquared() < 100 * 100)
	{
		Steam->Activate(true);
		Activated = true;
	}
}

