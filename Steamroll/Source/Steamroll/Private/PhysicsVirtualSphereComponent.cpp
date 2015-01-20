// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "PhysicsVirtualSphereComponent.h"
#include "SteamrollSphereComponent.h"
#include "SteamrollBall.h"
#include "Engine.h"


UPhysicsVirtualSphereComponent::UPhysicsVirtualSphereComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Location = FVector::ZeroVector;
	Velocity = FVector::ZeroVector;
}


void UPhysicsVirtualSphereComponent::SteamrollTick(float DeltaSeconds, USteamrollSphereComponent* RealSphere)
{	
	SetWorldLocation(Location, false);
	SetPhysicsLinearVelocity(Velocity);

	Location = RealSphere->GetComponentLocation();
	Velocity = RealSphere->Velocity;
}


void UPhysicsVirtualSphereComponent::ReceiveHit(USteamrollSphereComponent* RealSphere, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Ball hit!: ")) += Other->GetName());
	//DrawDebugDirectionalArrow(GetWorld(), HitLocation, HitLocation + NormalImpulse, 10.f, FColor::Red, false, 5.f);
	//DrawDebugDirectionalArrow(GetWorld(), HitLocation, HitLocation + HitNormal * 1000.f, 10.f, FColor::Blue, false, 5.f);

	FVector ReflectedVector = RealSphere->Velocity.MirrorByVector(HitNormal);
	ReflectedVector.Normalize();
	float Speed = RealSphere->Velocity.Size();

	FVector Velocity = ReflectedVector * Speed;
	FVector VelocityN = HitNormal * (Velocity | HitNormal);
	FVector VelocityT = Velocity - VelocityN;
		
	FVector NewVelocity = VelocityT + (NormalImpulse / GetMass()) * Hit.PhysMaterial->Restitution;// -VelocityN;
	//FVector NewVelocity = Sphere->Velocity + (NormalImpulse / VirtualSphere->GetMass());

	RealSphere->Velocity = NewVelocity;
	SetPhysicsLinearVelocity(RealSphere->Velocity);
}

