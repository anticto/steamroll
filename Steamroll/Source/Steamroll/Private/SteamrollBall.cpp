// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"
#include "Engine.h"
#include "SteamrollPlayerController.h"


ASteamrollBall::ASteamrollBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP), DraggingBall()
{
	Sphere = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("Sphere"));
	RootComponent = Sphere;

	DraggingBallSetSphere(Sphere);

	Activated = false;

	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;

	CollectedQuantity = 0.f;
	CollectCapacity = 10.f;

	bExplosionBlockedByContactSlot = false;

	TimeForNextPaint = 1.f;
}


void ASteamrollBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	bool bTouchingFloor = IsTouchingFloor();
	float SpeedSquared = GetVelocity().SizeSquared();

	if (HasSlotState(ESlotTypeEnum::SE_PAINT))
	{
		if (!IsActivated() && SpeedSquared >= StoppingSpeed * StoppingSpeed && bTouchingFloor)
		{
			TimeForNextPaint -= DeltaSeconds;

			if (TimeForNextPaint <= 0.f)
			{
				SplatPaint();
				TimeForNextPaint = 1.f;
			}
		}
	}

	if (HasSlotState(ESlotTypeEnum::SE_CONTACT))
	{
		bExplosionBlockedByContactSlot = true;

		TArray<AActor*> Actors;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);
		TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
		
		//UKismetSystemLibrary::SphereOverlapActors_NEW(GetWorld(), this->GetActorLocation(), Sphere->GetUnscaledSphereRadius() + 50.f, ObjectTypes, nullptr, ActorsToIgnore, Actors);
		Sphere->GetOverlappingActors(Actors);

		for (auto Actor : Actors)
		{
			if (InterfaceCast<IExplosionDestructibleInterface>(Actor))
			{
				ActivateBall();
				bExplosionBlockedByContactSlot = false;
				break;
			}
		}
	}

	DraggingBallTick(DeltaSeconds, bTouchingFloor, SpeedSquared);

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
	//GetWorldTimerManager().ClearTimer(this, &ASteamrollBall::Timeout1);  // In case the timer slot was set and somehow the ball was activated before it fired (remote activation for example)
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
	return SlotsConfig.GetSlotType(SlotIndex);
}


void ASteamrollBall::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.SetSlotType(SlotIndex, SlotTypeEnum);
}


ASteamrollPlayerController* ASteamrollBall::GetLocalPlayerController()
{
	UWorld* World = GetWorld();

	return World ? Cast<ASteamrollPlayerController>(GEngine->GetFirstLocalPlayerController(World)) : nullptr;
}


bool ASteamrollBall::HasSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	return SlotsConfig.HasSlotType(SlotTypeEnum);
}


int32 ASteamrollBall::CountSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	return SlotsConfig.CountSlotType(SlotTypeEnum);
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


bool ASteamrollBall::IsActivated()
{
	return Activated;
}


void ASteamrollBall::DraggingBallActivate()
{
	ActivateBall();
}


void ASteamrollBall::DraggingBallStop()
{
	StopBall();
}


void ASteamrollBall::Timeout1()
{
	ActivateTimerTrigger(1);
}


void ASteamrollBall::Timeout2()
{
	ActivateTimerTrigger(2);
}


void ASteamrollBall::Timeout3()
{
	ActivateTimerTrigger(3);
}


void ASteamrollBall::Timeout4()
{
	ActivateTimerTrigger(4);
}


void ASteamrollBall::BeginPlay()
{
	// Copy the slot configuration from the player controller
	UWorld *World = GetWorld();

	if (GEngine && World)
	{
		ASteamrollPlayerController* PlayerController = Cast<ASteamrollPlayerController>(GEngine->GetFirstLocalPlayerController(World));

		if (PlayerController)
		{
			SlotsConfig = PlayerController->SlotsConfig;
		}
	}

	Super::BeginPlay();	

	// If there are timer slots, start timers
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_TIME)
		{
			float Timeout = SlotsConfig.GetSlotParam(i, 1) * 10.f;

			switch (i)
			{
			case 1: GetWorldTimerManager().SetTimer(this, &ASteamrollBall::Timeout1, Timeout, false); break;
			case 2: GetWorldTimerManager().SetTimer(this, &ASteamrollBall::Timeout2, Timeout, false); break;
			case 3: GetWorldTimerManager().SetTimer(this, &ASteamrollBall::Timeout3, Timeout, false); break;
			case 4: GetWorldTimerManager().SetTimer(this, &ASteamrollBall::Timeout4, Timeout, false); break;
			}
		}
	}
}


void ASteamrollBall::WakeBall()
{
	DraggingBallReset();
}


void ASteamrollBall::SplatPaint_Implementation()
{
	
}


void ASteamrollBall::ActivateTimerTrigger(int32 SlotIndex)
{
	ActivateConnectedSlots(SlotIndex);
}


void ASteamrollBall::ActivateRemoteTriggers()
{
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_REMOTE)
		{
			ActivateConnectedSlots(i);
		}
	}
}


void ASteamrollBall::ActivateStopTriggers()
{
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_STOP)
		{
			ActivateConnectedSlots(i);
		}
	}
}


void ASteamrollBall::ActivateConnectedSlots(int32 SlotIndex)
{
	//if (IsTouchingFloor())
	//{
		for (uint32 i = 1; i < 5; i++)
		{
			if (SlotsConfig.GetSlotConnection(SlotIndex, i))
			{
				ActivateSlot(i);
			}
		}
	//}
}


void ASteamrollBall::ActivateSlot(int32 SlotIndex)
{
	if (!SlotsConfig.IsSlotUsed(SlotIndex))
	{
		SlotsConfig.SetSlotUsed(SlotIndex);

		ActivateSlotEvent(SlotsConfig.GetSlotType(SlotIndex), SlotsConfig.GetSlotParam(SlotIndex, 1), SlotsConfig.GetSlotParam(SlotIndex, 2));
	}
}

