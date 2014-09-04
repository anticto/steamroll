

#pragma once

#include "GameFramework/Pawn.h"
#include "PlayerBase.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API APlayerBase : public APawn
{
	GENERATED_UCLASS_BODY()

	/** StaticMesh used for the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> Base;

	/** Transform used to move the cannon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USceneComponent> AimTransform;

	/** StaticMesh used for the cannon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> Cannon;

	/** Spring arm for positioning the camera above the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USpringArmComponent> SpringArm;

	/** Camera to view the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UCameraComponent> Camera;

	/** What subclass of SteamRollTestBall is going to be fired */
	UPROPERTY(EditAnywhere, Category = Base)
	TSubclassOf<class ASteamrollBall> WhatToSpawn;

	/** Particle system used for the cannon firing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UParticleSystemComponent> Explosion;

	UFUNCTION()
	void Debug(FString Msg);

	/** Firing message from client to server */
	UFUNCTION(reliable, server, WithValidation)
	void FireServer(float ChargeTime, FRotator Rotation);

	///** Explosion message from server to clients */
	UFUNCTION(unreliable, NetMulticast)
	void ExplosionClient();

	///** Firing message from client to server */
	UFUNCTION(reliable, server, WithValidation)
	void RotationServer(FRotator Rotation);

	/** Firing message from server to clients */
	UFUNCTION(reliable, NetMulticast)
	void RotationClient(FRotator Rotation);

	/** Return how much the player has charged the base from 0 to 1 */
	UFUNCTION(BlueprintCallable, Category = Base)
	float GetCharge();

protected:

	/** Called for side to side input */
	void MoveRight(float Val);

	/** Called to angle the cannon */
	void MoveForward(float Val);

	/** Called to charge the cannon */
	void FireCharge();

	/** Called to fire the cannon */
	void FireRelease();

	/** Actual Firing procedure */
	void Fire(float ChargeTime);

	/** Firing timer timeout callback */
	void Timeout();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	// Firing timeout
	float FiringTimeout;
	float MinLaunchSpeed;
	float MaxLaunchSpeed;

};
