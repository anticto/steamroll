// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"
#include "Engine.h"
#include "SteamrollPlayerController.h"
#include "UnrealMathUtility.h"
#include "SteamrollSphereComponent.h"


ASteamrollBall::ASteamrollBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Sphere = PCIP.CreateDefaultSubobject<USteamrollSphereComponent>(this, TEXT("Sphere"));
	RootComponent = Sphere;

	Activated = false;

	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;

	CollectedQuantity = 0.f;
	CollectCapacity = 10.f;

	bExplosionBlockedByContactSlot = false;
	CurrentTime = 0.f;
	TimeForNextPaint = 1.f;

	// If there are timer slots, start timers
	for (int32 i = 1; i < 5; i++)
	{
		bIsTimerRunning[i] = SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_TIME;
	}
}


void ASteamrollBall::Tick(float DeltaSeconds)
{
	static const float StoppingSpeed = 10.f;

	Super::Tick(DeltaSeconds);
	Sphere->SteamrollTick(DeltaSeconds);

	CurrentTime += DeltaSeconds;
	bool bTouchingFloor = IsTouchingFloor();

	float SpeedSquared = GetVelocity().SizeSquared();

	if (!Activated && bTouchingFloor && SpeedSquared < StoppingSpeed)
	{
		ActivateBall();
	}

	for (uint32 i = 1; i < 5; ++i)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_TIME && !SlotsConfig.IsSlotUsed(i))
		{
			if (SlotsConfig.GetSlotParam(i, 0) * 10.f < CurrentTime)
			{
				SlotsConfig.SetSlotUsed(i);
				ActivateTimerTrigger(i);
				bIsTimerRunning[i] = false;
				//Velocity = FVector::ZeroVector;
			}
		}
	}

	if (HasSlotState(ESlotTypeEnum::SE_PAINT))
	{
		if (!Activated && SpeedSquared >= StoppingSpeed * StoppingSpeed && bTouchingFloor)
		{
			TimeForNextPaint -= DeltaSeconds;

			if (TimeForNextPaint <= 0.f)
			{
				SplatPaint();
				TimeForNextPaint = 1.f;
			}
		}
	}

	if (HasSlotState(ESlotTypeEnum::SE_CONTACT))
	{
		bExplosionBlockedByContactSlot = true;

		TArray<AActor*> Actors;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);
		TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
		
		Sphere->GetOverlappingActors(Actors);

		for (auto Actor : Actors)
		{
			if (InterfaceCast<IExplosionDestructibleInterface>(Actor))
			{
				ActivateBall();
				bExplosionBlockedByContactSlot = false;
				break;
			}
		}
	}

	//// Uncoment for debug boxes
	//if (TouchingFloor)
	//{
	//	float r = Sphere->GetScaledSphereRadius();
	//	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(r, r, r), FColor::Green);
	//}

	LastLoc = GetActorLocation();
}


void ASteamrollBall::ActivateBall_Implementation()
{
	Activated = true;
	StopBall();
}


void ASteamrollBall::StopBall()
{
	SetVelocity(FVector::ZeroVector);
}


TEnumAsByte<ESlotTypeEnum::SlotType> ASteamrollBall::GetSlotState(int32 SlotIndex)
{
	return SlotsConfig.GetSlotType(SlotIndex);
}


void ASteamrollBall::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.SetSlotType(SlotIndex, SlotTypeEnum);
}


ASteamrollPlayerController* ASteamrollBall::GetLocalPlayerController()
{
	UWorld* World = GetWorld();

	return World ? Cast<ASteamrollPlayerController>(GEngine->GetFirstLocalPlayerController(World)) : nullptr;
}


bool ASteamrollBall::HasSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	return SlotsConfig.HasSlotType(SlotTypeEnum);
}


int32 ASteamrollBall::CountSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	return SlotsConfig.CountSlotType(SlotTypeEnum);
}


FTransform ASteamrollBall::GetBallFlattenedTransform(const FVector& LastLocation, float WallDeploymentAngle) const
{
	FVector Direction = GetVelocity();
	Direction.Z = 0.f;
	Direction.Normalize();

	FVector Location = GetActorLocation() - FVector(0.f, 0.f, Sphere->GetScaledSphereRadius() - 0.5f);

	FRotator WallDeploymentRotation(0.f, WallDeploymentAngle, 0.f);
	Direction = WallDeploymentRotation.RotateVector(Direction);
	
	return FTransform(Direction.Rotation(), Location, FVector(1.f, 1.f, 1.f));
}


FTransform ASteamrollBall::GetBallAdjustedTransform() const
{
	FVector Location = GetActorLocation() - FVector(0.f, 0.f, Sphere->GetScaledSphereRadius() - 10.f);

	return FTransform(Location);
}


bool ASteamrollBall::IsTouchingFloor() const
{
	return Sphere->IsTouchingFloor(false);
}


void ASteamrollBall::BeginPlay()
{
	// Copy the slot configuration from the player controller
	UWorld *World = GetWorld();
	
	if (Instigator && GEngine && World)
	{
		ASteamrollPlayerController* PlayerController = Cast<ASteamrollPlayerController>(GEngine->GetFirstLocalPlayerController(World));

		if (PlayerController)
		{
			SlotsConfig = PlayerController->SlotsConfig;
		}
	}

	Super::BeginPlay();	
}


void ASteamrollBall::WakeBall()
{
	// Do nothing with the custom physics systems, leave it in case it is called from blueprint
}


void ASteamrollBall::SplatPaint_Implementation()
{
	
}


void ASteamrollBall::ExplosionEvent_Implementation()
{

}


void ASteamrollBall::ActivateTimerTrigger(int32 SlotIndex)
{
	DrawDebugString(GetWorld(), GetActorLocation(), FString::Printf(TEXT("%f"), CurrentTime), nullptr, FColor::Blue, 2.f);
	ActivateConnectedSlots(SlotIndex);
}


void ASteamrollBall::ActivateRemoteTriggers()
{
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_REMOTE)
		{
			ActivateConnectedSlots(i);
		}
	}
}


void ASteamrollBall::ActivateStopTriggers()
{
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_STOP)
		{
			ActivateConnectedSlots(i);
		}
	}
}


void ASteamrollBall::ActivateConnectedSlots(int32 SlotIndex)
{
	//if (IsTouchingFloor())
	//{
		for (uint32 i = 1; i < 5; i++)
		{
			if (SlotsConfig.GetSlotConnection(SlotIndex, i))
			{
				// Only activate the first available slot
				if (ActivateSlot(i)) 
				{
					break;
				}
			}
		}
	//}
}


bool ASteamrollBall::ActivateSlot(int32 SlotIndex)
{
	TEnumAsByte<ESlotTypeEnum::SlotType> Type = GetSlotState(SlotIndex);
	bool bIsTrigger = Type == ESlotTypeEnum::SE_TIME || Type == ESlotTypeEnum::SE_CONTACT || Type == ESlotTypeEnum::SE_REMOTE || Type == ESlotTypeEnum::SE_STOP;

	if (!SlotsConfig.IsSlotUsed(SlotIndex) && !bIsTrigger && Type != ESlotTypeEnum::SE_RAMP) // Ramps snap when hit a wall, they aren't directly activated with triggers
	{
		SlotsConfig.SetSlotUsed(SlotIndex);
		ActivateSlotEvent(SlotsConfig.GetSlotType(SlotIndex), SlotsConfig.GetSlotParam(SlotIndex, 1), SlotsConfig.GetSlotParam(SlotIndex, 2));

		return true;
	}

	return false;
}


FVector ASteamrollBall::GetVelocity() const
{
	return Sphere->Velocity;
}


void ASteamrollBall::SetVelocity(const FVector& NewVelocity)
{
	Sphere->Velocity = NewVelocity;
}


void ASteamrollBall::ActivateSnapRamp(const FVector& Location, const FVector& Normal)
{
	for (int32 i = 1; i < 5; i++)
	{
		bool bConnectedToTrigger = false;

		for (uint32 j = 1; j < 5; j++)
		{
			TEnumAsByte<ESlotTypeEnum::SlotType> Type = GetSlotState(j);
			bool bIsTrigger = Type == ESlotTypeEnum::SE_TIME;

			if (i != j && bIsTrigger && SlotsConfig.GetSlotConnection(j, i))
			{
				if (bIsTimerRunning[j])
				{
					bConnectedToTrigger = true;
					break;
				}
			}
		}

		if (!bConnectedToTrigger && SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_RAMP && !SlotsConfig.IsSlotUsed(i))
		{
			SlotsConfig.SetSlotUsed(i);
			SnapRampEvent(Location, Normal);
			break;
		}
	}
}

