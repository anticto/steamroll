// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "PlayerBase.h"
#include "SteamrollPlayerController.h"
#include "SteamrollBall.h"
#include "SteamrollSphereComponent.h"
#include "TrajectoryComponent.h"
#include "PhysicsVirtualSphereComponent.h"
#include "DeploymentSpot.h"

#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"


APlayerBase::APlayerBase(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Create mesh component for the base
	Base = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Base0"));
	Base->SetSimulatePhysics(false);
	Base->SetMobility(EComponentMobility::Movable);
	RootComponent = Base;

	AimTransform->AttachTo(Base);

	// Create mesh component for the cannon
	Cannon = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Cannon0"));
	Cannon->SetSimulatePhysics(false);
	Cannon->SetMobility(EComponentMobility::Movable);
	Cannon->AttachTo(AimTransform);
	Cannon->SetRelativeScale3D(FVector(2.f, 2.f, 4.f));
	Cannon->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	// Create a camera boom attached to the root (base)
	SpringArm = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("BaseSpringArm0"));
	SpringArm->AttachTo(AimTransform);
	SpringArm->bDoCollisionTest = true;
	SpringArm->RelativeRotation = FRotator(-25.f, 0.f, 0.f);
	SpringArm->TargetArmLength = 500.f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 3.f;

	// Create a camera and attach to boom	
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
	Camera2->AttachTo(RootComponent);
	Camera2->bUsePawnControlRotation = false;

	// Create explosion particle system
	Explosion = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Explosion0"));
	Explosion->bAutoActivate = false;

	TargetChargeTime = 0.f;
	ChargeTime = 0.f;
	ChargeTimeSpeed = 2.f;

	// Simulated walls for ball trajectory prediction
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Object0(TEXT("StaticMesh'/Game/Ball/Prediction/paretStatic.paretStatic'"));

	for (int32 i = 0; i < 4; ++i)
	{
		SimulatedWalls[i] = PCIP.CreateAbstractDefaultSubobject<UStaticMeshComponent>(this, *FString::Printf(TEXT("SimulatedWall%i"), i));

		SimulatedWalls[i]->SetStaticMesh(Object0.Object);
		SimulatedWalls[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SimulatedWalls[i]->SetVisibility(false);
		SimulatedWalls[i]->SetAbsolute(true, true);
		SimulatedWalls[i]->SetRelativeTransform(FTransform::Identity);
	}

	NumUsedSimulatedWalls = 0;

	// Simulated ramps for ball trajectory prediction
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Object1(TEXT("StaticMesh'/Game/Ball/Prediction/rampaStatic.rampaStatic'"));

	for (int32 i = 0; i < 4; ++i)
	{
		SimulatedRamps[i] = PCIP.CreateAbstractDefaultSubobject<UStaticMeshComponent>(this, *FString::Printf(TEXT("SimulatedRamp%i"), i));
		
		SimulatedRamps[i]->SetStaticMesh(Object1.Object);
		SimulatedRamps[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SimulatedRamps[i]->SetVisibility(false);
		SimulatedRamps[i]->SetAbsolute(true, true);
		SimulatedRamps[i]->SetRelativeTransform(FTransform::Identity);
	}

	NumUsedSimulatedRamps = 0;

	// Simulated explosions for ball trajectory prediction
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Object2(TEXT("StaticMesh'/Game/Ball/Prediction/explosionStatic.explosionStatic'"));

	for (int32 i = 0; i < 4; ++i)
	{
		SimulatedExplosions[i] = PCIP.CreateAbstractDefaultSubobject<UStaticMeshComponent>(this, *FString::Printf(TEXT("SimulatedExplosion%i"), i));

		SimulatedExplosions[i]->SetStaticMesh(Object2.Object);
		SimulatedExplosions[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SimulatedExplosions[i]->SetVisibility(false);
		SimulatedExplosions[i]->SetAbsolute(true, true, true);
		SimulatedExplosions[i]->SetRelativeTransform(FTransform::Identity);
	}

	NumUsedSimulatedExplosions = 0;
	SelectedSlot = 1;

	CumulativeYaw = 0.f;
	SteppedYaw = 0.f;
	CurrentYaw = 0.f;
	bMovedDuringTick = false;
	SecondsWithoutMoving = 0.f;
	PrevSign = 0.f;

	PrimaryActorTick.bCanEverTick = true;
}


void APlayerBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	for (int32 i = 0; i < 4; ++i)
	{
		SimulatedWalls[i]->CreateAndSetMaterialInstanceDynamic(0);
		SimulatedRamps[i]->CreateAndSetMaterialInstanceDynamic(0);
	}
}


void APlayerBase::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// set up gameplay key bindings
	InputComponent->BindAxis("MoveRightBase", this, &APlayerBase::MoveRight);
	InputComponent->BindAxis("MoveRightBaseKey", this, &APlayerBase::MoveRightKey);
	InputComponent->BindAxis("MoveForward", this, &APlayerBase::MoveForward);
	InputComponent->BindAxis("TriggerAxis", this, &APlayerBase::Trigger);

	InputComponent->BindAction("Undo", IE_Pressed, this, &APlayerBase::Undo);
	InputComponent->BindAction("RemoteTrigger", IE_Pressed, this, &APlayerBase::ActivateBall);

	InputComponent->BindAction("ChargeUp", IE_Pressed, this, &APlayerBase::SetChargeUp);
	InputComponent->BindAction("ChargeDown", IE_Pressed, this, &APlayerBase::SetChargeDown);
}


void APlayerBase::FireCharge()
{
	if (ChargeTime == 0.f)
	{
		bFiring = true;
	}
	else
	{
		FireServer(ChargeTime, AimTransform->RelativeRotation);
		TargetChargeTime = 0.f;
		ChargeTime = 0.f;
	}
}


void APlayerBase::FireRelease()
{
	bFiring = false;
}


ASteamrollBall* APlayerBase::CreateSimulatedBall()
{
	ASteamrollBall* SimulatedBall = Cast<ASteamrollBall>(GetWorld()->SpawnActor(ASteamrollBall::StaticClass()));

	if (SimulatedBall)
	{
		SimulatedBall->Sphere->bSimulationBall = true;
		SimulatedBall->SetActorEnableCollision(false);
		SimulatedBall->Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SimulatedBall->Sphere->SetSimulatePhysics(false);
		SimulatedBall->SetActorLocation(GetActorLocation());
		SimulatedBall->Sphere->SetSphereRadius(99.442101f);
		SimulatedBall->Sphere->PlayerBase = this;

		SimulatedBall->VirtualSphere->SetSimulatePhysics(false);
		SimulatedBall->VirtualSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (AttachedToDeploymentSpot)
		{
			SimulatedBall->ExplosionRadius = AttachedToDeploymentSpot->ExplosionRadius;
		}
	}

	return SimulatedBall;
}


void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	SimulatedBall = CreateSimulatedBall();
	CurrentYaw = AimTransform->GetComponentRotation().Yaw;
	SteppedYaw = CurrentYaw;

	if (AttachedToDeploymentSpot && AttachedToDeploymentSpot->CameraList.Num() != 0)
	{
		GetLocalPlayerController()->SetViewTarget(AttachedToDeploymentSpot->CameraList[AttachedToDeploymentSpot->CameraListActiveIndex]);
	}
}


void APlayerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Aiming
	if (bMovedDuringTick)
	{
		SecondsWithoutMoving = 0.f;
		bMovedDuringTick = false;
	}
	else
	{
		SecondsWithoutMoving += DeltaSeconds;
	}

	if (!FMath::IsNearlyEqual(CurrentYaw, SteppedYaw, 0.05f))
	{
		float Increment = SteppedYaw - CurrentYaw;

		// Prevent yaw wrap-around when going from -180 to 180 degrees
		if (FMath::Abs(Increment) > 180.f && FMath::Sign(SteppedYaw) != FMath::Sign(CurrentYaw))
		{
			Increment = -FMath::Sign(Increment) * (360.f - FMath::Abs(Increment));
		}
		
		float IncrementSpeed = FMath::Max(FMath::Abs(Increment * 8.f), 5.f);

		float TimedIncrement = FMath::Sign(Increment) * DeltaSeconds * IncrementSpeed;
		CurrentYaw += Increment >= 0.f ? FMath::Min(Increment, TimedIncrement) : FMath::Max(Increment, TimedIncrement);
		CurrentYaw = FMath::UnwindDegrees(CurrentYaw);
		AimTransform->SetWorldRotation(FRotator(0.f, CurrentYaw, 0.f));
		//UE_LOG(LogTemp, Warning, TEXT("Inc to %f, speed=%f"), CurrentYaw, IncrementSpeed);
	}
	else if (CurrentYaw != SteppedYaw)
	{
		CurrentYaw = SteppedYaw;
		CurrentYaw = FMath::UnwindDegrees(CurrentYaw);
		AimTransform->SetWorldRotation(FRotator(0.f, CurrentYaw, 0.f));
		//UE_LOG(LogTemp, Warning, TEXT("Snap to %f"), CurrentYaw);
	}

	// Charging, firing and simulation
	if (bFiring)
	{
		ChargeTime += DeltaSeconds;
		ChargeTime = FMath::Min(ChargeTime, FiringTimeout);
		TargetChargeTime = ChargeTime;
	}

	if (TargetChargeTime > ChargeTime)
	{
		ChargeTime = FMath::Min(TargetChargeTime, ChargeTime + ChargeTimeSpeed * DeltaSeconds);
	}
	else if (TargetChargeTime < ChargeTime)
	{
		ChargeTime = FMath::Max(TargetChargeTime, ChargeTime - ChargeTimeSpeed * DeltaSeconds);
	}

	if (SimulatedBall)
	{
		for (auto& Ball : SimulatedBall->WallReboundPredictionBalls)
		{
			Ball->Destroy();
		}

		SimulatedBall->Destroy();
		SimulatedBall = nullptr;
	}

	ClearSimulatedItems();

	if (ChargeTime > 0.f)
	{
		if (!SimulatedBall)
		{
			SimulatedBall = CreateSimulatedBall();
		}

		TArray<AActor*> OverlappingActors, ActorsToIgnore;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		FVector LaunchLocation = GetLaunchLocation();
		ActorsToIgnore.Add(this);

		bool bOverlap = UKismetSystemLibrary::SphereOverlapActors_NEW(GetWorld(), LaunchLocation, 100.f, ObjectTypes, nullptr, ActorsToIgnore, OverlappingActors);

		if (bOverlap)
		{
			bOverlap = false;

			for (auto Actor : OverlappingActors)
			{
				UStaticMeshComponent* Component = Cast<UStaticMeshComponent>(Actor->GetRootComponent());
				if (Component && Component->GetCollisionProfileName() != FName("DynamicPhysicsExceptBall"))
				{
					bOverlap = true;
					break;
				}
			}
		}

		if (!bOverlap)
		{
			auto PlayerController = GetLocalPlayerController();
			if (PlayerController)
			{
				SimulatedBall->SlotsConfig = PlayerController->SlotsConfig;
			}

			SimulatedBall->SetActorLocation(LaunchLocation);
			SimulatedBall->SetVelocity(GetLaunchVelocity(ChargeTime));

			SimulatedBall->Sphere->TrajectoryComponent->DeleteLocations();
			
			float TotalTime = SimulatedBall->Sphere->UpdateBallPhysics(10.f);
			auto MaterialInstance = SimulatedBall->Sphere->TrajectoryComponent->CreateAndSetMaterialInstanceDynamicFromMaterial(0, SimulatedBall->Sphere->TrajectoryComponent->GetMaterial(0));
			MaterialInstance->SetScalarParameterValue("Charging", ChargeTime != TargetChargeTime ? 1.f : 0.f);

			SimulatedBall->Sphere->TrajectoryComponent->SendData();

			SimulatedBall->SetActorEnableCollision(false);
			SimulatedBall->Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SimulatedBall->Sphere->SetSimulatePhysics(false);
			SimulatedBall->SetActorLocation(GetActorLocation());
		}
	}
}


void APlayerBase::MoveForward(float Val)
{
	if (Val != 0)
	{
		AimTransform->AddRelativeRotation(FRotator(-Val, 0.f, 0.f));

		//FRotator Rotation = AimTransform->GetComponentRotation();
		FRotator Rotation = AimTransform->RelativeRotation;

		if (Rotation.Pitch < 0.f)
		{
			//Debug(FString::Printf(TEXT("Pitch=%f"), Rotation.Pitch));
			Rotation.Pitch = 0.f;
			AimTransform->SetRelativeRotation(Rotation);
		}
		else if (Rotation.Pitch > 65.f)
		{
			Rotation.Pitch = 65.f;
			AimTransform->SetRelativeRotation(Rotation);
		}

		RotationServer(AimTransform->RelativeRotation);
	}
}


FVector APlayerBase::GetLaunchLocation() const
{
	FVector FiringOffset = AimTransform->GetComponentToWorld().TransformVector(FVector(450.f, 0.f, +120.f));

	return AimTransform->GetComponentLocation() + FiringOffset;
}


FVector APlayerBase::GetLaunchVelocity(float ChargeTime) const
{
	FVector Direction = AimTransform->GetComponentToWorld().TransformVector(FVector(1.f, 0.f, 0.f));
	float LaunchPower = ChargeTime / FiringTimeout;
	float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower) * AttachedToDeploymentSpot->LaunchPowerFactor;

	return Direction * LaunchSpeed;
}


void APlayerBase::Fire(float ChargeTime)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.Instigator = Instigator; // Must set instigator because otherwise the game logic will assume the ball was placed in the level by a designer and won't copy the player's slot configuration
	SpawnParams.bNoCollisionFail = false;

	FVector LaunchLocation = GetLaunchLocation();

	TArray<AActor*> OverlappingActors, ActorsToIgnore;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ActorsToIgnore.Add(this);

	bool bOverlap = UKismetSystemLibrary::SphereOverlapActors_NEW(GetWorld(), LaunchLocation, 100.f, ObjectTypes, nullptr, ActorsToIgnore, OverlappingActors);

	bool bAllItemsAvailable = GetLocalPlayerController()->CheckItemAvailability();

	if (bOverlap)
	{
		bOverlap = false;

		for (auto Actor : OverlappingActors)
		{
			UStaticMeshComponent* Component = Cast<UStaticMeshComponent>(Actor->GetRootComponent());
			if (Component && Component->GetCollisionProfileName() != FName("DynamicPhysicsExceptBall"))
			{
				bOverlap = true;
				break;
			}
		}
	}

	if (!bOverlap && bAllItemsAvailable)
	{
		ASteamrollBall* Ball = GetWorld()->SpawnActor<ASteamrollBall>(WhatToSpawn, LaunchLocation, AimTransform->GetComponentRotation(), SpawnParams);

		if (Ball)
		{
			SetLastDeployedActor(Ball);
			FVector LaunchVelocity = GetLaunchVelocity(ChargeTime);
			Ball->SetVelocity(LaunchVelocity);
			Ball->VirtualSphere->SetWorldLocation(LaunchLocation);
			Ball->VirtualSphere->SetPhysicsLinearVelocity(LaunchVelocity);
			Ball->ExplosionRadius = AttachedToDeploymentSpot->ExplosionRadius;

			GetLocalPlayerController()->SpendBallsInDeploymentSpot(1);
			GetLocalPlayerController()->SpendItemsInSlots();
			ExplosionClient();
		}
	}

	ClearSimulatedItems();
}


void APlayerBase::Explode()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.Instigator = Instigator;

	AEmitter* Emitter = GetWorld()->SpawnActor<AEmitter>(AEmitter::StaticClass(), AimTransform->GetComponentLocation(), AimTransform->GetComponentRotation(), SpawnParams);
	Emitter->SetTemplate(Explosion->Template);
	Emitter->AddActorLocalOffset(FVector(400.f, 0.f, 0.f));
	Emitter->SetActorScale3D(FVector(4.f, 4.f, 4.f));
	Emitter->SetLifeSpan(4.f);
}


void APlayerBase::Undo()
{
	ASteamrollPlayerController* SteamrollPlayerController = Cast<ASteamrollPlayerController>(this->GetController());

	if (SteamrollPlayerController && SteamrollPlayerController->LastDeployedActor)
	{
		TArray<AActor*> AttachedActors;
		SteamrollPlayerController->LastDeployedActor->GetAttachedActors(AttachedActors);

		for (auto Child : AttachedActors)
		{
			Child->Destroy();
		}

		SteamrollPlayerController->LastDeployedActor->Destroy();
		SteamrollPlayerController->LastDeployedActor = nullptr;
	}
}


void APlayerBase::SetLastDeployedActor(AActor* Actor)
{
	ASteamrollPlayerController* SteamrollPlayerController = Cast<ASteamrollPlayerController>(this->GetController());

	if (SteamrollPlayerController)
	{
		SteamrollPlayerController->LastDeployedActor = Actor;
	}
}


void APlayerBase::ActivateBall()
{
	for (TActorIterator<ASteamrollBall> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (!ActorItr->Activated && ActorItr->HasSlotState(ESlotTypeEnum::SE_IMP))
		{
			if (ActorItr->IsTouchingFloor())
			{
				ActorItr->Sphere->SetPhysicsLinearVelocity(ActorItr->Sphere->GetPhysicsLinearVelocity() + FVector(0.f, 0.f, 1000.f));
			}
		}
		else
		{
			ActorItr->ActivateRemoteTriggers();
		}
	}
}


void APlayerBase::DestroyBalls()
{
	Super::DestroyBalls();

	TargetChargeTime = 0.f;
}


void APlayerBase::FireDebug1() { TargetChargeTime = 0.1f * FiringTimeout; }
void APlayerBase::FireDebug2() { TargetChargeTime = 0.2f * FiringTimeout; }
void APlayerBase::FireDebug3() { TargetChargeTime = 0.3f * FiringTimeout; }
void APlayerBase::FireDebug4() { TargetChargeTime = 0.4f * FiringTimeout; }
void APlayerBase::FireDebug5() { TargetChargeTime = 0.5f * FiringTimeout; }
void APlayerBase::FireDebug6() { TargetChargeTime = 0.6f * FiringTimeout; }
void APlayerBase::FireDebug7() { TargetChargeTime = 0.7f * FiringTimeout; }
void APlayerBase::FireDebug8() { TargetChargeTime = 0.8f * FiringTimeout; }
void APlayerBase::FireDebug9() { TargetChargeTime = 0.9f * FiringTimeout; }
void APlayerBase::FireDebug10() { TargetChargeTime = 1.0f * FiringTimeout; }


void APlayerBase::SetChargeUp()
{
	TargetChargeTime += 0.1f * FiringTimeout;
	TargetChargeTime = FMath::Min(TargetChargeTime, FiringTimeout);
}


void APlayerBase::SetChargeDown()
{
	TargetChargeTime -= 0.1f * FiringTimeout;
	TargetChargeTime = FMath::Max(TargetChargeTime, 0.f);
}


void APlayerBase::Trigger(float Val)
{
	if (Val != 0.f)
	{
		ChargeTime += 0.02f * Val * FiringTimeout;
		ChargeTime = FMath::Clamp(ChargeTime, 0.f, FiringTimeout);

		TargetChargeTime = ChargeTime;
	}
}


void APlayerBase::ClearSimulatedItems()
{
	for (int32 i = 0; i < 4; ++i)
	{
		SimulatedWalls[i]->SetVisibility(false);
		SimulatedRamps[i]->SetVisibility(false);
		SimulatedExplosions[i]->SetVisibility(false);
	}

	NumUsedSimulatedWalls = 0;
	NumUsedSimulatedRamps = 0;
	NumUsedSimulatedExplosions = 0;
}


void APlayerBase::DrawSimulatedWall(const FVector &Location, const FRotator& Rotation, float CurrentTime, uint32 SlotIndex)
{
	if (NumUsedSimulatedWalls < 4)
	{
		SimulatedWalls[NumUsedSimulatedWalls]->SetRelativeLocation(Location);
		SimulatedWalls[NumUsedSimulatedWalls]->SetRelativeRotation(Rotation);
		SimulatedWalls[NumUsedSimulatedWalls]->SetVisibility(true);

		UMaterialInstanceDynamic* MatInstance = Cast<UMaterialInstanceDynamic>(SimulatedWalls[NumUsedSimulatedWalls]->GetMaterial(0));

		if (MatInstance)
		{
			MatInstance->SetScalarParameterValue("ManualTime", CurrentTime * 0.1f/* * 256.f*/);
			MatInstance->SetScalarParameterValue("Charging", ChargeTime != TargetChargeTime ? 1.f : 0.f);
			MatInstance->SetScalarParameterValue("Selected", SlotIndex == SelectedSlot ? 1.f : 0.15f);
		}

		//SimulatedWallsMaterials[NumUsedSimulatedWalls]->SetScalarParameterValue("ManualTime", CurrentTime * 0.1f/* * 256.f*/);

		NumUsedSimulatedWalls++;
	}
}


void APlayerBase::DrawSimulatedRamp(const FVector &Location, const FRotator& Rotation, float CurrentTime, uint32 SlotIndex)
{
	if (NumUsedSimulatedRamps < 4)
	{
		SimulatedRamps[NumUsedSimulatedRamps]->SetRelativeLocation(Location);
		SimulatedRamps[NumUsedSimulatedRamps]->SetRelativeRotation(Rotation);
		SimulatedRamps[NumUsedSimulatedRamps]->SetVisibility(true);

		UMaterialInstanceDynamic* MatInstance = Cast<UMaterialInstanceDynamic>(SimulatedRamps[NumUsedSimulatedRamps]->GetMaterial(0));

		if (MatInstance)
		{
			MatInstance->SetScalarParameterValue("ManualTime", CurrentTime * 0.1f/* * 256.f*/);
			MatInstance->SetScalarParameterValue("Charging", ChargeTime != TargetChargeTime ? 1.f : 0.f);
			MatInstance->SetScalarParameterValue("Selected", SlotIndex == SelectedSlot ? 1.f : 0.25f);
		}

		NumUsedSimulatedRamps++;
	}
}


void APlayerBase::DrawSimulatedExplosion(const FVector &Location, float Radius, float CurrentTime, uint32 SlotIndex)
{
	if (NumUsedSimulatedExplosions < 4)
	{
		SimulatedExplosions[NumUsedSimulatedExplosions]->SetRelativeLocation(Location);
		SimulatedExplosions[NumUsedSimulatedExplosions]->SetRelativeScale3D(FVector(Radius));
		SimulatedExplosions[NumUsedSimulatedExplosions]->SetVisibility(true);

		NumUsedSimulatedExplosions++;
	}
}


void APlayerBase::FireBlueprint()
{
	Fire(ChargeTime);
}


void APlayerBase::MoveRight(float Val)
{
	if (Val != 0.f)
	{		
		float Step = GetLocalPlayerController()->AimingStep;

		//bMovedDuringTick = true;

		if (FMath::Abs(CumulativeYaw) > 10.f && (SecondsWithoutMoving > 0.25f || (FMath::Sign(Val) != PrevSign && FMath::Sign(CumulativeYaw) != PrevSign)))
		{
			bMovedDuringTick = true;
			SecondsWithoutMoving = 0.f;
			CumulativeYaw = FMath::Sign(Val) * (Step + Step * 0.05f); // Make sure we force a step and don't miss it due to precision issues
		}
		else
		{
			CumulativeYaw += 2.f * Val;

			if (SecondsWithoutMoving == 0.f)
			{
				bMovedDuringTick = true;
			}
		}

		if (bMovedDuringTick)
		{
			PrevSign = FMath::Sign(Val);

			while (FMath::Abs(CumulativeYaw) >= Step)
			{
				if (CumulativeYaw > 0.f)
				{
					CumulativeYaw -= Step;
					SteppedYaw += Step;
				}
				else
				{
					CumulativeYaw += Step;
					SteppedYaw -= Step;
				}
			}

			SteppedYaw = FMath::UnwindDegrees(SteppedYaw);
		}

		//RotationServer(AimTransform->RelativeRotation);
	}
}


void APlayerBase::MoveRightKey(float Val)
{
	static float PrevVal = 0.f;

	if (Val != 0.f)
	{
		float Step = GetLocalPlayerController()->AimingStep;

		if (PrevVal != 0.f)
		{
			CumulativeYaw += 2.f * Val;
		}
		else
		{
			CumulativeYaw = FMath::Sign(Val) * (Step + Step * 0.05f); // Make sure we force a step and don't miss it due to precision issues
		}

		while (FMath::Abs(CumulativeYaw) >= Step)
		{
			if (CumulativeYaw > 0.f)
			{
				CumulativeYaw -= Step;
				SteppedYaw += Step;
			}
			else
			{
				CumulativeYaw += Step;
				SteppedYaw -= Step;
			}			
		}

		SteppedYaw = FMath::UnwindDegrees(SteppedYaw);
	}

	PrevVal = Val;
}


void APlayerBase::AttachToDeploymentSpot(class ADeploymentSpot* DeploymentSpot)
{
	AttachedToDeploymentSpot = DeploymentSpot;
	AttachedToDeploymentSpot->SetActorHiddenInGame(true);
}


ADeploymentSpot* APlayerBase::DetatchFromDeploymentSpot()
{
	if (AttachedToDeploymentSpot)
	{
		AttachedToDeploymentSpot->SetActorHiddenInGame(false);
		AttachedToDeploymentSpot->bHidden = false;
		
		auto Aux = AttachedToDeploymentSpot;
		AttachedToDeploymentSpot = nullptr;

		return Aux;
	}

	return nullptr;
}


AActor* APlayerBase::GetViewTargetActor()
{
	if (AttachedToDeploymentSpot && AttachedToDeploymentSpot->CameraList.Num() > 0)
	{
		return AttachedToDeploymentSpot->CameraList[AttachedToDeploymentSpot->CameraListActiveIndex];
	}

	return this;
}

