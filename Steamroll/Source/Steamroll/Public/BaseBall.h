

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

	/** Capsule used for the ball collison and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USphereComponent> Sphere;

	/** Mesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> GyroMesh;

	/** Mesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UStaticMeshComponent> SphereMesh;

	/** Spring arm for positioning the camera above the base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class USpringArmComponent> SpringArm;

protected:

	/** Actual Firing procedure */
	virtual void Fire(float ChargeTime) override;

	/** Actual Explosion procedure */
	virtual void Explode() override;
	
};
