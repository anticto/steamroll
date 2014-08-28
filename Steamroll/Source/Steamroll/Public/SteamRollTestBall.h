// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Pawn.h"
#include "SteamRollTestBall.generated.h"

UCLASS(config=Game)
class ASteamRollTestBall : public APawn
{
	GENERATED_UCLASS_BODY()

	/** StaticMesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Ball)
	TSubobjectPtr<class UStaticMeshComponent> Ball;

	/** Particle system used for the steam */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Base)
	TSubobjectPtr<class UParticleSystemComponent> Steam;

	bool Activated;

	virtual void Tick(float DeltaSeconds);

};
