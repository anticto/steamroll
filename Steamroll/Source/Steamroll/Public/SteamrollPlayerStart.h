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
	
};
