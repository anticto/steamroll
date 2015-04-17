// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "InvCounter.h"
#include "NumberCounter.h"
#include "Engine.h"

AInvCounter::AInvCounter(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	ItemType = ESlotTypeEnum::SE_EMPTY;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ModuleMesh(TEXT("StaticMesh'/Game/UI/Inventory/Inv_Modul.Inv_Modul'"));

	Module = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Module"));
	Module->SetStaticMesh(ModuleMesh.Object);
	RootComponent = Module;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ItemMesh(TEXT("StaticMesh'/Game/UI/Inventory/Inv_Big.Inv_Big'"));

	Item = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Item"));
	Item->SetStaticMesh(ModuleMesh.Object);
	Item->AttachTo(RootComponent);

	NumberCounter = PCIP.CreateDefaultSubobject<UChildActorComponent>(this, TEXT("NumberCounter"));
	NumberCounter->AttachTo(RootComponent);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CoverMesh(TEXT("SkeletalMesh'/Game/UI/Inventory/Inv_Doors.Inv_Doors'"));

	CoverTop = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CoverTop"));
	CoverTop->SetSkeletalMesh(CoverMesh.Object);
	CoverTop->AttachTo(RootComponent);

	CoverBottom = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CoverBottom"));
	CoverBottom->SetSkeletalMesh(CoverMesh.Object);
	CoverBottom->AttachTo(RootComponent);
}


void AInvCounter::SetNumber(int32 Num)
{
	ANumberCounter* ChildActor = Cast<ANumberCounter>(NumberCounter->ChildActor);

	if (Num != ChildActor->Count)
	{
		ChildActor->Count = Num;
		Num = FMath::Clamp(Num, 0, 99);

		ChildActor->NumberLeftText->SetText(FString::Printf(TEXT("%d"), Num / 10));
		ChildActor->NumberRightText->SetText(FString::Printf(TEXT("%d"), Num % 10));

		ChildActor->bRotating = true;
	}
}

