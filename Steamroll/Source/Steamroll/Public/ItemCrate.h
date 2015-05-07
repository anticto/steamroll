// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ItemCrate.generated.h"

UCLASS()
class STEAMROLL_API AItemCrate : public AActor
{
	GENERATED_BODY()
	
public:
	AItemCrate();

	UFUNCTION(BlueprintNativeEvent, Category = ItemCrate)
	void BallCollision(AActor* Actor, float Speed);
};
