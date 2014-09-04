// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Pawn.h"
#include "SteamrollBall.generated.h"

UCLASS(config=Game)
class ASteamrollBall : public APawn
{
	GENERATED_UCLASS_BODY()

	/** Capsule used for the ball collison and physics */
	UPROPERTY(VisibleAnywhere, Category = Ball)
	TSubobjectPtr<class USphereComponent> Sphere;

	bool Activated;

};
