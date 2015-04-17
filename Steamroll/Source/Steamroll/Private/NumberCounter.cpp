// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "NumberCounter.h"
#include "Engine.h"

ANumberCounter::ANumberCounter(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Count = 0;

	Root = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	RootComponent = Root;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("StaticMesh'/Game/UI/Inventory/Inv_Small.Inv_Small'"));

	NumberLeftCube = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("NumberLeftCube"));
	NumberLeftCube->SetStaticMesh(Cube.Object);
	NumberLeftCube->AttachTo(RootComponent);

	NumberRightCube = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("NumberRightCube"));
	NumberRightCube->SetStaticMesh(Cube.Object);
	NumberRightCube->AttachTo(RootComponent);

	NumberLeftText = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, TEXT("NumberLeftText"));
	NumberLeftText->SetText("0");
	NumberLeftText->AttachTo(NumberLeftCube);

	NumberRightText = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, TEXT("NumberRightText"));
	NumberRightText->SetText("0");
	NumberRightText->AttachTo(NumberRightCube);

	bRotating = false;
	TimeRotating = 0.f;

	PrimaryActorTick.bCanEverTick = true;
}


void ANumberCounter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bRotating)
	{
		TimeRotating += DeltaSeconds;

		if (TimeRotating > 1.f)
		{
			bRotating = false;
			TimeRotating = 0.f;
			NumberLeftCube->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
			NumberRightCube->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
		}
		else
		{			
			NumberLeftCube->AddRelativeRotation(FRotator(5000.f * DeltaSeconds, 0.f, 0.f));
			NumberRightCube->AddRelativeRotation(FRotator(3000.f * DeltaSeconds, 0.f, 0.f));
		}
	}
}

