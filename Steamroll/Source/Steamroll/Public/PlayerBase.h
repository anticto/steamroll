

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


protected:

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called to angle the cannon */
	void MoveForward(float Val);

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime) override;

	/** Actual Explosion procedure */
	virtual void Explode() override;

	/** Deletes last deployed actor */
	void Undo();

	void SetLastDeployedActor(AActor* Actor);
	void ActivateBall();

};
