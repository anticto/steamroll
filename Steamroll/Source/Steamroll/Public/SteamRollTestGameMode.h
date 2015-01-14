// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once
#include "GameFramework/GameMode.h"
#include "SteamRollTestGameMode.generated.h"

UCLASS(minimalapi)
class ASteamRollTestGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

	virtual AActor* ChoosePlayerStart(AController* Player) override;

};



