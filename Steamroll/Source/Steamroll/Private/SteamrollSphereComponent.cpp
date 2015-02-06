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


USteamrollSphereComponent::USteamrollSphereComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	TrajectoryComponent = PCIP.CreateDefaultSubobject<UTrajectoryComponent>(this, TEXT("TrajectoryComponent"));
	TrajectoryComponent->AttachTo(this);

	Velocity = FVector::ZeroVector;
	bPhysicsEnabled = true;
	RotationAxis = FVector::ZeroVector;
	NumFramesCollidingWithBall = 0;
	bSimulationBall = false;
	RemainingTime = 0.f;

	NumIterations = 10;
	MaxDeltaSeconds = 1.f / 60.f;
	DepenetrationSpeed = 1000.f;
	DragCoefficient = 0.2f;
	DragCoefficientSlow = 0.8f;
	DragCoefficientSlowSpeed = 100.f;
	DragConstantSlowSpeed = 10.f;
}


void USteamrollSphereComponent::SteamrollTick(float DeltaSeconds)
{
	if (bSimulationBall)
	{
		return;
	}

	RemainingTime = UpdateBallPhysics(DeltaSeconds + RemainingTime);
}


float USteamrollSphereComponent::UpdateBallPhysics(float DeltaSecondsUnsubdivided)
{
	if (!bPhysicsEnabled)
	{
		return 0.f;
	}

	USteamrollSphereComponent& Ball = *this;
	FVector& Velocity = Ball.Velocity;
	ASteamrollBall* BallActor = Cast<ASteamrollBall>(GetAttachmentRootActor());
	float BallRadius = Ball.GetScaledSphereRadius();
	
	if (BallActor && bSimulationBall)
	{
		ResetTimedSlots(BallActor);
	}

	float DeltaSeconds = DeltaSecondsUnsubdivided;

	//if (DeltaSecondsUnsubdivided > MaxDeltaSeconds)
	//{
	DeltaSeconds = MaxDeltaSeconds;
	//}

	float CurrentTime = 0.f;

	while (CurrentTime <= DeltaSecondsUnsubdivided)
	{
		bool bTouchingFloor = Ball.IsTouchingFloor();

		Ball.DrawTimedSlots(CurrentTime, Velocity);

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
		Ball.AddLocation(CurrentLocation, CurrentTime);
		Ball.ReduceVerticalVelocity(Velocity, bTouchingFloor, DeltaSeconds);
		FVector NewLocation = CurrentLocation + Velocity * DeltaSeconds;

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(Ball.GetAttachmentRootActor());
		FHitResult OutHit;
		Ball.LastCollidedActor = nullptr;
		ASteamrollBall* CollidedWithBallThisFrame = nullptr;

		float RemainingTime = DeltaSeconds;

		ETraceTypeQuery TraceTypeQuery;
		
		if (Ball.bSimulationBall)
		{
			TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel4); // PredictionTraceChannel	
		}
		else
		{
			TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel7); // RealBallTraceChannel
		}

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
				
				if (BallTunnel && BallTunnel->ConnectedTunnel)
				{						
					if ((BallTunnel->TriggerVolume->GetComponentLocation() - OutHit.Location).Size() < BallRadius + BallTunnel->TriggerVolume->GetScaledSphereRadius())
					{
						if (Ball.bSimulationBall)
						{
							if (!BallTunnel->bDiscovered)
							{
								DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("?")), nullptr, FColor::Red, 0.f);
								return DeltaSecondsUnsubdivided - CurrentTime; // Stop simulation if we hit an undiscovered tunnel
							}

							CurrentLocation = BallTunnel->ConnectedTunnel->GetActorLocation();
							Speed = Velocity.Size() * BallTunnel->ConnectedTunnel->SpeedMultiplier;
							Velocity = BallTunnel->ConnectedTunnel->Mesh->GetUpVector() * Speed;
							Ball.SetActorLocation(CurrentLocation);
							TrajectoryComponent->CutTrajectory();
							Ball.AddLocation(CurrentLocation, CurrentTime);

							float TravelTime = OutHit.Time * RemainingTime;
							RemainingTime -= TravelTime;
							NewLocation = CurrentLocation + Velocity * TravelTime;
							continue;
						}
						else
						{
							BallTunnel->TransportToOtherTunnelEnd(BallActor);
							return DeltaSeconds - CurrentTime;
						}
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
						CurrentLocation -= Velocity.SafeNormal() * 0.01f;
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

				DrawImpactSlots(Ball.LastCollidedActor, Velocity);

				// Activate steamball by contact trigger
				if (BallActor && BallActor->HasSlotState(ESlotTypeEnum::SE_EXPL) && BallActor->HasSlotState(ESlotTypeEnum::SE_CONTACT)
					&& Ball.LastCollidedActor->GetClass()->ImplementsInterface(UExplosionDestructibleInterface::StaticClass()))
				{
					if (!Ball.bSimulationBall && BallActor)
					{
						BallActor->ExplosionEvent();
					}
				}

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

					FVector ImpactRadiusVector = (OutHit.Location - OutHit.ImpactPoint).SafeNormal(); // In fact this is the ball's normal
					
					if (FMath::Abs(OutHit.ImpactNormal | ImpactRadiusVector) < 0.99f && FMath::Abs(OutHit.ImpactPoint.Z - OutHit.Location.Z) > BallRadius / 2.f)
					{
						// Impacted against corner, use an interpolated "corner" normal
						OutHit.ImpactNormal = ImpactRadiusVector;
						//DrawDebugSphere(Ball.GetWorld(), Ball.GetActorLocation(), Ball.GetScaledSphereRadius() + 0.1f, 20, FColor::Green, false, 0.f);
					}

					FVector ReflectedVector = Velocity.MirrorByVector(OutHit.ImpactNormal);
					ReflectedVector.Normalize();
					Speed = Velocity.Size();

					if (BallActor && Speed > 0.f && ((ReflectedVector | (Velocity / Speed)) < 0.f) && (FMath::Abs(OutHit.ImpactNormal | FVector::UpVector) < 0.1f))
					{
						if (OutHit.Actor->GetName().StartsWith("Paret_Nivell"))
						{
							ActivateSnapRamp(BallActor, OutHit.ImpactPoint, OutHit.ImpactNormal);
						}
					}

					//DrawDebugDirectionalArrow(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 100.f, 3.f, FColor::Yellow);
					//DrawDebugDirectionalArrow(Ball.GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + ReflectedVector * 100.f, 3.f, FColor::Green);
					//DrawDebugString(Ball.GetWorld(), OutHit.ImpactPoint, FString::Printf(TEXT("Itr=%d, Speed=%f"), Iteration, Speed), nullptr, FColor::White, 0.f);

					Velocity = ReflectedVector * Speed;
					FVector VelocityN = OutHit.ImpactNormal * (Velocity | OutHit.ImpactNormal);
					FVector VelocityT = Velocity - VelocityN;

					// If impact normal is not flat or vertical, that is, we hit a ramp, set restitution to 0 to cancel normal velocity and snap ball to the ramp to prevent it from rebounding hard and flying away
					float Restitution = OutHit.PhysMaterial->Restitution;

					if (FMath::IsWithin(OutHit.ImpactNormal | FVector::UpVector, 0.1f, 0.9f))
					{
						VelocityT = VelocityT.SafeNormal() * Speed;
						Restitution = 0.f;
					}

					Velocity = VelocityT + VelocityN * Restitution;
					Speed = Velocity.Size();
				}

				Ball.ReduceVerticalVelocity(Velocity, bTouchingFloor, DeltaSeconds);
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

				if (Ball.bSimulationBall)
				{
					//DrawDebugSphere(Ball.GetWorld(), Ball.GetActorLocation(), Ball.GetScaledSphereRadius() + 0.1f, 20, FColor::Magenta, false, 0.f);
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
				return DeltaSecondsUnsubdivided - CurrentTime; // Stop simulation if we are stopped
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
	}

	//if (SimulationLocations) DrawDebugString(Ball.GetWorld(), Ball.GetActorLocation(), FString::Printf(TEXT("Simulation end!")), nullptr, FColor::Red, 0.f);

	return DeltaSecondsUnsubdivided - CurrentTime;
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
	// Make ball rotate in the movement direction only if there's significat horizontal movement
	float HorizontalSpeed = FVector2D(Velocity.X, Velocity.Y).Size();
	
	if (HorizontalSpeed > 0.1f)
	{
		FVector RotationVector = FVector::CrossProduct(Velocity / Speed, FVector(0.f, 0.f, 1.f)); // Compute rotation axis
		RotationVector.Normalize();

		float ChangeRotationTime = 0.25f;
		RotationAxis = FMath::Lerp(RotationAxis, RotationVector, DeltaSeconds / ChangeRotationTime);

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


void USteamrollSphereComponent::DrawTimedSlots(float CurrentTime, const FVector& Velocity)
{
	ASteamrollBall* BallActor = Cast<ASteamrollBall>(GetAttachmentRootActor());

	if (bSimulationBall && BallActor)
	{
		for (uint32 i = 1; i < 5; ++i)
		{
			if (BallActor->GetSlotState(i) == ESlotTypeEnum::SE_TIME && !BallActor->SlotsConfig.IsSlotUsed(i))
			{
				if (BallActor->SlotsConfig.GetSlotParam(i, 0) * 10.f < CurrentTime)
				{
					for (uint32 j = 1; j < 5; ++j)
					{
						if (i != j && BallActor->SlotsConfig.GetSlotConnection(i, j) && !BallActor->SlotsConfig.IsSlotUsed(j))
						{
							if (BallActor->SlotsConfig.GetSlotType(j) == ESlotTypeEnum::SE_WALL)
							{
								DrawSimulationWall(BallActor, j);
								BallActor->SlotsConfig.SetSlotUsed(j);

								break;
							}
							else if (BallActor->SlotsConfig.GetSlotType(j) == ESlotTypeEnum::SE_EXPL)
							{
								DrawSimulationExplosion(BallActor);
								BallActor->SlotsConfig.SetSlotUsed(j);

								break;
							}
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


void USteamrollSphereComponent::DrawImpactSlots(AActor* HitActor, const FVector& Velocity)
{
	ASteamrollBall* BallActor = Cast<ASteamrollBall>(GetAttachmentRootActor());

	if (bSimulationBall && BallActor)
	{
		for (uint32 i = 1; i < 5; ++i)
		{
			if (BallActor->GetSlotState(i) == ESlotTypeEnum::SE_CONTACT && !BallActor->SlotsConfig.IsSlotUsed(i))
			{
				if (HitActor->GetClass()->ImplementsInterface(UExplosionDestructibleInterface::StaticClass()))
				{
					for (uint32 j = 1; j < 5; ++j)
					{
						if (i != j && BallActor->SlotsConfig.GetSlotConnection(i, j) && !BallActor->SlotsConfig.IsSlotUsed(j))
						{
							if (BallActor->SlotsConfig.GetSlotType(j) == ESlotTypeEnum::SE_WALL)
							{
								DrawSimulationWall(BallActor, j);
								BallActor->SlotsConfig.SetSlotUsed(j);
							}
							else if (BallActor->SlotsConfig.GetSlotType(j) == ESlotTypeEnum::SE_EXPL)
							{
								DrawSimulationExplosion(BallActor);
								BallActor->SlotsConfig.SetSlotUsed(j);
							}

							break;
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


void USteamrollSphereComponent::DrawSimulationWall(ASteamrollBall* BallActor, uint32 SlotIndex)
{
	float Angle = Velocity.Rotation().Yaw + (BallActor->SlotsConfig.GetSlotParam(SlotIndex, 0) - 0.5f) * 180.f + 90.f;
	FQuat Quat = FQuat(FVector::UpVector, FMath::DegreesToRadians(Angle));
	//DrawDebugBox(GetWorld(), GetActorLocation(), FVector(300.f, 15.f, 175.f), Quat, FColor::White);

	if (PlayerBase)
	{
		PlayerBase->DrawSimulatedWall(GetActorLocation() - FVector(0.f, 0.f, 95.f), FRotator(0.f, Angle, 0.f));
	}
}


void USteamrollSphereComponent::DrawSimulationExplosion(ASteamrollBall* BallActor)
{
	float Radius = 1000.f * FMath::Pow(BallActor->SlotsConfig.CountSlotType(ESlotTypeEnum::SE_EXPL), 2);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 10, FColor::Green);

	if (PlayerBase)
	{
		PlayerBase->DrawSimulatedExplosion(GetActorLocation(), Radius);
	}
}


void USteamrollSphereComponent::DrawSimulationRamp(const FVector& Location, const FVector& Normal)
{
	float Size = 150.f;
	FRotator Rotation = Normal.Rotation();
	//DrawDebugBox(GetWorld(), Location + Normal * Size * 0.5f, FVector(Size), Rotation.Quaternion(), FColor::White);

	Rotation.Yaw -= 90.f;

	if (PlayerBase)
	{
		PlayerBase->DrawSimulatedRamp(Location + Normal * Size * 2.62f - FVector(0.f, 0.f, 95.f), Rotation);
	}
}


void USteamrollSphereComponent::AddLocation(const FVector& Location, float CurrentTime)
{
	if (bSimulationBall)
	{
		TrajectoryComponent->AddLocation(Location, CurrentTime);
	}
}


void USteamrollSphereComponent::ReduceVerticalVelocity(FVector& Velocity, bool bTouchingFloor, float DeltaSeconds)
{
	bTouchingFloor = IsTouchingFloor(true);

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
				Velocity = Velocity.SafeNormal() * InitialLen;

				//Velocity.Z = 0.f;
				//Velocity.Normalize();
				//Velocity *= Len;

				//float RotationAngle = 90.f - AngleWithVertical;
				//Velocity = Velocity.RotateAngleAxis(RotationAngle, FVector::CrossProduct(FVector::UpVector, Velocity));
			}
		}
	}
}


bool USteamrollSphereComponent::IsTouchingFloor(bool bSphereTrace) const
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
	else
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetAttachmentRootActor());
		FHitResult OutHit;

		if (UKismetSystemLibrary::SphereTraceSingle_NEW(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(0.f, 0.f, -1.5f), GetScaledSphereRadius() + 10.5f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_PhysicsBody), true, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true))
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


void USteamrollSphereComponent::ActivateSnapRamp(ASteamrollBall* BallActor, const FVector& Location, const FVector& Normal)
{
	for (int32 i = 1; i < 5; i++)
	{
		bool bConnectedToTrigger = false;

		for (uint32 j = 1; j < 5; j++)
		{
			TEnumAsByte<ESlotTypeEnum::SlotType> Type = BallActor->GetSlotState(j);
			bool bIsTrigger = Type == ESlotTypeEnum::SE_TIME;

			if (i != j && bIsTrigger && BallActor->SlotsConfig.GetSlotConnection(j, i))
			{
				bConnectedToTrigger = true;
				break;
			}
		}

		if (!bConnectedToTrigger && BallActor->SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_RAMP && !BallActor->SlotsConfig.IsSlotUsed(i))
		{
			BallActor->SlotsConfig.SetSlotUsed(i);

			if (bSimulationBall)
			{
				DrawSimulationRamp(Location, Normal);
			}
			else
			{
				BallActor->SnapRampEvent(Location, Normal);
			}

			break;
		}
	}
}

