// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

 
/** 
 * A node to play sounds sequentially
 * WARNING: these are not seamless
 */

#pragma once
#include "Sound/SoundNode.h"
#include "SoundNodeCompose.generated.h"

UENUM()
enum class ESoundNodeComposeStage : uint8
{
	Default,
	Start,
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
	ESoundNodeComposeStage Stage = ESoundNodeComposeStage::Default;

	UPROPERTY(EditAnywhere)
	int32 Target = 0;
};


USTRUCT()
struct FMusicState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FTransition> Transitions;
};


/**
 Plays child nodes sequentially
*/
UCLASS(hidecategories = Object, editinlinenew, MinimalAPI, meta = (DisplayName = "Compose"))
class USoundNodeCompose : public USoundNode
{
	GENERATED_UCLASS_BODY()

	/** Transitions for each input state. */
	UPROPERTY(EditAnywhere, editfixedsize, Category=Concatenator)
	TArray<FMusicState> InputTransitions;

	/* Integer parameter controlling the current stage, that will guide the transitions. */
	UPROPERTY(EditAnywhere, Category = Compose)
	FName ParamName;

public:	
	// Begin USoundNode interface. 
	virtual bool NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance ) override;
	virtual float GetDuration( void ) override;
	virtual void ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances ) override;
	virtual int32 GetMaxChildNodes() const override 
	{ 
		return MAX_ALLOWED_CHILD_NODES; 
	}
	virtual void CreateStartingConnectors( void ) override;
	virtual void InsertChildNode( int32 Index ) override;
	virtual void RemoveChildNode( int32 Index ) override;
#if WITH_EDITOR
	/** Ensure amount of inputs matches new amount of children */
	virtual void SetChildNodes(TArray<USoundNode*>& InChildNodes) override;
#endif
	// End USoundNode interface. 
};

