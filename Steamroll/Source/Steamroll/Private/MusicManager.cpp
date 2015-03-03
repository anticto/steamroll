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

	//AudioComponent->OnAudioFinishedNative.AddUObject( this, &AMusicManager::OnAudioFinished );

	if (State<States.Num())
	{
		AudioComponent->Sound = Sound;
		AudioComponent->Play();
	}

}


void AMusicManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//if (AudioComponent->)
	//{
	//	// Change state?
	//	int LastTarget = 0;
	//	int NewTarget = 0;
	//	if (State < States.Num())
	//	{
	//		for (int i = 0; i < States[State].Transitions.Num(); ++i)
	//		{
	//			if (GameStage == States[State].Transitions[i].Stage)
	//			{
	//				State = States[State].Transitions[i].Target;
	//				break;
	//			}
	//		}
	//	}

	//	// Update play parameters
	//	if (State < States.Num())
	//	{
	//		AudioComponent->SetIntParameter("Active", NewTarget);
	//		AudioComponent->SetIntParameter("Fading", LastTarget);
	//	}
	//}
}

