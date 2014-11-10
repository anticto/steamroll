// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"
#include "Engine.h"
#include "SteamrollPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UnrealMathUtility.h"


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

	Velocity = FVector(0.f);
}


void ASteamrollBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	bool bTouchingFloor = IsTouchingFloor();
	UpdateBallPhysics(DeltaSeconds, bTouchingFloor);

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
	FVector Direction = GetVelocity();
	Direction.Z = 0.f;
	Direction.Normalize();

	FVector Location = GetActorLocation() - FVector(0.f, 0.f, Sphere->GetScaledSphereRadius() - 0.5f);

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
	
	if (Instigator && GEngine && World)
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
				// Only activate the first available slot
				if (ActivateSlot(i)) break;
			}
		}
	//}
}


bool ASteamrollBall::ActivateSlot(int32 SlotIndex)
{
	TEnumAsByte<ESlotTypeEnum::SlotType> Type = GetSlotState(SlotIndex);
	bool bIsTrigger = Type == ESlotTypeEnum::SE_TIME || Type == ESlotTypeEnum::SE_CONTACT || Type == ESlotTypeEnum::SE_REMOTE || Type == ESlotTypeEnum::SE_STOP;

	if (!SlotsConfig.IsSlotUsed(SlotIndex) && !bIsTrigger)
	{
		SlotsConfig.SetSlotUsed(SlotIndex);
		ActivateSlotEvent(SlotsConfig.GetSlotType(SlotIndex), SlotsConfig.GetSlotParam(SlotIndex, 1), SlotsConfig.GetSlotParam(SlotIndex, 2));

		return true;
	}

	return false;
}


FVector ASteamrollBall::GetVelocity() const
{
	return Velocity;
}


void ASteamrollBall::SetVelocity(const FVector& NewVelocity)
{
	Velocity = NewVelocity;
}


void ASteamrollBall::UpdateBallPhysics(float DeltaSeconds, bool bTouchingFloor)
{
	/** How many collision tests are going to be made per tick */
	const static uint32 NumIterations = 10;
	const static float DepenetrationSpeed = 1000.f;

	TArray<AActor*> OverlappingActors;
	Sphere->GetOverlappingActors(OverlappingActors);

	if (!bTouchingFloor)
	{
		Velocity.Z += GetWorld()->GetGravityZ() * DeltaSeconds;
	}

	float Speed = Velocity.Size();
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = CurrentLocation + Velocity * DeltaSeconds;

	// Make ball rotate in the movement direction only if there's significat horizontal movement
	float HorizontalSpeed = FVector2D(Velocity.X, Velocity.Y).Size();

	if (HorizontalSpeed > 0.1f)
	{
		FVector RotationVector = FVector::CrossProduct(Velocity / Speed, FVector(0.f, 0.f, 1.f)); // Compute rotation axis
		RotationVector.Normalize();
		float NumRotations = HorizontalSpeed * DeltaSeconds / (2.f * PI * Sphere->GetScaledSphereRadius()); // Distance travelled / Sphere circumference
		float RotationAngle = -360.f * NumRotations;

		FVector RotatedForwardVector = FVector(0.f, 0.f, 1.f).RotateAngleAxis(RotationAngle, RotationVector);
		FQuat Quat = FQuat::FindBetween(FVector(0.f, 0.f, 1.f), RotatedForwardVector);
		Sphere->AddWorldRotation(Quat.Rotator());
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	FHitResult OutHit;
	LastCollidedActor = nullptr;

	float RemainingTime = DeltaSeconds;

	for (uint32 Iteration = 0; Iteration < NumIterations && RemainingTime > 0.f; ++Iteration)
	{
		if (UKismetSystemLibrary::SphereTraceSingle_NEW(GetWorld(), CurrentLocation, NewLocation, Sphere->GetScaledSphereRadius(), UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody), true, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true))
		{
			//DrawDebugLine(GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 100.f, FColor::Red);

			LastCollidedActor = &*OutHit.Actor;			
			
			RemainingTime = RemainingTime - OutHit.Time * RemainingTime;
			CurrentLocation = OutHit.Location;// -Velocity.SafeNormal() * 0.001f;
			SetActorLocation(CurrentLocation);

			ASteamrollBall* OtherBall = Cast<ASteamrollBall>(LastCollidedActor);

			if (OtherBall) // Collided with another ball
			{
				//Check the balls do not intersect
				FVector PushVector = CurrentLocation - OtherBall->GetActorLocation();
				float Dist = PushVector.Size();

				if (Dist < Sphere->GetScaledSphereRadius() + OtherBall->Sphere->GetScaledSphereRadius())
				{
					PushVector = Dist > 0 ? PushVector / Dist : FVector::UpVector;
					SetActorLocation(GetActorLocation() + PushVector * DepenetrationSpeed * DeltaSeconds);
				}

				FVector& V1 = Velocity;
				FVector V2 = OtherBall->GetVelocity();
				FVector& N = OutHit.ImpactNormal;
				//FVector N = OutHit.Location - OtherBall->GetActorLocation();
				N.Normalize();

				FVector V1n = (V1 | N) * N; // Normal Velocity
				FVector V1t = V1 - V1n; // Tangential velocity
				
				FVector V2n = (V2 | N) * N;
				FVector V2t = V2 - V2n;

				float Restitution = 0.8f;
				Velocity = (V1t + V2n) * Restitution;
				OtherBall->SetVelocity((V2t + V1n) * Restitution);
				
				// TODO: Activate steamball by contact trigger
				
				break;
			}
			else
			{				
				//ActorsToIgnore.Add(LastCollidedActor);
				if (Iteration > 0)
				{
					ActorsToIgnore.Pop();
				}

				ActorsToIgnore.Push(LastCollidedActor);

				FVector ReflectedVector = (NewLocation - CurrentLocation).MirrorByVector(OutHit.ImpactNormal);
				ReflectedVector.Normalize();
				//DrawDebugLine(GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + ReflectedVector * 100.f, FColor::Green);
				float Restitution = OutHit.PhysMaterial->Restitution;
				float ImpactVelocityProjection = FVector::DotProduct(OutHit.ImpactNormal, Velocity);
				Velocity = ReflectedVector * Speed;
				Velocity = Velocity + OutHit.ImpactNormal * ImpactVelocityProjection * (1.f - Restitution);
			}

			NewLocation = CurrentLocation + Velocity * RemainingTime;
		}
		else
		{
			SetActorLocation(NewLocation);
			break;
		}

		if (Iteration == NumIterations - 1)
		{
			// Too many iterations so the collision situation might be too complicated to solve with the current velocity, let the ball drop freely
			//Velocity = FVector::ZeroVector;
			DrawDebugSphere(GetWorld(), GetActorLocation(), Sphere->GetScaledSphereRadius() + 0.1f, 20, FColor::Red, false, 0.5f);
			break;
		}
	}

	Velocity = Velocity - Velocity * 0.4f * DeltaSeconds;

	if (HorizontalSpeed < 0.1f)
	{
		Velocity.X = 0.f;
		Velocity.Y = 0.f;
	}

	Sphere->UpdateOverlaps();
	Sphere->UpdatePhysicsVolume(true);

	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Velocity=%s"), *(Velocity).ToString()));
	//UE_LOG(LogTemp, Warning, TEXT("Velocity=%s"), *(Velocity).ToString());
}

