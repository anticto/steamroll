// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Pawn.h"
#include "SteamrollBall.generated.h"

UCLASS(config=Game)
class ASteamrollBall : public APawn
{
	GENERATED_UCLASS_BODY()

	/** Capsule used for the ball collison and physics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball)
	TSubobjectPtr<class USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball)
	bool Activated;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = Ball)
	void ActivateBall();

protected:
	float Age;
	bool Dragging;
	static float StartDraggingTime;

};
