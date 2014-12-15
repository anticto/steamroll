// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Pawn.h"
#include "SlotsConfig.h"
#include "ExplosionDestructibleInterface.h"
#include "SteamrollBall.generated.h"


UCLASS(config=Game)
class ASteamrollBall : public AActor, public IExplosionDestructibleInterface
{
	GENERATED_UCLASS_BODY()

	/** Capsule used for the ball collison and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball)
	TSubobjectPtr<class USteamrollSphereComponent> Sphere;

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

	UFUNCTION(BlueprintNativeEvent, Category = Slots)
	void SnapRampEvent(const FVector& Location, const FVector& Normal);

	UFUNCTION(BlueprintImplementableEvent, Category = Slots)
	virtual void ActivateSlotEvent(const TEnumAsByte<ESlotTypeEnum::SlotType>& SlotType, float Param1, float Param2);

	void ActivateRemoteTriggers();

	FVector GetVelocity() const;
	void SetVelocity(const FVector& NewVelocity);

protected:
	float CurrentTime;
	float TimeForNextPaint;

	/** Timer slot timeout */
	bool bIsTimerRunning[5];

	virtual void BeginPlay() override;

	/** Trigger and slot activation system */
	void ActivateTimerTrigger(int32 SlotIndex);
	void ActivateStopTriggers();
	void ActivateConnectedSlots(int32 SlotIndex);
	bool ActivateSlot(int32 SlotIndex);

};
