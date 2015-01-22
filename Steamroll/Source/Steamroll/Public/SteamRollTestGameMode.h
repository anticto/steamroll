// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once
#include "GameFramework/GameMode.h"
#include "SteamRollTestGameMode.generated.h"

UCLASS(minimalapi)
class ASteamRollTestGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

	TSubclassOf<class APawn> PlayerBaseClass;
	TSubclassOf<class APawn> BaseBallClass;

	virtual AActor* ChoosePlayerStart(AController* Player) override;

	/** spawns default pawn for player*/
	virtual APawn* SpawnDefaultPawnFor(AController* NewPlayer, class AActor* StartSpot) override;

};



