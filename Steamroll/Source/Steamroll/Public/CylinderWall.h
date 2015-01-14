// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ExplosionDestructibleInterface.h"
#include "CylinderWall.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API ACylinderWall : public AActor, public IExplosionDestructibleInterface
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	TSubobjectPtr<class USceneComponent> TransformBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	TSubobjectPtr<class UStaticMeshComponent> Cylinder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Wall)
	bool ExpandedVertically;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Wall)
	bool ExpandedHorizontally;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Wall)
	float ExpansionSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wall)
	float MaxScaleWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wall)
	float MaxScaleHeight;

	virtual void Tick(float DeltaSeconds) override;
	
};
