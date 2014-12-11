// Fill out your copyright notice in the Description page of Project Settings.

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
	TSubobjectPtr<class UTrajectoryComponent> TrajectoryComponent;

	/** Collision/Physics system */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	AActor* LastCollidedActor;

	// Physics constants

	/** How many collision tests are going to be made per subtick */
	uint32 NumIterations;
	/** Timestep length */
	float MaxDeltaSeconds;
	float DepenetrationSpeed;
	float DragCoefficient;
	float DragCoefficientSlow;
	float DragCoefficientSlowSpeed;
	float DragConstantSlowSpeed;

	// Physical Simulation of trajectory or actual player gameplay?
	bool bSimulationBall;

	/** How many consecutive frames the ball has been colliding with a ball, used to stop the ball velocity if it has become stuck */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	uint32 NumFramesCollidingWithBall;
	float RemainingTime;

	void SteamrollTick(float DeltaSeconds);
	FVector DragPhysics(const FVector& Velocity, float TravelTime);
	
	float UpdateBallPhysics(float DeltaSeconds);
	void SeparateBalls(class ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds);
	void RotateBall(FVector& Velocity, float Speed, float DeltaSeconds);
	void ResetTimedSlots(ASteamrollBall* BallActor);
	void DrawTimedSlots(float CurrentTime, const FVector& Velocity);
	void DrawImpactSlots(AActor* HitActor, const FVector& Velocity);
	bool IsTouchingFloor(bool bSphereTrace = false) const;
	FVector GetActorLocation() const;
	void SetActorLocation(const FVector& Location);

	UFUNCTION()
	virtual void OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	

private:
	void AddLocation(const FVector& Location);
	void ReduceVerticalVelocity(FVector& Velocity, bool bTouchingFloor, float DeltaSeconds);
	void DrawSimulationWall(ASteamrollBall* BallActor, uint32 SlotIndex);
	void DrawSimulationExplosion(ASteamrollBall* BallActor);
	void DrawSimulationRamp(const FVector& Location, const FVector& Normal);
	void ActivateSnapRamp(ASteamrollBall* BallActor, const FVector& Location, const FVector& Normal);
};
