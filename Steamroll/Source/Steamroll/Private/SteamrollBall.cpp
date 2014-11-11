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
	NumFramesCollidingWithBall = 0;
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

	bool bHit = GetWorld()->LineTraceSingle(
		                               RV_Hit, //result
		                               GetActorLocation(), //start
		                               GetActorLocation() + FVector(0.f, 0.f, -2.f * Sphere->GetScaledSphereRadius()), //end at twice the radius
		                               ECC_PhysicsBody, //collision channel
		                               RV_TraceParams
		                               );

	// If hit is at half or less the ray length (twice the radius) the ball is touching the floor
	if (bHit && RV_Hit.Time < 0.51f)
	{
		float HitDotProduct = RV_Hit.ImpactNormal | FVector(1.f, 0.f, 0.f); // Dot product with horizontal vector

		if (FMath::Abs(HitDotProduct) < 0.1f) 
		{
			return true; // Only return true if the hit surface is more or less horizontal
		}
	}

	return false;
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


void ASteamrollBall::ExplosionEvent_Implementation()
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
	const static uint32 NumIterations = 5;
	const static float DepenetrationSpeed = 1000.f;
	const static float DragCoefficient = 0.4f;

	if (!bTouchingFloor)
	{
		// Only add gravity if not touching the floor and don't add velocity proportional to DeltaSeconds if the ball is stopped as it may produce jumps on balls resting on the floor if framerate is very low
		Velocity.Z += Velocity.Z == 0.f ? -0.1f : GetWorld()->GetGravityZ() * DeltaSeconds; 
	}
	else
	{
		//DrawDebugSphere(GetWorld(), GetActorLocation(), Sphere->GetScaledSphereRadius() + 0.1f, 15, FColor::Green, false, 0.0f);
		if (Velocity.Z < 0.1f)
		{
			Velocity.Z = 0.f;
		}
	}

	float Speed = Velocity.Size();
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = CurrentLocation + Velocity * DeltaSeconds;	

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	FHitResult OutHit;
	LastCollidedActor = nullptr;
	ASteamrollBall* CollidedWithBallThisFrame = nullptr;	

	float RemainingTime = DeltaSeconds;	

	for (uint32 Iteration = 0; Iteration < NumIterations && RemainingTime > 0.f; ++Iteration)
	{
		if (UKismetSystemLibrary::SphereTraceSingle_NEW(GetWorld(), CurrentLocation, NewLocation, Sphere->GetScaledSphereRadius(), UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody), true, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true))
		{
			LastCollidedActor = &*OutHit.Actor;	
			
			float TravelTime = OutHit.Time * RemainingTime;
			RotateBall(Velocity, Speed, TravelTime);
			RemainingTime = RemainingTime - TravelTime;
			CurrentLocation = OutHit.Location;// -Velocity.SafeNormal() * 0.001f;
			SetActorLocation(CurrentLocation);

			// Activate steamball by contact trigger
			if (HasSlotState(ESlotTypeEnum::SE_EXPL) && HasSlotState(ESlotTypeEnum::SE_CONTACT) && LastCollidedActor->GetClass()->ImplementsInterface(UExplosionDestructibleInterface::StaticClass()))
			{
				ExplosionEvent();
			}

			ASteamrollBall* OtherBall = Cast<ASteamrollBall>(LastCollidedActor);

			if (OtherBall) // Collided with another ball
			{
				CollidedWithBallThisFrame = OtherBall;
				// Check the balls do not intersect, if they do push this one away from the other
				FVector PushVector = CurrentLocation - OtherBall->GetActorLocation();
				float Dist = PushVector.Size();

				if (Dist < Sphere->GetScaledSphereRadius() + OtherBall->Sphere->GetScaledSphereRadius())
				{
					PushVector = Dist > 0.f ? PushVector / Dist : FVector::UpVector;
					SeparateBalls(OtherBall, PushVector, DepenetrationSpeed, DeltaSeconds);
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

				float Restitution = OutHit.PhysMaterial->Restitution;
				Velocity = (V1t + V2n) * Restitution;
				OtherBall->SetVelocity((V2t + V1n) * Restitution);
				
				// Activate the other steamball by contact trigger if needed
				if (OtherBall->HasSlotState(ESlotTypeEnum::SE_EXPL) && OtherBall->HasSlotState(ESlotTypeEnum::SE_CONTACT))
				{
					OtherBall->ExplosionEvent();
				}

				break;
			}
			else
			{			
				//if (Iteration > 0)
				//{
				//	ActorsToIgnore.Pop();
				//}

				ActorsToIgnore.Push(LastCollidedActor);

				FVector ReflectedVector = (NewLocation - CurrentLocation).MirrorByVector(OutHit.ImpactNormal);
				ReflectedVector.Normalize();

				float ImpactVelocityProjection = FVector::DotProduct(OutHit.ImpactNormal, Velocity);
				Velocity = ReflectedVector * Speed;
				Velocity = Velocity + OutHit.ImpactNormal * ImpactVelocityProjection * (1.f - OutHit.PhysMaterial->Restitution);
			}

			NewLocation = CurrentLocation + Velocity * RemainingTime;
		}
		else
		{
			RotateBall(Velocity, Speed, RemainingTime);
			SetActorLocation(NewLocation); // No collision, so the ball can travel all the way
			break;
		}

		if (Iteration == NumIterations - 1)
		{
			// Too many iterations so the collision situation might be too complicated to solve with the current velocity, let the ball drop freely
			Velocity = FVector::ZeroVector;
			DrawDebugSphere(GetWorld(), GetActorLocation(), Sphere->GetScaledSphereRadius() + 0.1f, 20, FColor::Red, false, 0.5f);
			break;
		}
	}	

	Velocity = Velocity - Velocity * DragCoefficient * DeltaSeconds; // Drag equation

	if (FVector2D(Velocity.X, Velocity.Y).SizeSquared() < FMath::Square(0.1f)) // Stop horizontal movement if very low speed
	{
		Velocity.X = 0.f;
		Velocity.Y = 0.f;
	}

	// Try to push away balls that are resting on each other
	NumFramesCollidingWithBall = CollidedWithBallThisFrame ? NumFramesCollidingWithBall + 1 : 0;

	if (NumFramesCollidingWithBall > 2)
	{
		Velocity = FVector::ZeroVector;
		NumFramesCollidingWithBall = 0;

		if (CollidedWithBallThisFrame)
		{
			CollidedWithBallThisFrame->SetVelocity(FVector::ZeroVector);
			CollidedWithBallThisFrame->NumFramesCollidingWithBall = 0;

			FVector PushVector = GetActorLocation() - CollidedWithBallThisFrame->GetActorLocation();
			PushVector.Normalize();
			SeparateBalls(CollidedWithBallThisFrame, PushVector, DepenetrationSpeed, DeltaSeconds);			
		}		
	}

	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Velocity=%s"), *(Velocity).ToString()));
	//UE_LOG(LogTemp, Warning, TEXT("Velocity=%s"), *(Velocity).ToString());
}


void ASteamrollBall::SeparateBalls(ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds)
{
	DeltaSeconds = 0.001f;
	SetActorLocation(GetActorLocation() + PushVector * DepenetrationSpeed * DeltaSeconds, true);
	OtherBall->SetActorLocation(OtherBall->GetActorLocation() - PushVector * DepenetrationSpeed * DeltaSeconds, true);
}


void ASteamrollBall::RotateBall(FVector& Velocity, float Speed, float DeltaSeconds)
{
	// Make ball rotate in the movement direction only if there's significat horizontal movement
	float HorizontalSpeed = FVector2D(Velocity.X, Velocity.Y).Size();

	if (HorizontalSpeed > 0.1f)
	{
		FVector RotationVector = FVector::CrossProduct(Velocity / Speed, FVector(0.f, 0.f, 1.f)); // Compute rotation axis
		RotationVector.Normalize();
		float NumRotations = HorizontalSpeed * DeltaSeconds / (2.f * PI * Sphere->GetScaledSphereRadius()); // Distance travelled / Sphere circumference
		float RotationAngle = -360.f * NumRotations; // Convert Rotations to degrees

		FVector RotatedForwardVector = FVector(0.f, 0.f, 1.f).RotateAngleAxis(RotationAngle, RotationVector);
		FQuat Quat = FQuat::FindBetween(FVector(0.f, 0.f, 1.f), RotatedForwardVector);
		Sphere->AddWorldRotation(Quat.Rotator());
	}
}

