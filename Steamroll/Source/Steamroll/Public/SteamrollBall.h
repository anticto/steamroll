// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Pawn.h"
#include "SlotsConfig.h"
#include "ExplosionDestructibleInterface.h"
#include "DraggingBall.h"
#include "SteamrollBall.generated.h"


UCLASS(config=Game)
class ASteamrollBall : public AActor, public IExplosionDestructibleInterface, public DraggingBall
{
	GENERATED_UCLASS_BODY()

	/** Capsule used for the ball collison and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball)
	TSubobjectPtr<class USphereComponent> Sphere;

	UPROPERTY()
	TSubobjectPtr<class UTrajectoryComponent> TrajectoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball)
	bool Activated;

	/** Last tick's ball location */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball)
	FVector LastLoc;

	/** Collect ball's collected resource quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collect)
	float CollectedQuantity;

	/** How much collectable resource this ball can carry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collect)
	float CollectCapacity;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	FSlotsConfigStruct SlotsConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	bool bExplosionBlockedByContactSlot;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ball)
	void ActivateBall();

	/** Gets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotState(int32 SlotIndex);

	/** Sets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);

	UFUNCTION(BlueprintCallable, Category = Slots)
	class ASteamrollPlayerController* GetLocalPlayerController();

	UFUNCTION(BlueprintCallable, Category = Slots)
	bool HasSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);

	UFUNCTION(BlueprintCallable, Category = Slots)
	int32 CountSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);

	/** Returns a world transform representing the ball's location and orientation without its spin, that is, the orientation is its linear direction */
	UFUNCTION(BlueprintCallable, Category = Ball)
	FTransform GetBallFlattenedTransform(const FVector& LastLocation, float WallDeploymentAngle) const;

	/** Returns the ball world location adjusting for its radius while keeping it slightly above the floor */
	UFUNCTION(BlueprintCallable, Category = Ball)
	FTransform GetBallAdjustedTransform() const;

	/** Returns true if the ball is touching the floor */
	UFUNCTION(BlueprintCallable, Category = Ball)
	bool IsTouchingFloor() const;

	/** Returns true if the ball is touching the floor */
	UFUNCTION(BlueprintCallable, Category = Ball)
	void StopBall();

	/** Resets the dragging parameters and restarts the physical simulation if sleeping */
	UFUNCTION(BlueprintCallable, Category = Ball)
	void WakeBall();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Slots)
	void SplatPaint();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Slots)
	void ExplosionEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = Slots)
	virtual void SnapRampEvent(const FVector& Location, const FVector& Normal);

	UFUNCTION(BlueprintImplementableEvent, Category = Slots)
	virtual void ActivateSlotEvent(const TEnumAsByte<ESlotTypeEnum::SlotType>& SlotType, float Param1, float Param2);

	void ActivateRemoteTriggers();

	// Physical Simulation
	bool bSimulationBall;

	FVector GetVelocity() const;
	void SetVelocity(const FVector& NewVelocity);
	void DrawPhysicalSimulation();
	static void UpdateBallPhysics(ASteamrollBall& Ball, float DeltaSeconds);
	static FVector DragPhysics(const FVector& Velocity, float TravelTime, float DragCoefficient);

protected:

	float TimeForNextPaint;
	
	bool IsActivated();
	void DraggingBallActivate();
	void DraggingBallStop();

	/** Timer slot timeout */
	void Timeout1();
	void Timeout2();
	void Timeout3();
	void Timeout4();
	bool bIsTimerRunning[5];

	virtual void BeginPlay() override;

	/** Trigger and slot activation system */
	void ActivateTimerTrigger(int32 SlotIndex);
	void ActivateStopTriggers();
	void ActivateConnectedSlots(int32 SlotIndex);
	bool ActivateSlot(int32 SlotIndex);
	void ActivateSnapRamp(const FVector& Location, const FVector& Normal);

	/** Collision/Physics system */	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	AActor* LastCollidedActor;

	/** How many consecutive frames the ball has been colliding with a ball, used to stop the ball velocity if it has become stuck */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Physics)
	uint32 NumFramesCollidingWithBall;

	void UpdateBallPhysics(float DeltaSeconds);	
	void SeparateBalls(ASteamrollBall* OtherBall, const FVector& PushVector, float DepenetrationSpeed, float DeltaSeconds);
	void RotateBall(FVector& Velocity, float Speed, float DeltaSeconds);

private:
	void AddLocation(const FVector& Location);

};
