// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "JetStream.generated.h"

UCLASS()
class STEAMROLL_API AJetStream : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJetStream();

	UPROPERTY(EditAnywhere)
	float Power;

	UPROPERTY(EditAnywhere)
	float MaxSpeed;

	UPROPERTY(EditAnywhere)
	float TurnFactor;

	UPROPERTY(EditAnywhere)
	TArray<bool> StreamActive;
};
