// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamRollTestGameMode.h"
#include "PlayerBase.h"
#include "SteamrollPlayerStart.h"
#include "DeploymentSpot.h"
#include "BaseBall.h"
#include "SteamrollPlayerController.h"

#include "Engine.h"

ASteamRollTestGameMode::ASteamRollTestGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our ball
	DefaultPawnClass = APlayerBase::StaticClass();

	static ConstructorHelpers::FObjectFinder<UBlueprint> ItemBlueprint(TEXT("Blueprint'/Game/Base/Blueprint_PlayerBase.Blueprint_PlayerBase'"));
	if (ItemBlueprint.Object){
		PlayerBaseClass = (UClass*)ItemBlueprint.Object->GeneratedClass;
	}

	static ConstructorHelpers::FObjectFinder<UBlueprint> ItemBlueprint2(TEXT("Blueprint'/Game/Base/Blueprint_BaseBall.Blueprint_BaseBall'"));
	if (ItemBlueprint2.Object){
		BaseBallClass = (UClass*)ItemBlueprint2.Object->GeneratedClass;
	}

	if (GEngine && GEngine->GameViewport)
	{
		// Remove the default pause message
		GEngine->GameViewport->SetSuppressTransitionMessage(true);
	}
}


AActor* ASteamRollTestGameMode::ChoosePlayerStart(AController* Player)
{
	static int32 NextPlayerStartIndex = 0;

	bool bPIEStart = false;

	if (0 == PlayerStarts.Num())
		return NULL; // No player starts

	// Spawn at the next player start point
	APlayerStart* FoundPlayerStart = PlayerStarts[NextPlayerStartIndex % PlayerStarts.Num()];

	// Search for any PIE points
	for (int32 PlayerStartIndex = 0; PlayerStartIndex < PlayerStarts.Num(); ++PlayerStartIndex)
	{
		APlayerStart* PlayerStart = PlayerStarts[PlayerStartIndex];

		if (Cast<APlayerStartPIE>(PlayerStart) != NULL)
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			bPIEStart = true;
			break;
		}
	}

	if (!bPIEStart)
	{
		NextPlayerStartIndex++;
	}

	return FoundPlayerStart;
}


APawn* ASteamRollTestGameMode::SpawnDefaultPawnFor(AController* NewPlayer, class AActor* StartSpot)
{
	//choose the player start location based on player start's PlayerStartTag
	FRotator StartRotation(ForceInit);
	FVector StartLocation;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	SpawnInfo.bNoCollisionFail = true;

	APawn* ResultPawn;
	UClass* PawnClass;

	ASteamrollPlayerStart* PlayerStart = Cast<ASteamrollPlayerStart>(StartSpot);

	if (PlayerStart && PlayerStart->DeploymentSpot)
	{
		PawnClass = PlayerBaseClass;
		StartRotation.Yaw = PlayerStart->DeploymentSpot->GetActorRotation().Yaw;
		StartLocation = PlayerStart->DeploymentSpot->GetActorLocation();

		ASteamrollPlayerController* PlayerController = Cast<ASteamrollPlayerController>(NewPlayer);

		if (PlayerController)
		{
			PlayerController->SlotContent = PlayerStart->DeploymentSpot->SlotContent;
		}

		PlayerStart->DeploymentSpot->Destroy();
	}
	else
	{
		StartSpot = PlayerStart ? PlayerStart : StartSpot;

		PawnClass = BaseBallClass;
		StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
		StartLocation = StartSpot->GetActorLocation();
	}

	ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, StartLocation, StartRotation, SpawnInfo);

	if (ResultPawn && PawnClass == BaseBallClass)
	{
		Cast<ABaseBall>(ResultPawn)->UndeployInstantly();
	}

	return ResultPawn;
}

