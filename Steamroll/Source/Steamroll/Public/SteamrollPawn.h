// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "SteamrollPawn.generated.h"

/**
 * Base class with common functionality for pawns in the Steamroll game, for instance PlayerBase and BaseBall
 */
UCLASS()
class STEAMROLL_API ASteamrollPawn : public APawn
{
	GENERATED_UCLASS_BODY()

	/** Transform used to aim the cannon or base ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USceneComponent> AimTransform;

	/** Camera attached to the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UCameraComponent> Camera;

	/** Camera free to move around the level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UCameraComponent> Camera2;

	// Camera controls
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base)
	bool RaiseCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base)
	bool LowerCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	UCameraAnim* CameraAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base)
	UCameraAnimInst* CameraAnimInst;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Base)
	float CurrCameraTime;

	UFUNCTION()
	void Debug(FString Msg);

	/** Firing message from client to server */
	UFUNCTION(reliable, server, WithValidation)
	void FireServer(float ChargeTime, FRotator Rotation);

	/** Explosion message from server to clients */
	UFUNCTION(unreliable, NetMulticast)
	void ExplosionClient();

	/** Cannon rotation message from client to server */
	UFUNCTION(reliable, server, WithValidation)
	void RotationServer(FRotator Rotation);

	/** Cannon rotation from server to clients */
	UFUNCTION(reliable, NetMulticast)
	void RotationClient(FRotator Rotation);

	/** Return how much the player has charged the base from 0 to 1 */
	UFUNCTION(BlueprintCallable, Category = Base)
	float GetCharge();

	/** Sets the current play time of the Camera Anim */
	UFUNCTION(BlueprintCallable, Category = Base)
	void SetCameraAnim(float Time, float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = Base)
	class ASteamrollPlayerController* GetLocalPlayerController();

	/** Called to associate camera with its animation, should be called right after possession */
	UFUNCTION(BlueprintCallable, Category = Base)
	void InitCameraAnim();

	/** Firing timer timeout callback */
	virtual void Timeout();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called for side to side input */
	virtual void MoveRight(float Val);

	/** Called to charge the cannon */
	virtual void FireCharge();

	/** Called to fire the cannon */
	virtual void FireRelease();

	/** Similar to FireRelease but with controlled power for debugging purposes */
	virtual void FireDebug1();
	virtual void FireDebug2();
	virtual void FireDebug3();
	virtual void FireDebug4();
	virtual void FireDebug5();
	virtual void FireDebug6();
	virtual void FireDebug7();
	virtual void FireDebug8();
	virtual void FireDebug9();
	virtual void FireDebug10();

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime);

	/** Actual Explosion procedure */
	virtual void Explode();

	/** Destroys all the balls in the scene */
	virtual void DestroyBalls();

	void ToggleFireSimulation();

	void RaiseCameraPressed();
	void RaiseCameraReleased();
	void LowerCameraPressed();
	void LowerCameraReleased();

	// Firing timeout
	float FiringTimeout;
	float MinLaunchSpeed;
	float MaxLaunchSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	bool bFiring;
	bool bFireSimulation;

	// Camera controls	
	float CameraSpeed;
	float MaxHeight;
	float MinHeight;
	
};
