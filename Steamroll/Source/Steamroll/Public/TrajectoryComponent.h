// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "ProceduralMeshComponent.h"
#include "TrajectoryComponent.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API UTrajectoryComponent : public UProceduralMeshComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	UMaterialInstanceDynamic* MatInstance;

	void AddLocation(const FVector& Location, float CurrentTime, bool bOverrideMinDistBetweenLocations = false);
	void DeleteLocations();
	void SendData();
	void CutTrajectory();

protected:
	FVector LastLocation;
	FVector LastXAxis;
	FVector LastP0;
	FVector LastP1;
	float LastTime;
	bool bAddedFirstLocation;
	bool bLastPValid;
	float Radius;
	float CumulativeV;
};
