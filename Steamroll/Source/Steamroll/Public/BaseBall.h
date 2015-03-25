// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "SteamrollPawn.h"
#include "BaseBall.generated.h"

/**
 * The undeployed player base
 */
UCLASS()
class STEAMROLL_API ABaseBall : public ASteamrollPawn
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball)
	bool Activated;

	/** Capsule used for the ball collison and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	class USteamrollSphereComponent* Sphere;

	/** Shadow physics capsule used to get collision events from physx */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = VirtualBall)
	class UPhysicsVirtualSphereComponent* VirtualSphere;

	/** Mesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	class UStaticMeshComponent* GyroMesh;

	/** Mesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	class UStaticMeshComponent* SphereMesh;

	/** Spring arm for positioning the camera above the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	class USpringArmComponent* SpringArm;

	/** Ball acceleration value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float Acceleration;

	/** Ball max speed value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float MaxSpeed;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Base)
	void UndeployInstantly();

	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void Tick(float DeltaSeconds);

	/** Called to charge the cannon */
	void Trigger(float Val);

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime) override;

	/** Actual Explosion procedure */
	virtual void Explode() override;

	virtual void LeftClickDown();
};
