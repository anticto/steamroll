// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Debris.generated.h"

UCLASS()
class STEAMROLL_API ADebris : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebris();

	UPROPERTY(BlueprintReadWrite, Category = Debris)
	float Opacity;

	UPROPERTY(BlueprintReadWrite, Category = Debris)
	bool bDissolving;
};
