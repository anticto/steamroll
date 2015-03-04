// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "MusicManager.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundNodeSwitch.h"


AMusicManager::AMusicManager(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	AudioComponent = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("AudioComponent"));
	RootComponent = AudioComponent;
	AudioComponent->bIsUISound = true;
	AudioComponent->bOverrideAttenuation = true;
	AudioComponent->AttenuationOverrides.bAttenuate = false;
	AudioComponent->AttenuationOverrides.bSpatialize = false;
}


void AMusicManager::BeginPlay()
{
	Super::BeginPlay();

	AudioComponent->Sound = Sound;
	AudioComponent->Play();
}


void AMusicManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AudioComponent->SetIntParameter("Stage", (int32)GameStage);
}

