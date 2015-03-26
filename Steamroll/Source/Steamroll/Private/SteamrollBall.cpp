// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollBall.h"
#include "Engine.h"
#include "SteamrollPlayerController.h"
#include "UnrealMathUtility.h"
#include "SteamrollSphereComponent.h"
#include "BallTunnel.h"
#include "PhysicsVirtualSphereComponent.h"


ASteamrollBall::ASteamrollBall(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Sphere = PCIP.CreateDefaultSubobject<USteamrollSphereComponent>(this, TEXT("Sphere"));
	RootComponent = Sphere;

	VirtualSphere = PCIP.CreateDefaultSubobject<UPhysicsVirtualSphereComponent>(this, TEXT("VirtualSphere"));
	VirtualSphere->AttachTo(RootComponent);

	Activated = false;

	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;

	CollectedQuantity = 0.f;
	CollectCapacity = 10.f;

	bExplosionBlockedByContactSlot = false;
	CurrentTime = 0.f;
	TimeForNextPaint = 1.f;
	bTravellingInTunnel = false;
}


void ASteamrollBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Sphere->SteamrollTick(DeltaSeconds);

	VirtualSphere->SteamrollTick(DeltaSeconds, Sphere);

	//bool bTouchingFloor = IsTouchingFloor();

	//float SpeedSquared = GetVelocity().SizeSquared();

	//static const float StoppingSpeed = 10.f;
	//if (HasSlotState(ESlotTypeEnum::SE_PAINT))
	//{
	//	if (!Activated && SpeedSquared >= StoppingSpeed * StoppingSpeed && bTouchingFloor)
	//	{
	//		TimeForNextPaint -= DeltaSeconds;

	//		if (TimeForNextPaint <= 0.f)
	//		{
	//			SplatPaint();
	//			TimeForNextPaint = 1.f;
	//		}
	//	}
	//}

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


TEnumAsByte<ESlotTypeEnum::SlotType> ASteamrollBall::GetSlotActivatorType(int32 SlotIndex)
{
	return SlotsConfig.GetSlotActivatorType(SlotIndex);
}


void ASteamrollBall::SetSlotActivatorType(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.SetSlotActivatorType(SlotIndex, SlotTypeEnum);
}


float ASteamrollBall::GetSlotAngle(int32 SlotIndex)
{
	return SlotsConfig.GetSlotAngle(SlotIndex);
}


void ASteamrollBall::SetSlotAngle(int32 SlotIndex, float Value)
{
	SlotsConfig.SetSlotAngle(SlotIndex, Value);
}


float ASteamrollBall::GetSlotTime(int32 SlotIndex)
{
	return SlotsConfig.GetSlotTime(SlotIndex);
}


void ASteamrollBall::SetSlotTime(int32 SlotIndex, float Value)
{
	SlotsConfig.SetSlotTime(SlotIndex, Value);
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

	//VirtualSphere->SetWorldLocation(Sphere->GetComponentLocation());
	//VirtualSphere->SetPhysicsLinearVelocity(Sphere->Velocity);
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
	//DrawDebugString(GetWorld(), GetActorLocation(), FString::Printf(TEXT("%f"), CurrentTime), nullptr, FColor::Blue, 2.f);
	ActivateSlot(SlotIndex);
}


void ASteamrollBall::ActivateRemoteTriggers()
{
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotActivatorType(i) == ESlotTypeEnum::SE_REMOTE)
		{
			ActivateSlot(i);
		}
	}
}


bool ASteamrollBall::ActivateSlot(int32 SlotIndex)
{
	TEnumAsByte<ESlotTypeEnum::SlotType> Type = GetSlotState(SlotIndex);

	if (!SlotsConfig.IsSlotUsed(SlotIndex) && Type != ESlotTypeEnum::SE_EMPTY && Type != ESlotTypeEnum::SE_RAMP) // Ramps snap when hit a wall, they aren't directly activated with triggers
	{
		SlotsConfig.SetSlotUsed(SlotIndex);
		ActivateSlotEvent(SlotsConfig.GetSlotType(SlotIndex), SlotsConfig.GetSlotAngle(SlotIndex), SlotsConfig.GetSlotTime(SlotIndex));

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


void ASteamrollBall::SnapRampEvent_Implementation(const FVector& Location, const FVector& Normal)
{

}


void ASteamrollBall::ExecuteTransport(ABallTunnel* ConnectedTunnel, float Speed)
{
	ConnectedTunnel->bDiscovered = true;

	ASteamrollBall* Ball = this;

	//FVector NewLocation = ConnectedTunnel->GetActorLocation();
	//FVector NewVelocity = ConnectedTunnel->Mesh->GetUpVector() * (Speed * ConnectedTunnel->SpeedMultiplier);
	//Ball->TeleportTo(NewLocation, Ball->GetActorRotation(), false, true);
	//Ball->SetVelocity(NewVelocity);

	//Ball->VirtualSphere->SetWorldLocation(NewLocation);
	//Ball->VirtualSphere->SetPhysicsLinearVelocity(NewVelocity);

	Ball->Sphere->bPhysicsEnabled = true;
	Ball->SetActorHiddenInGame(false);
	Ball->SetActorEnableCollision(true);
	Ball->Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Ball->bTravellingInTunnel = false;

	Ball->VirtualSphere->SetSimulatePhysics(true);
}


void ASteamrollBall::ReceiveHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	
	if (MyComp == VirtualSphere && Other->IsRootComponentMovable() && !Cast<ASteamrollBall>(Other) 
		&& !(OtherComp->GetCollisionProfileName() == FName("OnlyDynamicPhysics")) && !(OtherComp->GetCollisionProfileName() == FName("DynamicPhysicsExceptBall")))
	{
		VirtualSphere->ReceiveHit(Sphere, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	}
}


bool ASteamrollBall::HasUnusedExplosionSlot()
{
	for (int32 i = 1; i < 5; i++)
	{
		if (SlotsConfig.GetSlotType(i) == ESlotTypeEnum::SE_EXPL && !SlotsConfig.IsSlotUsed(i))
		{
			return true;
		}
	}

	return false;
}

