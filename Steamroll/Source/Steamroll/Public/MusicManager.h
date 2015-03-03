// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "Sound/AmbientSound.h"
#include "MusicManager.generated.h"


UENUM()
enum class EGameStage : uint8
{
	Start,
	End,
	Default,
	Stage1,
	Stage2,
	Stage3,
	Stage4,
	Stage5
};


USTRUCT()
struct FTransition
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	EGameStage Stage = EGameStage::Default;

	UPROPERTY(EditAnywhere)
	int32 Target = 0;
};


USTRUCT()
struct FMusicState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	TArray<FTransition> Transitions;
};


/**
 * 
 */
UCLASS()
class STEAMROLL_API AMusicManager : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:
	
	// UActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:

	UPROPERTY(EditAnywhere)
	EGameStage GameStage = EGameStage::Start;

	UPROPERTY(EditAnywhere)
	class USoundCue* Sound;

	UPROPERTY(EditAnywhere)
	TArray<FMusicState> States;

	UPROPERTY(EditAnywhere)
	int32 State;

	UPROPERTY(EditAnywhere)
	UAudioComponent* AudioComponent;

private:

	//void OnAudioFinished( class UAudioComponent* AC );

};
