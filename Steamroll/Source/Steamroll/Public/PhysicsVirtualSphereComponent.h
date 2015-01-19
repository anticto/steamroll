// // Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "Components/SphereComponent.h"
#include "PhysicsVirtualSphereComponent.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API UPhysicsVirtualSphereComponent : public USphereComponent
{
	GENERATED_UCLASS_BODY()

	FVector Location, Velocity;

	void SteamrollTick(float DeltaSeconds, class USteamrollSphereComponent* RealSphere);

	void ReceiveHit(class USteamrollSphereComponent* RealSphere, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);
	
};
