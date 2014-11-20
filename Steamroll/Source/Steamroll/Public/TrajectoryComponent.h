// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "TrajectoryComponent.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API UTrajectoryComponent : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()

	// Begin UPrimitiveComponent interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual bool IsZeroExtent() const override;
	virtual struct FCollisionShape GetCollisionShape(float Inflation = 0.0f) const override;
	virtual bool AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const override;
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;
	// End UPrimitiveComponent interface.

	TArray<FVector> SimulatedLocations;
	float Radius;

	UPROPERTY()
	UMaterial* TrajectoryMat;
};
