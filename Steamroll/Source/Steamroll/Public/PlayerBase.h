// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "SteamrollPawn.h"
#include "PlayerBase.generated.h"

/**
 * The deployed player base
 */
UCLASS()
class STEAMROLL_API APlayerBase : public ASteamrollPawn
{
	GENERATED_UCLASS_BODY()

	/** StaticMesh used for the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> Base;

	/** StaticMesh used for the cannon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> Cannon;

	/** Spring arm for positioning the camera above the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USpringArmComponent> SpringArm;

	/** What subclass of SteamRollTestBall is going to be fired */
	UPROPERTY(EditAnywhere, Category = Base)
	TSubclassOf<class ASteamrollBall> WhatToSpawn;

	/** Particle system used for the cannon firing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UParticleSystemComponent> Explosion;

	/** Base's collected resource quantity */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Collect)
	float CollectedQuantity;

	/** Simulated ball for trajectory display */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SimulatedBall)
	ASteamrollBall* SimulatedBall;

	/** Cannon charge level */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base)
	float ChargeTime;


protected:

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void FireCharge() override;
	void FireRelease() override;

	/** Called to angle the cannon */
	void MoveForward(float Val);

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime) override;

	/** Actual Explosion procedure */
	virtual void Explode() override;

	virtual void DestroyBalls() override;

	virtual void FireDebug1() override;
	virtual void FireDebug2() override;
	virtual void FireDebug3() override;
	virtual void FireDebug4() override;
	virtual void FireDebug5() override;
	virtual void FireDebug6() override;
	virtual void FireDebug7() override;
	virtual void FireDebug8() override;
	virtual void FireDebug9() override;
	virtual void FireDebug10() override;

	/** Set fire charge in increments when a button is pressed, for example with the mouse wheel */
	virtual void SetChargeUp();
	virtual void SetChargeDown();

	/** Deletes last deployed actor */
	void Undo();

	void SetLastDeployedActor(AActor* Actor);
	void ActivateBall();
	FVector GetLaunchLocation() const;
	FVector GetLaunchVelocity(float ChargeTime) const;
	ASteamrollBall* CreateSimulatedBall();

	/** The value ChargeTime should be smoothly moved to to avoid abrupt trajectory changes */
	float TargetChargeTime;
	/** The speed at which the ChargeTime changes second */
	float ChargeTimeSpeed;

};
