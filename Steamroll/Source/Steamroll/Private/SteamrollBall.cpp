// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"
#include "Engine.h"
#include "SteamrollPlayerController.h"
#include "Kismet/KismetMathLibrary.h"

float ASteamrollBall::StartDraggingTime = 2.f;

ASteamrollBall::ASteamrollBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Sphere = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("Sphere"));
	RootComponent = Sphere;

	StoppingSpeed = 10;
	Activated = false;
	Age = 0.f;
	Dragging = false;

	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;
}


void ASteamrollBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	bool TouchingFloor = IsTouchingFloor();
	float SpeedSquared = GetVelocity().SizeSquared();

	if (HasSlotState(ESlotTypeEnum::SE_CONTACT))
	{
		TArray<AActor*> Actors;
		TArray<AActor *> ActorsToIgnore;
		ActorsToIgnore.Add(this);
		TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
		
		UKismetSystemLibrary::SphereOverlapActors_NEW(GetWorld(), this->GetActorLocation(), Sphere->GetUnscaledSphereRadius() + 50.f, ObjectTypes, nullptr, ActorsToIgnore, Actors);
		
		for (auto Actor : Actors)
		{
			if (InterfaceCast<IExplosionDestructibleInterface>(Actor))
			{
				ActivateBall();
			}
		}
	}

	if (!Activated && SpeedSquared < StoppingSpeed * StoppingSpeed && TouchingFloor)
	{
		ActivateBall();
	}
	else if (SpeedSquared < StoppingSpeed * StoppingSpeed)
	{
		StopBall();
	}

	Age += DeltaSeconds;

	if (!Dragging && Age > StartDraggingTime && TouchingFloor)
	{
		Dragging = true;
		Sphere->SetLinearDamping(3.f);
		Sphere->SetAngularDamping(3.f);
	}
	else if (!TouchingFloor)
	{
		Dragging = false;
		Age = 0.f;
		Sphere->SetLinearDamping(0.01f);
		Sphere->SetAngularDamping(0.f);
	}

	//// Uncoment for debug boxes
	//if (TouchingFloor)
	//{
	//	float r = Sphere->GetScaledSphereRadius();
	//	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(r, r, r), FColor::Green);
	//}
	//
	//if (Dragging)
	//{
	//	DrawDebug2DDonut(GetWorld(), GetTransform().ToMatrixWithScale(), Sphere->GetScaledSphereRadius(), Sphere->GetUnscaledSphereRadius() * 2.f, 20, FColor::Red);
	//}

	LastLoc = GetActorLocation();
}


void ASteamrollBall::ActivateBall_Implementation()
{
	Activated = true;
	StopBall();
}


void ASteamrollBall::StopBall()
{
	Sphere->PutRigidBodyToSleep();
	Sphere->SetAllPhysicsLinearVelocity(FVector(0.f, 0.f, 0.f));
	Sphere->SetAllPhysicsAngularVelocity(FVector(0.f, 0.f, 0.f));
}


TEnumAsByte<ESlotTypeEnum::SlotType> ASteamrollBall::GetSlotState(int32 SlotIndex)
{
	return SlotsConfig.Slots[SlotIndex - 1];
}


void ASteamrollBall::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.Slots[SlotIndex - 1] = SlotTypeEnum;
}


ASteamrollPlayerController* ASteamrollBall::GetLocalPlayerController()
{
	UWorld* World = GetWorld();

	return World ? Cast<ASteamrollPlayerController>(GEngine->GetFirstLocalPlayerController(World)) : nullptr;
}


bool ASteamrollBall::HasSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	return SlotsConfig.HasSlotState(SlotTypeEnum);
}


int32 ASteamrollBall::CountSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	return SlotsConfig.CountSlotState(SlotTypeEnum);
}


FTransform ASteamrollBall::GetBallFlattenedTransform(const FVector& LastLocation, float WallDeploymentAngle) const
{
	FVector Direction = GetActorLocation() - LastLocation;
	Direction.Z = 0.f;
	Direction.Normalize();

	FVector Location = GetActorLocation() - FVector(0.f, 0.f, Sphere->GetScaledSphereRadius() - 10.f);

	FRotator WallDeploymentRotation(0.f, WallDeploymentAngle, 0.f);
	Direction = WallDeploymentRotation.RotateVector(Direction);
	
	return FTransform(Direction.Rotation(), Location, FVector(1.f, 1.f, 1.f));
}


FTransform ASteamrollBall::GetBallAdjustedTransform() const
{
	FVector Location = GetActorLocation() - FVector(0.f, 0.f, Sphere->GetScaledSphereRadius() - 10.f);

	return FTransform(Location);
}


bool ASteamrollBall::IsTouchingFloor() const
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;
	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	return GetWorld()->LineTraceSingle(
		                               RV_Hit, //result
		                               GetActorLocation(), //start
		                               GetActorLocation() + FVector(0.f, 0.f, -Sphere->GetScaledSphereRadius() - 1.f), //end
		                               ECC_PhysicsBody, //collision channel
		                               RV_TraceParams
		                               );
}

