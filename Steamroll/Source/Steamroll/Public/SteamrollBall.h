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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball)
	bool Activated;

	/** Last tick's ball location */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball)
	FVector LastLoc;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ball)
	void ActivateBall();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	FSlotsConfigStruct SlotsConfig;

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

protected:
	
	bool IsActivated();
	void DraggingBallActivate();
	void DraggingBallStop();

	/** Timer slot timeout */
	void Timeout();

	virtual void BeginPlay() override;

};
