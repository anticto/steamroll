// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "Sound/AmbientSound.h"
#include "SoundNodeCompose.h"
#include "MusicManager.generated.h"




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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESoundNodeComposeStage GameStage = ESoundNodeComposeStage::Start;

	UPROPERTY(EditAnywhere)
	class USoundCue* Sound;

	UPROPERTY(EditAnywhere)
	UAudioComponent* AudioComponent;


};
