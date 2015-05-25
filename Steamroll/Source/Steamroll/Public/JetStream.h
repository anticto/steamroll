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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = JetStream)
	float Power;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = JetStream)
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = JetStream)
	float TurnFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = JetStream)
	TArray<bool> StreamActive;
};
