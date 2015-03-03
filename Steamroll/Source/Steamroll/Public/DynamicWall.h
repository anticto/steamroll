// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ExplosionDestructibleInterface.h"
#include "DynamicWall.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API ADynamicWall : public AActor, public IExplosionDestructibleInterface
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	class USceneComponent* TransformBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	class USceneComponent* TransformLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	class USceneComponent* TransformRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	class UStaticMeshComponent* WallLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Wall)
	class UStaticMeshComponent* WallRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Wall)
	bool ExpandedVertically;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Wall)
	bool ExpandedHorizontallyLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Wall)
	bool ExpandedHorizontallyRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wall)
	float MaxScaleWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Wall)
	float MaxScaleHeight;

	virtual void Tick(float DeltaSeconds) override;

};
