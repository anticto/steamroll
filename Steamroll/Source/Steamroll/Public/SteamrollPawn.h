

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

	/** Camera to view the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UCameraComponent> Camera;

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

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called for side to side input */
	virtual void MoveRight(float Val);

	/** Called to charge the cannon */
	void FireCharge();

	/** Called to fire the cannon */
	void FireRelease();

	/** Firing timer timeout callback */
	void Timeout();

	/** Similar to FireRelease but with controlled power for debugging purposes */
	void FireDebug1();
	void FireDebug2();
	void FireDebug3();
	void FireDebug4();
	void FireDebug5();
	void FireDebug6();
	void FireDebug7();
	void FireDebug8();
	void FireDebug9();
	void FireDebug10();

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime);

	/** Actual Explosion procedure */
	virtual void Explode();

	/** Destroys all the balls in the scene */
	void DestroyBalls();

	void RaiseCameraPressed();
	void RaiseCameraReleased();
	void LowerCameraPressed();
	void LowerCameraReleased();

	// Firing timeout
	float FiringTimeout;
	float MinLaunchSpeed;
	float MaxLaunchSpeed;

	// Camera controls	
	float CameraSpeed;
	float MaxHeight;
	float MinHeight;
	
};
