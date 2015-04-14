// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "ItemInventory.h"
#include "Engine.h"


// Sets default values
AItemInventory::AItemInventory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	NumCounters = 7;
	Counters.Reserve(NumCounters);

	Counter0 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter0");
	Counter1 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter1");
	Counter2 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter2");
	Counter3 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter3");
	Counter4 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter4");
	Counter5 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter5");
	Counter6 = CreateAbstractDefaultSubobject<UChildActorComponent>("Counter6");
	
	Counters.Add(Counter0);
	Counters.Add(Counter1);
	Counters.Add(Counter2);
	Counters.Add(Counter3);
	Counters.Add(Counter4);
	Counters.Add(Counter5);
	Counters.Add(Counter6);

//	static ConstructorHelpers::FObjectFinder<UBlueprint> YourBPOb(TEXT("Blueprint'/Game/HUDScene/Inventory/ItemInventoryBP.ItemInventoryBP'")); 
//	
//	if (YourBPOb.Object != NULL) 
//	{ 
//		//AInvCounter* Class = Cast<AInvCounter>(YourBPOb.Object->GeneratedClass);
//
//		for (int i = 0; i < NumCounters; ++i)
//		{
//			Counters[i]->ChildActorClass = *Cast<TSubclassOf<AActor>>(YourBPOb.Object->GeneratedClass);
//		}
//	}
}


void AItemInventory::PostInitializeComponents()
{
	Super::PostInitializeComponents();
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

