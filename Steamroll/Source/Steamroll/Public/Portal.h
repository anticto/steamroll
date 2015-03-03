// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Portal.generated.h"

/**
 * Teleportation device
 */
UCLASS()
class STEAMROLL_API APortal : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portal)
	class UBoxComponent* BoxPortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
	APortal* LinkedPortal;
	
	UFUNCTION()
	void OnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
