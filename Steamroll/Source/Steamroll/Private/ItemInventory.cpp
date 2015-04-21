// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "ItemInventory.h"
#include "Engine.h"
#include "SlotContentConfig.h"
#include "InvCounter.h"
#include "NumberCounter.h"


// Sets default values
AItemInventory::AItemInventory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Frame"));
	RootComponent = Frame;

	NumCounters = 6;
	Counters.Reserve(NumCounters);

	Counter0 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter0");
	Counter1 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter1");
	Counter2 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter2");
	Counter3 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter3");
	Counter4 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter4");
	Counter5 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter5");
	//Counter6 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter6");
	
	Counters.Add(Counter0);
	Counters.Add(Counter1);
	Counters.Add(Counter2);
	Counters.Add(Counter3);
	Counters.Add(Counter4);
	Counters.Add(Counter5);
	//Counters.Add(Counter6);

	for (auto& Counter : Counters)
	{
		Counter->AttachTo(RootComponent);
	}
}


void AItemInventory::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Counters.Last() && Counters.Last()->ChildActor)
	{
		AInvCounter* NumCounter = Cast<AInvCounter>(Counters.Last()->ChildActor);

		NumCounter->CoverTop->SetPlayRate(-1.f);
		NumCounter->CoverTop->Play(false);

		NumCounter->CoverBottom->SetPlayRate(-1.f);
		NumCounter->CoverBottom->Play(false);

		NumCounter->Item->SetMaterial(0, BallMaterial);
	}
}

// Called when the game starts or when spawned
void AItemInventory::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemInventory::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}


AInvCounter* AItemInventory::GetCounter(int32 Index)
{
	return Cast<AInvCounter>(Counters[Index]->ChildActor);
}


void AItemInventory::UpdateInventory(const TArray<FSlotContentConfigStruct>& SlotContent)
{
	for (auto& Slot : SlotContent)
	{
		if (Slot.SlotType == ESlotTypeEnum::SE_EMPTY)
		{
			continue;
		}

		bool bFound = false;
		AInvCounter* FreeCounter = nullptr;

		for (int32 i = 0; i < Counters.Num(); ++i)
		{
			AInvCounter* Counter = GetCounter(i);

			if (Slot.SlotType == Counter->ItemType)
			{
				bFound = true;
				Counter->SetNumber(Slot.Quantity);

				break;
			} 
			else if (Counter->ItemType == ESlotTypeEnum::SE_EMPTY && i < Counters.Num() - 1)
			{
				FreeCounter = Counter;
			}
		}

		if (!bFound && FreeCounter)
		{
			FreeCounter->ItemType = Slot.SlotType;

			switch (Slot.SlotType)
			{
			case ESlotTypeEnum::SE_WALL:
				FreeCounter->Item->SetMaterial(0, WallMaterial);
				break;

			case ESlotTypeEnum::SE_EXPL:
				FreeCounter->Item->SetMaterial(0, ExplosionMaterial);
				break;

			case ESlotTypeEnum::SE_RAMP:
				FreeCounter->Item->SetMaterial(0, RampMaterial);
				break;

			default:
				break;
			}

			FreeCounter->CoverTop->SetPlayRate(-1.f);
			FreeCounter->CoverTop->Play(false);

			FreeCounter->CoverBottom->SetPlayRate(-1.f);
			FreeCounter->CoverBottom->Play(false);

			FreeCounter->SetNumber(Slot.Quantity);
		}
	}
}


void AItemInventory::UpdateNumBalls(int32 NumBalls)
{
	if (Counters.Last() && Counters.Last()->ChildActor)
	{
		AInvCounter* NumCounter = Cast<AInvCounter>(Counters.Last()->ChildActor);

		NumCounter->SetNumber(NumBalls);
	}
}


void AItemInventory::ClearInventory()
{
	for (int32 i = 0; i < Counters.Num() - 1; ++i)
	{
		AInvCounter* Counter = GetCounter(i);

		if (Counter)
		{
			Counter->ItemType = ESlotTypeEnum::SE_EMPTY;

			// Close all covers
			Counter->CoverTop->SetPosition(10.f);
			Counter->CoverBottom->SetPosition(10.f);
		}
	}
}

