// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"
#include "Engine.h"
#include "SteamrollPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UnrealMathUtility.h"
#include "TrajectoryComponent.h"


ASteamrollBall::ASteamrollBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP), DraggingBall()
{
	Sphere = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("Sphere"));
	RootComponent = Sphere;

	TrajectoryComponent = PCIP.CreateDefaultSubobject<UTrajectoryComponent>(this, TEXT("TrajectoryComponent"));
	TrajectoryComponent->AttachTo(RootComponent);

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
	bSimulationBall = false;
}


void ASteamrollBall::Tick(float DeltaSeconds)
{
	if (bSimulationBall)
	{
		return;
	}

	Super::Tick(DeltaSeconds);

	bool bTouchingFloor = IsTouchingFloor();
	UpdateBallPhysics(DeltaSeconds);

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
	RV_TraceParams.bTraceComplex = false;
	RV_TraceParams.bTraceAsyncScene = false;
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


void ASteamrollBall::UpdateBallPhysics(float DeltaSeconds)
{
	UpdateBallPhysics(*this, DeltaSeconds);
}


void ASteamrollBall::AddLocation(const FVector& Location)
{
	if (bSimulationBall)
	{
		TrajectoryComponent->SimulatedLocations.Add(Location);
	}
}


void ASteamrollBall::UpdateBallPhysics(ASteamrollBall& Ball, float DeltaSecondsUnsubdivided)
{
	/** How many collision tests are going to be made per subtick */
	const static uint32 NumIterations = 10;
	const static float MaxDeltaSeconds = 1.f / 30.f;
	const static float DepenetrationSpeed = 1000.f;
	const static float DragCoefficient = 0.4f;

	FVector& Velocity = Ball.Velocity;
	
	float DeltaSeconds = DeltaSecondsUnsubdivided;

	if (DeltaSecondsUnsubdivided > MaxDeltaSeconds)
	{
		DeltaSeconds = MaxDeltaSeconds;
	}

	float CurrentTime = 0.f;

	while (CurrentTime <= DeltaSecondsUnsubdivided)
	{
		bool bTouchingFloor = Ball.IsTouchingFloor();

		if (!bTouchingFloor)
		{
			Velocity.Z += Ball.GetWorld()->GetGravityZ() * DeltaSeconds;
		}
		else
		{
			//DrawDebugSphere(GetWorld(), GetActorLocation(), Sphere->GetScaledSphereRadius() + 0.1f, 15, FColor::Green, false, 0.0f);
			if (Velocity.Z < 0.1f)
			{
				Velocity.Z = 0.f;
			}
		}

		//Velocity.Z = 0.f;

		float Speed = Velocity.Size();
		FVector CurrentLocation = Ball.GetActorLocation();
		Ball.AddLocation(CurrentLocation);
		FVector NewLocation = CurrentLocation + Velocity * DeltaSeconds;

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(&Ball);
		FHitResult OutHit;
		Ball.LastCollidedActor = nullptr;
		ASteamrollBall* CollidedWithBallThisFrame = nullptr;

		float RemainingTime = DeltaSeconds;

		for (uint32 Iteration = 0; Iteration < NumIterations && RemainingTime > 0.f; ++Iteration)
		{
			if (UKismetSystemLibrary::SphereTraceSingle_NEW(Ball.GetWorld(), CurrentLocation, NewLocation, Ball.Sphere->GetScaledSphereRadius(), UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody), true, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true))
			{
				if (OutHit.Time == 0.f && OutHit.Location == OutHit.ImpactPoint)
				{
					if (Iteration == NumIterations - 3)
					{
						ActorsToIgnore.Add(&*OutHit.Actor); // We are probably stuck on an object Iteration is very high, ignore the problematic object
					}

					// Warning! Hack to discard some degenerate collisions that sometimes ue4 returns where the above condition is satistied (impossible, as the location of a sphere cannot be the impact point and on top of that the time of impact be zero)
					//ActorsToIgnore.Add(&*OutHit.Actor);
					if (OutHit.bStartPenetrating && OutHit.PenetrationDepth > 0.f)
					{
						CurrentLocation += OutHit.Normal * OutHit.PenetrationDepth;						
					}
					else
					{
						CurrentLocation -= Velocity.SafeNormal() * 0.01f;
					}

					Ball.SetActorLocation(CurrentLocation);
					Ball.AddLocation(CurrentLocation);

					continue;
				}

				Ball.LastCollidedActor = &*OutHit.Actor;

				float TravelTime = OutHit.Time * RemainingTime;
				Velocity = DragPhysics(Velocity, TravelTime, DragCoefficient);
				
				if (!Ball.bSimulationBall)
				{
					Ball.RotateBall(Velocity, Speed, TravelTime);
				}

				RemainingTime = RemainingTime - TravelTime;
				CurrentLocation = OutHit.Location + OutHit.ImpactNormal * 0.1f;

				Ball.SetActorLocation(CurrentLocation);
				Ball.AddLocation(CurrentLocation);

				// Activate steamball by contact trigger
				if (Ball.HasSlotState(ESlotTypeEnum::SE_EXPL) && Ball.HasSlotState(ESlotTypeEnum::SE_CONTACT)
					&& Ball.LastCollidedActor->GetClass()->ImplementsInterface(UExplosionDestructibleInterface::StaticClass()))
				{
					if (!Ball.bSimulationBall)
					{
						Ball.ExplosionEvent();
					}
				}

				ASteamrollBall* OtherBall = Cast<ASteamrollBall>(Ball.LastCollidedActor);

				if (OtherBall) // Collided with another ball
				{
					CollidedWithBallThisFrame = OtherBall;
					// Check the balls do not intersect, if they do push this one away from the other
					FVector PushVector = CurrentLocation - OtherBall->GetActorLocation();
					float Dist = PushVector.Size();

					if (Dist < Ball.Sphere->GetScaledSphereRadius() + OtherBall->Sphere->GetScaledSphereRadius())
					{
						PushVector = Dist > 0.f ? PushVector / Dist : FVector::UpVector;
						Ball.SeparateBalls(OtherBall, PushVector, DepenetrationSpeed, DeltaSeconds);
					}

					FVector& V1 = Velocity;
					FVector V2 = OtherBall->GetVelocity();
					FVector& N = OutHit.ImpactNormal;
					N.Normalize();

					FVector V1n = (V1 | N) * N; // Normal Velocity
					FVector V1t = V1 - V1n; // Tangential velocity

					FVector V2n = (V2 | N) * N;
					FVector V2t = V2 - V2n;

					float Restitution = OutHit.PhysMaterial->Restitution;
					Velocity = (V1t + V2n) * Restitution;
					Speed = Velocity.Size();

					if (!Ball.bSimulationBall)
					{
						OtherBall->SetVelocity((V2t + V1n) * Restitution);
					}

					// Activate the other steamball by contact trigger if needed
					if (OtherBall->HasSlotState(ESlotTypeEnum::SE_EXPL) && OtherBall->HasSlotState(ESlotTypeEnum::SE_CONTACT))
					{
						if (!Ball.bSimulationBall)
						{
							OtherBall->ExplosionEvent();
						}
					}

					break;
				}
				else
				{
					//ActorsToIgnore.Push(Ball.LastCollidedActor);
					CollidedWithBallThisFrame = nullptr;
					
					FVector ReflectedVector = Velocity.MirrorByVector(OutHit.ImpactNormal);
					ReflectedVector.Normalize();
					Speed = Velocity.Size();

					//DrawDebugDirectionalArrow(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 100.f, 3.f, FColor::Yellow);
					//DrawDebugDirectionalArrow(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + ReflectedVector * 100.f, 3.f, FColor::Green);
					//DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Itr=%d, Speed=%f"), Iteration, Speed), nullptr, FColor::White, 0.f);
					
					Velocity = ReflectedVector * Speed;
					FVector VelocityN = OutHit.ImpactNormal * (Velocity | OutHit.ImpactNormal);
					FVector VelocityT = Velocity - VelocityN;
					Velocity = VelocityT + VelocityN * OutHit.PhysMaterial->Restitution;
					Speed = Velocity.Size();
				}

				NewLocation = CurrentLocation + Velocity * RemainingTime;
			}
			else
			{				
				CollidedWithBallThisFrame = nullptr;

				if (!Ball.bSimulationBall)
				{
					Ball.RotateBall(Velocity, Speed, RemainingTime);
				}

				Velocity = DragPhysics(Velocity, RemainingTime, DragCoefficient);
				Ball.SetActorLocation(NewLocation); // No collision, so the ball can travel all the way
				Ball.AddLocation(NewLocation);

				//if(SimulationLocations) DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Full Itr=%d, Speed=%f"), Iteration, Velocity.Size()), nullptr, FColor::White, 0.f);

				break;
			}

			if (Iteration == NumIterations - 1)
			{
				// Too many iterations so the collision situation might be too complicated to solve with the current velocity, let the ball drop freely
				Velocity = FVector::ZeroVector;

				if (Ball.bSimulationBall)
				{
					DrawDebugSphere(Ball.GetWorld(), Ball.GetActorLocation(), Ball.Sphere->GetScaledSphereRadius() + 0.1f, 20, FColor::Magenta, false, 0.f);
					//DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Iteration Limit!")), nullptr, FColor::Red, 0.f);
				}

				break;
			}
		}		

		if (FVector2D(Velocity.X, Velocity.Y).SizeSquared() < FMath::Square(0.1f)) // Stop horizontal movement if very low speed
		{
			Velocity.X = 0.f;
			Velocity.Y = 0.f;

			if (Ball.bSimulationBall && FMath::Abs(Velocity.Y) < 0.1f)
			{
				return; // Stop simulation if we are stopped
			}
		}

		// Try to push away balls that are resting on each other
		Ball.NumFramesCollidingWithBall = CollidedWithBallThisFrame ? Ball.NumFramesCollidingWithBall + 1 : 0;

		if (Ball.NumFramesCollidingWithBall > 2)
		{
			Velocity = FVector::ZeroVector;
			Ball.NumFramesCollidingWithBall = 0;

			if (CollidedWithBallThisFrame)
			{
				if (!Ball.bSimulationBall)
				{
					CollidedWithBallThisFrame->SetVelocity(FVector::ZeroVector);
					CollidedWithBallThisFrame->NumFramesCollidingWithBall = 0;
				}

				FVector PushVector = Ball.GetActorLocation() - CollidedWithBallThisFrame->GetActorLocation();
				PushVector.Normalize();
				Ball.SeparateBalls(CollidedWithBallThisFrame, PushVector, DepenetrationSpeed, DeltaSeconds);

				CollidedWithBallThisFrame = nullptr;
			}
		}

		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Velocity=%s"), *(Velocity).ToString()));
		//UE_LOG(LogTemp, Warning, TEXT("Velocity=%s"), *(Velocity).ToString());

		DeltaSeconds = FMath::Min(DeltaSeconds, DeltaSecondsUnsubdivided - CurrentTime + 0.01f);
		CurrentTime += DeltaSeconds;
	}

	//if (SimulationLocations) DrawDebugString(Ball.GetWorld(), Ball.GetActorLocation(), FString::Printf(TEXT("Simulation end!")), nullptr, FColor::Red, 0.f);
}


FVector ASteamrollBall::DragPhysics(const FVector& Velocity, float TravelTime, float DragCoefficient)
{
	return Velocity - Velocity * DragCoefficient * TravelTime;
}


void ASteamrollBall::SeparateBalls(ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds)
{
	DeltaSeconds = 0.001f;
	SetActorLocation(GetActorLocation() + PushVector * DepenetrationSpeed * DeltaSeconds, true);
	AddLocation(GetActorLocation());
	
	if (!bSimulationBall)
	{
		OtherBall->SetActorLocation(OtherBall->GetActorLocation() - PushVector * DepenetrationSpeed * DeltaSeconds, true);
	}
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


void ASteamrollBall::DrawPhysicalSimulation()
{
	float Radius = Sphere->GetScaledSphereRadius();
	TArray<FVector>* SimulatedLocations = &TrajectoryComponent->SimulatedLocations;
	
	if (!SimulatedLocations || SimulatedLocations->Num() == 0)
	{
		return;
	}
	else if (SimulatedLocations->Num() == 1)
	{
		DrawDebugSphere(GetWorld(), SimulatedLocations->operator[](0), Radius, 10, FColor::Yellow, false, -1.f);
		return;
	}

	DrawDebugSphere(GetWorld(), SimulatedLocations->operator[](0), Radius, 10, FColor::Yellow, false, -1.f);

	uint32 Step = 4;

	for (int32 Itr = Step; Itr < SimulatedLocations->Num() - 1; Itr += Step)
	{
		//DrawDebugCylinder(GetWorld(), SimulatedLocations->operator[](Itr - Step), SimulatedLocations->operator[](Itr), Sphere->GetScaledSphereRadius(), 10, FColor::Blue, false, -1.f);
		DrawDebugLine(GetWorld(), SimulatedLocations->operator[](Itr - Step), SimulatedLocations->operator[](Itr), FColor::Blue, false, -1.f, 0, 2.f * Sphere->GetScaledSphereRadius());
		
		//DrawDebugSphere(GetWorld(), SimulatedLocations->operator[](Itr), Radius, 10, FColor::Red, false, -1.f);
	}
	
	DrawDebugSphere(GetWorld(), SimulatedLocations->operator[](SimulatedLocations->Num() - 1), Radius, 10, FColor::Yellow, false, -1.f);
}

