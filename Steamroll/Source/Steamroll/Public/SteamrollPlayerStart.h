// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "Engine/NavigationObjectBase.h"
#include "GameFramework/PlayerStart.h"
#include "SteamrollPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API ASteamrollPlayerStart : public APlayerStart
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	class ADeploymentSpot* DeploymentSpot;
	
	/** How much steam pressure the base ball will start with in case it is not attached to a deployment spot, range is [0, 1] */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float SteamPressure;
};
