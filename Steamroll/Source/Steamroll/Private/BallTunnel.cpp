// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "BallTunnel.h"
#include "SteamrollBall.h"
#include "TimerManager.h"
#include "SteamrollSphereComponent.h"


ABallTunnel::ABallTunnel(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Root = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	RootComponent = Root;

	Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	Mesh->AttachTo(RootComponent);

	TriggerVolume = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("TriggerVolume"));
	TriggerVolume->AttachTo(RootComponent);

	ConnectedTunnel = nullptr;
	bDiscovered = false;
	TransportDelaySeconds = 1.f;
	SpeedMultiplier = 1.f;
}


void ABallTunnel::TransportToOtherTunnelEnd(AActor* OtherActor)
{
	if (ConnectedTunnel)
	{
		ASteamrollBall* Ball = Cast<ASteamrollBall>(OtherActor);

		if (Ball)
		{
			bDiscovered = true;

			Ball->SetActorHiddenInGame(true);
			Ball->SetActorEnableCollision(false);
			Ball->Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Ball->Sphere->SetCollisionProfileName(FName("NoCollision"));

			float Speed = Ball->GetVelocity().Size();
			Ball->Sphere->bPhysicsEnabled = false;

			auto Delegate = FTimerDelegate::CreateUFunction(Ball, FName("ExecuteTransport"), ConnectedTunnel, Speed);
			GetWorldTimerManager().SetTimer(Delegate, TransportDelaySeconds, false);
	}
	}
}

