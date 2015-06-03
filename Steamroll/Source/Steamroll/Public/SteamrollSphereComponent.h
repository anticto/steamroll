// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "Components/SphereComponent.h"
#include "SteamrollSphereComponent.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API USteamrollSphereComponent : public USphereComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	class UTrajectoryComponent* TrajectoryComponent;

	/** Collision/Physics system */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	bool bPhysicsEnabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	FVector RotationAxis;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	AActor* LastCollidedActor;

	/** The PlayerBase that shot this sphere component if it is actually a steamball, used to be able to draw simulated walls and ramps */
	UPROPERTY()
	class APlayerBase* PlayerBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	USoundCue* BounceSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ParticleSystem)
	UParticleSystem* SparksParticleSystem;

	// Physics constants

	/** How many collision tests are going to be made per subtick */
	uint32 NumIterations;
	/** Timestep length */
	float MaxDeltaSeconds;
	float StoppingSpeed;
	float DepenetrationSpeed;
	float DragCoefficient;
	float DragCoefficientSlow;
	float DragCoefficientSlowSpeed;
	float DragConstantSlowSpeed;

	// Physical Simulation of trajectory or actual player gameplay?
	bool bSimulationBall;

	/** True if the ball is simulating a secondary bounce from a wall item */
	bool bSecondarySimulation;

	// Max Speed this ball could have been launched with, used to scale bounce sound volumes
	float MaxInitialSpeed;

	/** How many consecutive frames the ball has been colliding with a ball, used to stop the ball velocity if it has become stuck */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	uint32 NumFramesCollidingWithBall;

	/** Frame interpolation data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics)
	FVector LastLoc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics)
	FQuat LastRot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics)
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics)
	FQuat Rotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics)
	float TimeToNextPhysTick;

	bool bInterpolationDataInitialized;

	UFUNCTION(BlueprintCallable, Category = Physics)
	void SetInterpRotation(FRotator NewRotation);

	void SteamrollTick(float DeltaSeconds);
	FVector DragPhysics(const FVector& Velocity, float TravelTime);
	
	void UpdateBallPhysics(uint32 NumPhysTicks);
	void UpdateBallPhysics(float DeltaSeconds);
	void SeparateBalls(FVector& InOutActorLocation, class ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds, float CurrentTime);
	void RotateBall(FVector& Velocity, float Speed, float DeltaSeconds);
	void ResetTimedSlots(ASteamrollBall* BallActor);
	void DrawTimedSlots(const FVector& ActorLocation, ASteamrollBall* BallActor, float CurrentTime, const FVector& Velocity, bool bForceRemainingSlots = false);
	void HandleImpactSlots(const FVector& ActorLocation, ASteamrollBall* BallActor, AActor* HitActor, const FVector& Velocity, float CurrentTime);
	bool IsTouchingFloor(const FVector& ActorLocation, ECollisionChannel CollisionChannel, bool bSphereTrace = false) const;
	FVector GetActorLocation() const;
	void SetActorLocation(const FVector& Location);

private:
	void AddLocation(const FVector& Location, float CurrentTime);
	void ReduceVerticalVelocity(const FVector& ActorLocation, FVector& Velocity, bool bTouchingFloor, float DeltaSeconds, ECollisionChannel CollisionChannel);
	void DrawSimulationWall(const FVector& ActorLocation, ASteamrollBall* BallActor, int32 SlotIndex, float CurrentTime);
	void DrawSimulationExplosion(const FVector& ActorLocation, ASteamrollBall* BallActor, int32 SlotIndex, float CurrentTime);
	void DrawSimulationRamp(const FVector& Location, int32 SlotIndex, const FVector& Normal, float CurrentTime);
	void ActivateSnapRamp(ASteamrollBall* BallActor, const FVector& Location, const FVector& Normal, float CurrentTime);
	void ActivateStopTriggers(const FVector& ActorLocation, ASteamrollBall* BallActor, float CurrentTime);
	void SetLastPredictionTime(ASteamrollBall* BallActor, float LastPredictionTime);
	void PlayBounceSound(const FVector& ActorLocation, AActor* Actor, float VolumeFactor);
	void PlayRollingSoundAndSparks(ASteamrollBall* BallActor, class ABaseBall* BaseBallActor, float Speed, bool bTouchingFloor);
	void ProcessJetStreams(TArray<FHitResult> &Hits, float DeltaSeconds);
};
