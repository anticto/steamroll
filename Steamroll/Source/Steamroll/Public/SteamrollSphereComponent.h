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

	// Physical Simulation of trajectory or actual player gameplay?
	bool bSimulationBall;

	/** How many consecutive frames the ball has been colliding with a ball, used to stop the ball velocity if it has become stuck */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	uint32 NumFramesCollidingWithBall;
	float RemainingTime;

	void SteamrollTick(float DeltaSeconds);
	void DrawPhysicalSimulation();
	//static float UpdateBallPhysics(ASteamrollBall& Ball, float DeltaSeconds);
	static FVector DragPhysics(const FVector& Velocity, float TravelTime, float DragCoefficient);
	
	float UpdateBallPhysics(float DeltaSeconds);
	void SeparateBalls(class ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds);
	void RotateBall(FVector& Velocity, float Speed, float DeltaSeconds);
	void ResetTimedSlots(ASteamrollBall* BallActor);
	void DrawTimedSlots(float CurrentTime, const FVector& Velocity);
	bool IsTouchingFloor(bool bSphereTrace = false) const;
	FVector GetActorLocation() const;
	void SetActorLocation(const FVector& Location);

private:
	void AddLocation(const FVector& Location);
	void ReduceVerticalVelocity(FVector& Velocity, bool bTouchingFloor, float DeltaSeconds);
	
};
