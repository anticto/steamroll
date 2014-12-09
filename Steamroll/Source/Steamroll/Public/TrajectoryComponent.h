// Fill out your copyright notice in the Description page of Project Settings.

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

	void AddLocation(const FVector& Location, bool bOverrideMinDistBetweenLocations = false);
	void DeleteLocations();
	void SendData();

protected:
	FVector LastLocation;
	FVector LastXAxis;
	FVector LastP0;
	FVector LastP1;
	bool bAddedFirstLocation;
	bool bLastPValid;
	float Radius;
	float CumulativeV;

	UPROPERTY()
	UMaterial* TrajectoryMat;
};
