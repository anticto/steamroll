

#pragma once

#include "GameFramework/Actor.h"
#include "LaserSight.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API ALaserSight : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = LaserSight)
	bool FirstHit;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = LaserSight)
	FVector FirstHitLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = LaserSight)
	FVector ReflectedVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = LaserSight)
	bool SecondHit;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = LaserSight)
	FVector SecondHitLocation;
	

	virtual void Tick(float DeltaSeconds) override;
};
