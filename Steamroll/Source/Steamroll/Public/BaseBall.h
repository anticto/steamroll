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
	TSubobjectPtr<class USteamrollSphereComponent> Sphere;

	/** Shadow physics capsule used to get collision events from physx */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = VirtualBall)
	TSubobjectPtr<class UPhysicsVirtualSphereComponent> VirtualSphere;

	/** Mesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> GyroMesh;

	/** Mesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> SphereMesh;

	/** Spring arm for positioning the camera above the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USpringArmComponent> SpringArm;

	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:

	virtual void Tick(float DeltaSeconds);

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime) override;

	/** Actual Explosion procedure */
	virtual void Explode() override;
};
