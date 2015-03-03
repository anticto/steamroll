// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "CylinderWall.h"
#include "Kismet/KismetSystemLibrary.h"


ACylinderWall::ACylinderWall(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Base transform
	TransformBase = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("TransformBase"));
	RootComponent = TransformBase;

	// Cylinder wall part
	Cylinder = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Cylinder"));
	Cylinder->AttachTo(TransformBase);

	// Common properties
	ExpandedVertically = false;
	ExpandedHorizontally = false;

	PrimaryActorTick.bCanEverTick = true;
	MaxScaleHeight = 10.f;
	MaxScaleWidth = 10.f;
	ExpansionSpeed = 4.f;
}


void ACylinderWall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;

	TArray<AActor *> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<class AActor*> OutActors;

	if (!ExpandedVertically)
	{
		UKismetSystemLibrary::ComponentOverlapActors_NEW(Cylinder, Cylinder->GetComponentTransform(), ObjectTypes, nullptr, ActorsToIgnore, OutActors);

		if (OutActors.Num() == 0)
		{
			FVector Aux = TransformBase->RelativeScale3D;
			Aux.Z += DeltaSeconds * ExpansionSpeed;
			TransformBase->SetRelativeScale3D(Aux);

			if (Aux.Z > MaxScaleHeight)
			{
				ExpandedVertically = true;
			}
		}
		else
		{
			ExpandedVertically = true;
		}
	}
	else
	{
		if (!ExpandedHorizontally)
		{
			UKismetSystemLibrary::ComponentOverlapActors_NEW(Cylinder, Cylinder->GetComponentTransform(), ObjectTypes, nullptr, ActorsToIgnore, OutActors);

			if (OutActors.Num() == 0)
			{
				FVector Aux = TransformBase->RelativeScale3D;
				Aux.X += DeltaSeconds * ExpansionSpeed;
				Aux.Y += DeltaSeconds * ExpansionSpeed;
				TransformBase->SetRelativeScale3D(Aux);

				if (Aux.X > MaxScaleWidth)
				{
					ExpandedHorizontally = true;
				}
			}
			else
			{
				ExpandedHorizontally = true;
			}
		}
	}
}


