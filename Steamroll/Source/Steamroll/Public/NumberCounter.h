// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "NumberCounter.generated.h"

/**
 * 3d rotating number display
 */
UCLASS()
class STEAMROLL_API ANumberCounter : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	int32 Count;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	UStaticMeshComponent* NumberLeftCube;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	class UTextRenderComponent* NumberLeftText;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	UStaticMeshComponent* NumberRightCube;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	class UTextRenderComponent* NumberRightText;
};
