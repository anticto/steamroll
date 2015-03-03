// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "SlotsConfig.h"
#include "ExplosionDestructibleInterface.h"
#include "SteamrollBall.generated.h"


UCLASS(config=Game)
class ASteamrollBall : public AActor, public IExplosionDestructibleInterface
{
	GENERATED_UCLASS_BODY()

	/** Capsule used for ball collison and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball)
	class USteamrollSphereComponent* Sphere;

	/** Shadow physics capsule used to get collision events from physx */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = VirtualBall)
	class UPhysicsVirtualSphereComponent* VirtualSphere;

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

	/** True if the ball is travelling through a tunnel and it does not have to have its timer time incremented */
	bool bTravellingInTunnel;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	FSlotsConfigStruct SlotsConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	bool bExplosionBlockedByContactSlot;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ball)
	void ActivateBall();

	UFUNCTION(BlueprintCallable, Category = Slots)
	class ASteamrollPlayerController* GetLocalPlayerController();

	/** Gets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotState(int32 SlotIndex);

	/** Sets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);	

	/** Gets a slot's activator type, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotActivatorType(int32 SlotIndex);

	/** Sets a slot's activator type, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotActivatorType(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);

	/** Gets a slot's angle, SlotIndex goes from 1 to 4 */
	float GetSlotAngle(int32 SlotIndex);

	/** Sets a slot's angle, SlotIndex goes from 1 to 4 */
	void SetSlotAngle(int32 SlotIndex, float Value);

	/** Gets a slot's time, SlotIndex goes from 1 to 4 */
	float GetSlotTime(int32 SlotIndex);

	/** Sets a slot's time, SlotIndex goes from 1 to 4 */
	void SetSlotTime(int32 SlotIndex, float Value);

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
	virtual void ActivateSlotEvent(const TEnumAsByte<ESlotTypeEnum::SlotType>& SlotType, float Angle, float Time);

	void ActivateRemoteTriggers();
	bool ActivateSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = Tunnel)
	void ExecuteTransport(class ABallTunnel* Tunnel, float Speed);

	FVector GetVelocity() const;
	void SetVelocity(const FVector& NewVelocity);

	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	float CurrentTime;
	float TimeForNextPaint;

	virtual void BeginPlay() override;

	/** Trigger and slot activation system */
	void ActivateTimerTrigger(int32 SlotIndex);
	void ActivateStopTriggers();	
};
