

#include "Steamroll.h"
#include "DynamicWall.h"
#include "Kismet/KismetSystemLibrary.h"


ADynamicWall::ADynamicWall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Base transform
	TransformBase = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("TransformBase"));
	RootComponent = TransformBase;

	// Left wall part
	TransformLeft = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("TransformLeft"));
	TransformLeft->AttachTo(TransformBase);

	WallLeft = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("WallLeft"));
	WallLeft->AttachTo(TransformLeft);
	
	ExpandedHorizontallyLeft = false;

	// Right wall part
	TransformRight = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("TransformRight"));
	TransformRight->AttachTo(TransformBase);

	WallRight = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("WallRight"));
	WallRight->AttachTo(TransformRight);

	ExpandedHorizontallyRight = false;

	// Common properties
	ExpandedVertically = false;
	PrimaryActorTick.bCanEverTick = true;
	MaxScaleHeight = 3.f;
	MaxScaleWidth = 3.f;
}


void ADynamicWall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;

	TArray<AActor *> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<class AActor*> OutActorsLeft, OutActorsRight;

	if (!ExpandedVertically)
	{					
		UKismetSystemLibrary::ComponentOverlapActors_NEW(WallLeft, WallLeft->GetComponentTransform(), ObjectTypes, nullptr, ActorsToIgnore, OutActorsLeft);
		UKismetSystemLibrary::ComponentOverlapActors_NEW(WallRight, WallRight->GetComponentTransform(), ObjectTypes, nullptr, ActorsToIgnore, OutActorsLeft);
		
		if (OutActorsLeft.Num() == 0 && OutActorsRight.Num() == 0)
		{
			FVector Aux = TransformLeft->RelativeScale3D;
			Aux.Z += DeltaSeconds;
			TransformLeft->SetRelativeScale3D(Aux);

			Aux = TransformRight->RelativeScale3D;
			Aux.Z += DeltaSeconds;
			TransformRight->SetRelativeScale3D(Aux);

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
		if (!ExpandedHorizontallyLeft)
		{
			UKismetSystemLibrary::ComponentOverlapActors_NEW(WallLeft, WallLeft->GetComponentTransform(), ObjectTypes, nullptr, ActorsToIgnore, OutActorsLeft);

			if (OutActorsLeft.Num() == 0)
			{
				FVector Aux = TransformLeft->RelativeScale3D;
				Aux.X += DeltaSeconds;
				TransformLeft->SetRelativeScale3D(Aux);

				if (Aux.X > MaxScaleWidth)
				{
					ExpandedHorizontallyLeft = true;
				}
			}
			else
			{
				ExpandedHorizontallyLeft = true;
			}
		}

		if (!ExpandedHorizontallyRight)
		{
			UKismetSystemLibrary::ComponentOverlapActors_NEW(WallRight, WallRight->GetComponentTransform(), ObjectTypes, nullptr, ActorsToIgnore, OutActorsRight);

			if (OutActorsRight.Num() == 0)
			{
				FVector Aux = TransformRight->RelativeScale3D;
				Aux.X += DeltaSeconds;
				TransformRight->SetRelativeScale3D(Aux);

				if (Aux.X > MaxScaleWidth)
				{
					ExpandedHorizontallyRight = true;
				}
			}
			else
			{
				ExpandedHorizontallyRight = true;
			}
		}
	}
}


