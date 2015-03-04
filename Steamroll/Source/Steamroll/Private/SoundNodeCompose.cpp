// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "Steamroll.h"
#include "Engine.h"
#include "SoundDefinitions.h"
#include "SoundNodeCompose.h"

/*-----------------------------------------------------------------------------
         USoundNodeCompose implementation.
-----------------------------------------------------------------------------*/
USoundNodeCompose::USoundNodeCompose(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool USoundNodeCompose::NotifyWaveInstanceFinished( FWaveInstance* WaveInstance )
{
	FActiveSound& ActiveSound = *WaveInstance->ActiveSound;
	//const UPTRINT NodeWaveInstanceHash = WaveInstance->NotifyBufferFinishedHooks.GetHashForNode(this);
	//RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( int32 ) );
	//DECLARE_SOUNDNODE_ELEMENT( int32, NodeIndex );
	//check( *RequiresInitialization == 0 );

	//// Allow wave instance to be played again the next iteration.
	//WaveInstance->bIsStarted = false;
	//WaveInstance->bIsFinished = false;

	//// Advance index.
	//NodeIndex++;

	//return (NodeIndex < ChildNodes.Num());
	return false;
}


float USoundNodeCompose::GetDuration()
{
	return INDEFINITELY_LOOPING_DURATION;
}

void USoundNodeCompose::CreateStartingConnectors()
{
	// Concatenators default to two two connectors.
	InsertChildNode( ChildNodes.Num() );
	InsertChildNode( ChildNodes.Num() );
}


void USoundNodeCompose::InsertChildNode( int32 Index )
{
	Super::InsertChildNode( Index );
	InputTransitions.Insert(FMusicState(), Index);
}


void USoundNodeCompose::RemoveChildNode( int32 Index )
{
	Super::RemoveChildNode( Index );
	InputTransitions.RemoveAt(Index);
}

void USoundNodeCompose::ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD(sizeof(int32) + sizeof(float));
	DECLARE_SOUNDNODE_ELEMENT(int32, NodeIndex);
	DECLARE_SOUNDNODE_ELEMENT(float, AccumulatedTime);

	// Start from the beginning.
	if( *RequiresInitialization )
	{
		NodeIndex = 0;
		AccumulatedTime = 0.0f;
		*RequiresInitialization = false;
	}

	if (NodeIndex<ChildNodes.Num() )
	{
		float TimeLeft = 0.0f;
		USoundNode* CurrentChildNode = ChildNodes[NodeIndex];
		if (CurrentChildNode)
		{
			TimeLeft = CurrentChildNode->GetDuration() + AccumulatedTime - ActiveSound.PlaybackTime;
			UE_LOG(LogAudio, Error, TEXT("Time Left [%2.3f]"), TimeLeft);
			if (TimeLeft > 0.0f)
			{
				FSoundParseParameters UpdatedParams = ParseParams;
				UpdatedParams.StartTime = AccumulatedTime;
				UpdatedParams.bLooping = true;
				CurrentChildNode->ParseNodes(AudioDevice, GetNodeWaveInstanceHash(NodeWaveInstanceHash, CurrentChildNode, NodeIndex), ActiveSound, UpdatedParams, WaveInstances);
			}
		}

		if (TimeLeft < 0.0f)
		{
			// Go to next state
			AccumulatedTime += CurrentChildNode->GetDuration();

			int32 StageParamValue = 0;
			ActiveSound.GetIntParameter(ParamName, StageParamValue);

			// Find next node index
			int32 NextNodeIndex = NodeIndex;
			if (NodeIndex<InputTransitions.Num())
			{
				// Transition for this stage?
				bool bFound = false;
				int32 DefaultState = NodeIndex;
				for (int32 t = 0; t < InputTransitions[NodeIndex].Transitions.Num() && NodeIndex == NextNodeIndex; ++t)
				{
					const FTransition& Transition = InputTransitions[NodeIndex].Transitions[t];
					if (Transition.Stage == ESoundNodeComposeStage::Default)
					{
						DefaultState = Transition.Target;
					}
					else if ( (int32)Transition.Stage == StageParamValue )
					{
						bFound = true;
						NextNodeIndex = Transition.Target;
					}
				}

				if (!bFound)
				{
					NextNodeIndex = DefaultState;
				}
			}

			NodeIndex = NextNodeIndex;

			if (NodeIndex < ChildNodes.Num())
			{
				CurrentChildNode = ChildNodes[NodeIndex];
				if (CurrentChildNode)
				{
					FSoundParseParameters UpdatedParams = ParseParams;
					UpdatedParams.StartTime = -AccumulatedTime;
					UpdatedParams.bLooping = true;
					CurrentChildNode->ParseNodes(AudioDevice, GetNodeWaveInstanceHash(NodeWaveInstanceHash, CurrentChildNode, NodeIndex), ActiveSound, UpdatedParams, WaveInstances);
				}
			}
		}
	}
}

#if WITH_EDITOR
void USoundNodeCompose::SetChildNodes(TArray<USoundNode*>& InChildNodes)
{
	Super::SetChildNodes(InChildNodes);

	if (InputTransitions.Num() < ChildNodes.Num())
	{
		while (InputTransitions.Num() < ChildNodes.Num())
		{
			int32 NewIndex = InputTransitions.Num();
			InputTransitions.Insert(FMusicState(), NewIndex);
		}
	}
	else if (InputTransitions.Num() > ChildNodes.Num())
	{
		const int32 NumToRemove = InputTransitions.Num() - ChildNodes.Num();
		InputTransitions.RemoveAt(InputTransitions.Num() - NumToRemove, NumToRemove);
	}
}
#endif //WITH_EDITOR