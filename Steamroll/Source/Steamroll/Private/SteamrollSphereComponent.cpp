// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollSphereComponent.h"
#include "TrajectoryComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ExplosionDestructibleInterface.h"
#include "SteamrollBall.h"
#include "SlotsConfig.h"
#include "Engine.h"
#include "PlayerBase.h"
#include "BallTunnel.h"
#include "PhysicsVirtualSphereComponent.h"
#include "SteamrollPlayerController.h"
#include "PlayerBase.h"


USteamrollSphereComponent::USteamrollSphereComponent(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	TrajectoryComponent = PCIP.CreateDefaultSubobject<UTrajectoryComponent>(this, TEXT("TrajectoryComponent"));
	TrajectoryComponent->AttachTo(this);

	Velocity = FVector::ZeroVector;
	MaxInitialSpeed = 10000000.f;
	bPhysicsEnabled = true;
	RotationAxis = FVector::ZeroVector;
	NumFramesCollidingWithBall = 0;
	bSimulationBall = false;
	RemainingTime = 0.f;

	NumIterations = 10;
	MaxDeltaSeconds = 1.f / 60.f;
	StoppingSpeed = 50.f;
	DepenetrationSpeed = 1000.f;
	DragCoefficient = 0.2f;
	DragCoefficientSlow = 0.95f;
	DragCoefficientSlowSpeed = 500.f;
	DragConstantSlowSpeed = 10.f;

	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCueLoader(TEXT("SoundCue'/Game/So/BallBounce/BallBounceSoundCue.BallBounceSoundCue'"));
	BounceSoundCue = SoundCueLoader.Object;
}


float USteamrollSphereComponent::SteamrollTick(float DeltaSeconds)
{
	if (bSimulationBall)
	{
		return 0.f;
	}

	float TimeToSimulate = DeltaSeconds + RemainingTime;
	RemainingTime = UpdateBallPhysics(TimeToSimulate);

	return TimeToSimulate - RemainingTime;
}


float USteamrollSphereComponent::UpdateBallPhysics(float DeltaSecondsUnsubdivided)
{
	if (!bPhysicsEnabled)
	{
		return 0.f;
	}

	USteamrollSphereComponent& Ball = *this;
	FVector& Velocity = Ball.Velocity;
	AActor* SoundActor = GetAttachmentRootActor();
	ASteamrollBall* BallActor = Cast<ASteamrollBall>(SoundActor);
	float BallRadius = Ball.GetScaledSphereRadius();
	
	if (bSimulationBall)
	{
		ResetTimedSlots(BallActor);
	}

	ETraceTypeQuery TraceTypeQuery;
	ECollisionChannel CollisionChannel;

	if (Ball.bSimulationBall)
	{
		CollisionChannel = ECollisionChannel::ECC_GameTraceChannel4; // PredictionTraceChannel	
	}
	else
	{
		CollisionChannel = ECollisionChannel::ECC_GameTraceChannel7; // RealBallTraceChannel
	}

	TraceTypeQuery = UEngineTypes::ConvertToTraceType(CollisionChannel);

	float DeltaSeconds = MaxDeltaSeconds; // Constant timestep
	float CurrentTime = 0.f;

	while (CurrentTime <= DeltaSecondsUnsubdivided)
	{
		bool bTouchingFloor = IsTouchingFloor(CollisionChannel);

		Ball.DrawTimedSlots(BallActor, CurrentTime, Velocity);

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

		float Speed = Velocity.Size();
		FVector CurrentLocation = Ball.GetActorLocation();
		Ball.AddLocation(CurrentLocation, CurrentTime);
		Ball.ReduceVerticalVelocity(Velocity, bTouchingFloor, DeltaSeconds, CollisionChannel);
		FVector NewLocation = CurrentLocation + Velocity * DeltaSeconds;

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(Ball.GetAttachmentRootActor());
		FHitResult OutHit;
		Ball.LastCollidedActor = nullptr;
		ASteamrollBall* CollidedWithBallThisFrame = nullptr;

		float RemainingTime = DeltaSeconds;

		for (uint32 Iteration = 0; Iteration < NumIterations && RemainingTime > 0.f; ++Iteration)
		{
			bool bCollision = UKismetSystemLibrary::SphereTraceSingle_NEW(Ball.GetWorld(), CurrentLocation, NewLocation, BallRadius, TraceTypeQuery, true, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true);
			ASteamrollBall* OtherBall;
				
			OtherBall = bCollision ? Cast<ASteamrollBall>(&*OutHit.Actor) : nullptr;

			if (OutHit.Actor != nullptr && OutHit.GetComponent()->GetCollisionProfileName() == FName("DynamicPhysics"))
			{
				DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("?")), nullptr, FColor::Red, 0.f);
			}		
			
			if (bCollision)
			{				
				// Check for tunnels
				auto BallTunnel = Cast<ABallTunnel>(&*OutHit.Actor);
				
				if (BallActor && BallTunnel && BallTunnel->ConnectedTunnel)
				{						
					if ((BallTunnel->TriggerVolume->GetComponentLocation() - OutHit.Location).Size() < BallRadius + BallTunnel->TriggerVolume->GetScaledSphereRadius())
					{
						CurrentLocation = BallTunnel->ConnectedTunnel->GetActorLocation();
						Speed = Velocity.Size() * BallTunnel->ConnectedTunnel->SpeedMultiplier;
						Velocity = BallTunnel->ConnectedTunnel->Mesh->GetUpVector() * Speed;
						Ball.SetActorLocation(CurrentLocation);

						if (Ball.bSimulationBall)
						{
							if (!BallTunnel->bDiscovered)
							{
								DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("?")), nullptr, FColor::Red, 0.f);
								SetLastPredictionTime(BallActor, CurrentTime);
								return DeltaSecondsUnsubdivided - CurrentTime; // Stop simulation if we hit an undiscovered tunnel
							}

							
							TrajectoryComponent->CutTrajectory();
							Ball.AddLocation(CurrentLocation, CurrentTime);
						}
						else
						{
							// Return 0.f unused time to spend all the time in the current timestep, to make reality and simulation coincide
							BallActor->VirtualSphere->SetWorldLocation(NewLocation);
							BallActor->VirtualSphere->SetPhysicsLinearVelocity(Velocity);
							BallTunnel->TransportToOtherTunnelEnd(BallActor);
						}

						// Set RemainingTime = 0.f to spend all the time in the current timestep, to make reality and simulation coincide since they are handled differently (simulation continues simulating while reality will stop TransportDelaySeconds and then restart)
						// No need to set TravelTime or NewLocation because as we have ended the current iteration, they will be reset in the next one
						// float TravelTime = OutHit.Time * RemainingTime;
						RemainingTime = 0.f;
						// NewLocation = CurrentLocation + Velocity * TravelTime;

						continue;
					}
				}
				
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
						CurrentLocation -= Velocity.GetSafeNormal() * 0.01f;
					}

					Ball.SetActorLocation(CurrentLocation);
					Ball.AddLocation(CurrentLocation, CurrentTime);

					continue;
				}

				Ball.LastCollidedActor = &*OutHit.Actor;

				float TravelTime = OutHit.Time * RemainingTime;
				Velocity = DragPhysics(Velocity, TravelTime);

				if (!Ball.bSimulationBall)
				{
					Ball.RotateBall(Velocity, Speed, TravelTime);
				}

				RemainingTime = RemainingTime - TravelTime;
				CurrentLocation = OutHit.Location + OutHit.ImpactNormal * 0.1f;

				Ball.SetActorLocation(CurrentLocation);
				Ball.AddLocation(CurrentLocation, CurrentTime);

				// Activate steamball by contact trigger
				HandleImpactSlots(BallActor, Ball.LastCollidedActor, Velocity, CurrentTime);

				if (OtherBall) // Collided with another ball
				{
					CollidedWithBallThisFrame = OtherBall;
					// Check the balls do not intersect, if they do push this one away from the other
					FVector PushVector = CurrentLocation - OtherBall->GetActorLocation();
					float Dist = PushVector.Size();

					if (Dist < BallRadius + OtherBall->Sphere->GetScaledSphereRadius())
					{
						PushVector = Dist > 0.f ? PushVector / Dist : FVector::UpVector;
						Ball.SeparateBalls(OtherBall, PushVector, DepenetrationSpeed, DeltaSeconds, CurrentTime);
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
					if (!Ball.bSimulationBall)
					{
						for (int32 i = 1; i < 5; ++i)
						{
							if (OtherBall->GetSlotState(i) == ESlotTypeEnum::SE_EXPL && OtherBall->GetSlotActivatorType(i) == ESlotTypeEnum::SE_CONTACT)
							{
								OtherBall->ExplosionEvent();
								break;
							}
						}
					}

					break;
				}
				else
				{
					//ActorsToIgnore.Push(Ball.LastCollidedActor);
					CollidedWithBallThisFrame = nullptr;

					FVector ImpactRadiusVector = (OutHit.Location - OutHit.ImpactPoint).GetSafeNormal(); // In fact this is the ball's normal					
					float Aux = OutHit.ImpactNormal | ImpactRadiusVector;
					
					if (FMath::Abs(OutHit.ImpactNormal | ImpactRadiusVector) < 1.f && FMath::Abs(OutHit.ImpactPoint.Z - OutHit.Location.Z) > BallRadius / 2.f)
					{
						// Impacted against corner, use an interpolated "corner" normal
						OutHit.ImpactNormal = ImpactRadiusVector;
						//DrawDebugSphere(Ball.GetWorld(), Ball.GetActorLocation(), Ball.GetScaledSphereRadius() + 0.1f, 20, FColor::Green, false, 0.f);
					}
					
					FVector ReflectedVector = Velocity.MirrorByVector(OutHit.ImpactNormal);
					ReflectedVector.Normalize();
					Speed = Velocity.Size();
					FVector OldVelocity = Velocity;
					float OldSpeed = Speed;

					if (BallActor && Speed > 0.f && ((ReflectedVector | (Velocity / Speed)) < 0.f) && (FMath::Abs(OutHit.ImpactNormal | FVector::UpVector) < 0.1f))
					{
						if (OutHit.Actor != nullptr && OutHit.Actor->GetName().StartsWith("Paret_Nivell"))
						{
							ActivateSnapRamp(BallActor, OutHit.ImpactPoint, OutHit.ImpactNormal, CurrentTime);
						}
					}

					//DrawDebugDirectionalArrow(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 100.f, 3.f, FColor::Yellow);
					//DrawDebugDirectionalArrow(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + ReflectedVector * 100.f, 3.f, FColor::Green);
					//DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Itr=%d, Speed=%f"), Iteration, Speed), nullptr, FColor::White, 0.f);
					//DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.Actor->GetName(), nullptr, FColor::White, 0.f);

					Velocity = ReflectedVector * Speed;
					FVector VelocityN = OutHit.ImpactNormal * (Velocity | OutHit.ImpactNormal);
					FVector VelocityT = Velocity - VelocityN;

					// If impact normal is not flat or vertical, that is, we hit a ramp, set restitution to 0 to cancel normal velocity and snap ball to the ramp to prevent it from rebounding hard and flying away
					float Restitution = OutHit.PhysMaterial->Restitution;

					if (FMath::IsWithin(OutHit.ImpactNormal | FVector::UpVector, 0.1f, 0.9f))
					{
						VelocityT = VelocityT.GetSafeNormal() * Speed;
						Restitution = 0.f;
					}

					Velocity = VelocityT + VelocityN * Restitution;
					Speed = Velocity.Size();

					if (!bSimulationBall && OldSpeed > 0.f && Speed > 0.f && FMath::Acos(FVector::DotProduct(OldVelocity / OldSpeed, Velocity / Speed)) > FMath::DegreesToRadians(20.f))
					{
						PlayBounceSound(SoundActor, OldSpeed / MaxInitialSpeed);
					}
				}

				Ball.ReduceVerticalVelocity(Velocity, bTouchingFloor, DeltaSeconds, CollisionChannel);
				NewLocation = CurrentLocation + Velocity * RemainingTime;
			}
			else
			{
				CollidedWithBallThisFrame = nullptr;

				if (!Ball.bSimulationBall)
				{
					Ball.RotateBall(Velocity, Speed, RemainingTime);
				}

				Velocity = DragPhysics(Velocity, RemainingTime);
				Ball.SetActorLocation(NewLocation); // No collision, so the ball can travel all the way
				Ball.AddLocation(NewLocation, CurrentTime);

				//if(SimulationLocations) DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Full Itr=%d, Speed=%f"), Iteration, Velocity.Size()), nullptr, FColor::White, 0.f);

				break;
			}

			if (Iteration == NumIterations - 1)
			{
				// Too many iterations so the collision situation might be too complicated to solve with the current velocity, let the ball drop freely
				Velocity = FVector::ZeroVector;

				//if (Ball.bSimulationBall)
				//{
				//	DrawDebugSphere(Ball.GetWorld(), Ball.GetActorLocation(), Ball.GetScaledSphereRadius() + 0.1f, 20, FColor::Magenta, false, 0.f);
				//	DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Iteration Limit!")), nullptr, FColor::Red, 0.f);
				//}

				break;
			}
		}

		if (FVector2D(Velocity.X, Velocity.Y).SizeSquared() < FMath::Square(StoppingSpeed)) // Stop horizontal movement if very low speed
		{			
			if (bTouchingFloor)
			{
				DrawTimedSlots(BallActor, CurrentTime, Velocity, true); // Force all remaining timed slots to activate
				ActivateStopTriggers(BallActor, CurrentTime);

				if (Ball.bSimulationBall)
				{
					SetLastPredictionTime(BallActor, CurrentTime);
					return DeltaSecondsUnsubdivided - CurrentTime; // Stop simulation if we are stopped
				}

				if (BallActor && !BallActor->Activated)
				{
					BallActor->ActivateBall();
				}
			}

			Velocity.X = 0.f;
			Velocity.Y = 0.f;
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
					CollidedWithBallThisFrame->Sphere->NumFramesCollidingWithBall = 0;
				}

				FVector PushVector = Ball.GetActorLocation() - CollidedWithBallThisFrame->GetActorLocation();
				PushVector.Normalize();
				Ball.SeparateBalls(CollidedWithBallThisFrame, PushVector, DepenetrationSpeed, DeltaSeconds, CurrentTime);

				CollidedWithBallThisFrame = nullptr;
			}
		}

		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Velocity=%s"), *(Velocity).ToString()));
		//UE_LOG(LogTemp, Warning, TEXT("Velocity=%s"), *(Velocity).ToString());		

		CurrentTime += DeltaSeconds;

		if (BallActor)
		{
			BallActor->CurrentTime += DeltaSeconds;

			float AuxTime = Ball.bSimulationBall ? CurrentTime : BallActor->CurrentTime;

			if (AuxTime >= 10.f && !BallActor->Activated)
			{
				DrawTimedSlots(BallActor, AuxTime, Velocity, true); // Force all remaining timed slots to activate
				ActivateStopTriggers(BallActor, CurrentTime);

				BallActor->ActivateBall();

				if (Ball.bSimulationBall)
				{
					SetLastPredictionTime(BallActor, CurrentTime);
					return DeltaSecondsUnsubdivided - CurrentTime; // Stop simulation
				}
			}
		}
	}

	//if (SimulationLocations) DrawDebugString(Ball.GetWorld(), Ball.GetActorLocation(), FString::Printf(TEXT("Simulation end!")), nullptr, FColor::Red, 0.f);
	
	SetLastPredictionTime(BallActor, CurrentTime);
	return DeltaSecondsUnsubdivided - CurrentTime;
}


void USteamrollSphereComponent::SetLastPredictionTime(ASteamrollBall* BallActor, float LastPredictionTime)
{
	if (bSimulationBall)
	{
		BallActor->GetLocalPlayerController()->SetLastPredictionTime(LastPredictionTime);
	}
}


FVector USteamrollSphereComponent::DragPhysics(const FVector& Velocity, float TravelTime)
{
	float SelectedCoefficient;

	if (Velocity.SizeSquared() > FMath::Square(DragCoefficientSlowSpeed))
	{
		SelectedCoefficient = DragCoefficient;
	}
	else
	{
		SelectedCoefficient = DragCoefficientSlow;
	}

	float ActualCoefficient = FMath::Min(SelectedCoefficient * TravelTime, 1.f);

	return Velocity - Velocity * ActualCoefficient;
}


void USteamrollSphereComponent::SeparateBalls(ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds, float CurrentTime)
{
	DeltaSeconds = 0.001f;
	GetAttachmentRootActor()->SetActorLocation(GetActorLocation() + PushVector * DepenetrationSpeed * DeltaSeconds, true);
	AddLocation(GetActorLocation(), CurrentTime);

	if (!bSimulationBall)
	{
		OtherBall->SetActorLocation(OtherBall->GetActorLocation() - PushVector * DepenetrationSpeed * DeltaSeconds, true);
	}
}


void USteamrollSphereComponent::RotateBall(FVector& Velocity, float Speed, float DeltaSeconds)
{
	// Make ball rotate in the movement direction only if there's significant horizontal movement
	float HorizontalSpeed = FVector2D(Velocity.X, Velocity.Y).Size();
	
	if (HorizontalSpeed > 0.1f)
	{
		FVector RotationVector = FVector::CrossProduct(Velocity / Speed, FVector(0.f, 0.f, 1.f)); // Compute rotation axis
		RotationVector.Normalize();

		float ChangeRotationTime = 0.25f;
		RotationAxis = FMath::Lerp(RotationAxis, RotationVector, FMath::Min(1.f, DeltaSeconds / ChangeRotationTime));

		float NumRotations = HorizontalSpeed * DeltaSeconds / (2.f * PI * GetScaledSphereRadius()); // Distance travelled / Sphere circumference
		float RotationAngle = -360.f * NumRotations; // Convert Rotations to degrees
		
		FVector RotatedForwardVector = FVector(0.f, 0.f, 1.f).RotateAngleAxis(RotationAngle, RotationAxis);
		FQuat Quat = FQuat::FindBetween(FVector(0.f, 0.f, 1.f), RotatedForwardVector);
		AddWorldRotation(Quat.Rotator());
	}
}


void USteamrollSphereComponent::ResetTimedSlots(ASteamrollBall* BallActor)
{
	if (BallActor)
	{
		for (uint32 i = 1; i < 5; ++i)
		{
			BallActor->SlotsConfig.SetSlotUsed(i, false);
		}
	}
}


void USteamrollSphereComponent::DrawTimedSlots(ASteamrollBall* BallActor, float CurrentTime, const FVector& Velocity, bool bForceRemainingSlots)
{
	if (BallActor)
	{
		for (uint32 i = 1; i < 5; ++i)
		{
			if (BallActor->GetSlotState(i) != ESlotTypeEnum::SE_EMPTY && BallActor->GetSlotActivatorType(i) == ESlotTypeEnum::SE_TIME && 
				!BallActor->SlotsConfig.IsSlotUsed(i) && BallActor->GetSlotState(i) != ESlotTypeEnum::SE_RAMP)
			{
				//if(CurrentTime > 9.5f) UE_LOG(LogTemp, Warning, TEXT("%f, %f"), BallActor->GetSlotTime(i), CurrentTime);

				float TestTime = bSimulationBall ? CurrentTime : BallActor->CurrentTime;

				if (bForceRemainingSlots || BallActor->GetSlotTime(i) < TestTime || (BallActor->GetSlotTime(i) > 9.9f && TestTime > 9.85f))
				{
					if (bSimulationBall)
					{
						if (BallActor->GetSlotState(i) == ESlotTypeEnum::SE_WALL)
						{
							DrawSimulationWall(BallActor, i, TestTime);
						}
						else if (BallActor->GetSlotState(i) == ESlotTypeEnum::SE_EXPL)
						{
							DrawSimulationExplosion(BallActor, i, TestTime);
						}

						BallActor->SlotsConfig.SetSlotUsed(i);
					}
					else
					{
						//UE_LOG(LogTemp, Warning, TEXT("Deploy time=%f, Timer time=%f"), TestTime, BallActor->GetSlotTime(i));
						BallActor->ActivateTimerTrigger(i);
					}
					
					//DrawDebugSphere(GetWorld(), GetActorLocation(), GetScaledSphereRadius(), 10, FColor::White);
					//DrawDebugString(GetWorld(), GetActorLocation() + FVector(-50.f, -50.f, -50.f), FString::Printf(TEXT("%f"), CurrentTime), nullptr, FColor::Red, 0.f);
				}
			}
		}
	}
}


void USteamrollSphereComponent::HandleImpactSlots(ASteamrollBall* BallActor, AActor* HitActor, const FVector& Velocity, float CurrentTime)
{
	if (BallActor && HitActor)
	{
		for (uint32 i = 1; i < 5; ++i)
		{
			if (BallActor->GetSlotActivatorType(i) == ESlotTypeEnum::SE_CONTACT && !BallActor->SlotsConfig.IsSlotUsed(i) && BallActor->GetSlotState(i) != ESlotTypeEnum::SE_RAMP) // Ramps are handled separately
			{
				if (HitActor->GetClass()->ImplementsInterface(UExplosionDestructibleInterface::StaticClass()))
				{
					if (BallActor->SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_WALL)
					{
						if (bSimulationBall)
						{
							DrawSimulationWall(BallActor, i, CurrentTime);
						}
						else
						{
							BallActor->ActivateSlot(i);
						}
					}
					else if (BallActor->SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_EXPL)
					{
						if (bSimulationBall)
						{
							DrawSimulationExplosion(BallActor, i, CurrentTime);
						}
						else
						{
							BallActor->ExplosionEvent();
						}
					}

					BallActor->SlotsConfig.SetSlotUsed(i);
					//DrawDebugSphere(GetWorld(), GetActorLocation(), GetScaledSphereRadius(), 10, FColor::White);
					//DrawDebugString(GetWorld(), GetActorLocation() + FVector(-50.f, -50.f, -50.f), FString::Printf(TEXT("%f"), CurrentTime), nullptr, FColor::Red, 0.f);
				}
			}
		}
	}
}


void USteamrollSphereComponent::DrawSimulationWall(ASteamrollBall* BallActor, int32 SlotIndex, float CurrentTime)
{
	if (BallActor)
	{
		float Angle = Velocity.Rotation().Yaw + BallActor->GetSlotAngle(SlotIndex) + 90.f;
		//FQuat Quat = FQuat(FVector::UpVector, FMath::DegreesToRadians(Angle));
		//DrawDebugBox(GetWorld(), GetActorLocation(), FVector(300.f, 15.f, 175.f), Quat, FColor::White);

		if (PlayerBase)
		{
			PlayerBase->DrawSimulatedWall(GetActorLocation() - FVector(0.f, 0.f, 95.f), FRotator(0.f, Angle, 0.f), CurrentTime, SlotIndex);
		}

		// Rebound prediction
		float WallAngle = BallActor->GetSlotAngle(SlotIndex);

		if (FMath::Abs(WallAngle) > 0.f && FMath::Abs(WallAngle) < 90.f)
		{
			float WallHalfWidth = 1.f;
			float BallRadius = BallActor->Sphere->GetScaledSphereRadius();
			FVector Direction = Velocity.GetSafeNormal();

			float Displacement1 = WallHalfWidth / FMath::Cos(FMath::DegreesToRadians(WallAngle));
			float Displacement2 = BallRadius / FMath::Cos(FMath::DegreesToRadians(WallAngle));
			FVector ReboundLocation = GetActorLocation() - Direction * (Displacement1 + Displacement2);
			//DrawDebugSphere(GetWorld(), ReboundLocation, BallRadius, 20, FColor::Green);

			ASteamrollBall* SimulatedBall = Cast<ASteamrollBall>(GetWorld()->SpawnActor(ASteamrollBall::StaticClass()));

			if (SimulatedBall)
			{
				BallActor->WallReboundPredictionBalls.Add(SimulatedBall);
				SimulatedBall->Sphere->TrajectoryComponent->MatInstance->SetScalarParameterValue("IsPrimaryPrediction", 0.f);

				SimulatedBall->Sphere->bSimulationBall = true;
				SimulatedBall->SetActorEnableCollision(false);
				SimulatedBall->Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SimulatedBall->Sphere->SetSimulatePhysics(false);
				SimulatedBall->SetActorLocation(GetActorLocation());
				SimulatedBall->Sphere->SetSphereRadius(99.442101f);
				SimulatedBall->Sphere->PlayerBase = PlayerBase;

				SimulatedBall->VirtualSphere->SetSimulatePhysics(false);
				SimulatedBall->VirtualSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				SimulatedBall->SetActorLocation(ReboundLocation);

				FVector Normal = -Direction.RotateAngleAxis(WallAngle, FVector::UpVector);
				FVector NewVelocity = Velocity.GetSafeNormal() * 1500.f;
				SimulatedBall->SetVelocity(NewVelocity.MirrorByVector(Normal));

				SimulatedBall->Sphere->TrajectoryComponent->DeleteLocations();

				float TotalTime = SimulatedBall->Sphere->UpdateBallPhysics(2.f);
				auto MaterialInstance = SimulatedBall->Sphere->TrajectoryComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, SimulatedBall->Sphere->TrajectoryComponent->GetMaterial(0));
				MaterialInstance->SetScalarParameterValue("Charging", 0.f);

				SimulatedBall->Sphere->TrajectoryComponent->SendData();

				SimulatedBall->SetActorEnableCollision(false);
				SimulatedBall->Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SimulatedBall->Sphere->SetSimulatePhysics(false);
			}
		}
	}
}


void USteamrollSphereComponent::DrawSimulationExplosion(ASteamrollBall* BallActor, int32 SlotIndex, float CurrentTime)
{
	if (BallActor)
	{
		float Radius = BallActor->ComputeExplosionRadius();
		//DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 10, FColor::Green);

		if (PlayerBase)
		{
			PlayerBase->DrawSimulatedExplosion(GetActorLocation(), Radius, CurrentTime, SlotIndex);
		}
	}
}


void USteamrollSphereComponent::DrawSimulationRamp(const FVector& Location, int32 SlotIndex, const FVector& Normal, float CurrentTime)
{
	float Size = 150.f;
	FRotator Rotation = Normal.Rotation();
	//DrawDebugBox(GetWorld(), Location + Normal * Size * 0.5f, FVector(Size), Rotation.Quaternion(), FColor::White);

	Rotation.Yaw -= 90.f;

	if (PlayerBase)
	{
		PlayerBase->DrawSimulatedRamp(Location + Normal * Size * 2.62f - FVector(0.f, 0.f, 95.f), Rotation, CurrentTime, SlotIndex);
	}
}


void USteamrollSphereComponent::AddLocation(const FVector& Location, float CurrentTime)
{
	if (bSimulationBall)
	{
		TrajectoryComponent->AddLocation(Location, CurrentTime);
	}
}


void USteamrollSphereComponent::ReduceVerticalVelocity(FVector& Velocity, bool bTouchingFloor, float DeltaSeconds, ECollisionChannel CollisionChannel)
{
	bTouchingFloor = IsTouchingFloor(CollisionChannel, true);

	if (!bTouchingFloor && Velocity.Z > 10.f)
	{
		float Len = Velocity.Size();

		if (Len != 0.f)
		{
			float AngleWithVertical = FMath::RadiansToDegrees(FMath::Acos((Velocity / Len) | FVector::UpVector));

			if (AngleWithVertical > 5.f)
			{
				float InitialLen = Velocity.Size();
				float Drag = FMath::Min(10.f * Velocity.Z * DeltaSeconds, Velocity.Z);
				Velocity.Z = Velocity.Z - Drag;
				Velocity = Velocity.GetSafeNormal() * InitialLen;

				//Velocity.Z = 0.f;
				//Velocity.Normalize();
				//Velocity *= Len;

				//float RotationAngle = 90.f - AngleWithVertical;
				//Velocity = Velocity.RotateAngleAxis(RotationAngle, FVector::CrossProduct(FVector::UpVector, Velocity));
			}
		}
	}
}


bool USteamrollSphereComponent::IsTouchingFloor(ECollisionChannel CollisionChannel, bool bSphereTrace) const
{
	if (!bSphereTrace)
	{
		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, GetAttachmentRootActor());
		RV_TraceParams.bTraceComplex = false;
		RV_TraceParams.bTraceAsyncScene = false;
		RV_TraceParams.bReturnPhysicalMaterial = false;
		//Re-initialize hit info
		FHitResult RV_Hit(ForceInit);

		bool bHit = GetWorld()->LineTraceSingle(
			RV_Hit, //result
			GetActorLocation(), //start
			GetActorLocation() + FVector(0.f, 0.f, -2.f * GetScaledSphereRadius()), //end at twice the radius
			CollisionChannel, //collision channel
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
	else
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetAttachmentRootActor());
		FHitResult OutHit;

		if (UKismetSystemLibrary::SphereTraceSingle_NEW(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0.f, 0.f, -1.5f), GetScaledSphereRadius() + 10.5f, UEngineTypes::ConvertToTraceType(CollisionChannel), true, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true))
		{
			/*if (OutHit.Time < 0.05f)
			{
			return true;
			}*/
			//DrawDebugSphere(GetWorld(), GetActorLocation(), Sphere->GetScaledSphereRadius() + 0.1f, 15, FColor::Red, false, 0.0f);

			return true;
		}

		//DrawDebugSphere(GetWorld(), GetActorLocation(), Sphere->GetScaledSphereRadius() + 0.1f, 15, FColor::Green, false, 0.0f);

		return false;
	}
}


FVector USteamrollSphereComponent::GetActorLocation() const
{
	return this->GetComponentLocation();
}


void USteamrollSphereComponent::SetActorLocation(const FVector& Location)
{
	this->SetWorldLocation(Location);
}


void USteamrollSphereComponent::ActivateSnapRamp(ASteamrollBall* BallActor, const FVector& Location, const FVector& Normal, float CurrentTime)
{
	if (BallActor)
	{
		for (int32 i = 1; i < 5; i++)
		{
			if (BallActor->SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_RAMP && !BallActor->SlotsConfig.IsSlotUsed(i))
			{
				BallActor->SlotsConfig.SetSlotUsed(i);

				if (bSimulationBall)
				{
					DrawSimulationRamp(Location, i, Normal, CurrentTime);
				}
				else
				{
					BallActor->SnapRampEvent(Location, Normal);
				}

				break;
			}
		}
	}
}


void USteamrollSphereComponent::ActivateStopTriggers(ASteamrollBall* BallActor, float CurrentTime)
{
	if (BallActor)
	{
		for (int32 i = 1; i < 5; i++)
		{
			if (BallActor->SlotsConfig.GetSlotActivatorType(i) == ESlotTypeEnum::SE_STOP && !BallActor->SlotsConfig.IsSlotUsed(i))
			{				
				if (BallActor->SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_WALL)
				{
					if (bSimulationBall)
					{
						DrawSimulationWall(BallActor, i, CurrentTime);
						BallActor->SlotsConfig.SetSlotUsed(i);
					}
					else
					{
						BallActor->ActivateSlot(i);
					}
				}
				else if (BallActor->SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_EXPL)
				{
					if (bSimulationBall)
					{
						DrawSimulationExplosion(BallActor, i, CurrentTime);
						BallActor->SlotsConfig.SetSlotUsed(i);
					}
					else
					{
						BallActor->ExplosionEvent();
					}
				}

				break;
			}
		}
	}
}


void USteamrollSphereComponent::PlayBounceSound(AActor* Actor, float VolumeFactor)
{
	if (!bSimulationBall)
	{
		Actor->PlaySoundOnActor(BounceSoundCue, VolumeFactor);
	}
}

