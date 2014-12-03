

#include "Steamroll.h"
#include "PlayerBase.h"
#include "SteamrollPlayerController.h"
#include "SteamrollBall.h"
#include "SteamrollSphereComponent.h"
#include "TrajectoryComponent.h"

#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"


APlayerBase::APlayerBase(const class FPostConstructInitializeProperties& PCIP)
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

	ChargeTime = 0.f;

	PrimaryActorTick.bCanEverTick = true;
}


void APlayerBase::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// set up gameplay key bindings
	InputComponent->BindAxis("MoveForward", this, &APlayerBase::MoveForward);

	InputComponent->BindAction("Undo", IE_Pressed, this, &APlayerBase::Undo);
	InputComponent->BindAction("RemoteTrigger", IE_Pressed, this, &APlayerBase::ActivateBall);
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
		//SimulatedBall->bHidden = false;
		SimulatedBall->Sphere->bSimulationBall = true;
		SimulatedBall->SetActorEnableCollision(false);
		SimulatedBall->Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SimulatedBall->Sphere->SetSimulatePhysics(false);
		SimulatedBall->SetActorLocation(GetActorLocation());
		SimulatedBall->Sphere->SetSphereRadius(99.442101f);
	}

	return SimulatedBall;
}


void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	SimulatedBall = CreateSimulatedBall();
}


void APlayerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFiring)
	{
		ChargeTime += DeltaSeconds;
		ChargeTime = FMath::Min(ChargeTime, FiringTimeout);
	}

	if (SimulatedBall)
	{
		SimulatedBall->Destroy();
		SimulatedBall = nullptr;
	}

	if (ChargeTime > 0.f)
	{
		if (!SimulatedBall)
		{
			SimulatedBall = CreateSimulatedBall();
		}

		TArray<AActor*> OverlappingActors, ActorsToIgnore;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		FVector LaunchLocation = GetLaunchLocation();

		if (!UKismetSystemLibrary::SphereOverlapActors_NEW(GetWorld(), LaunchLocation, 100.f, ObjectTypes, nullptr, ActorsToIgnore, OverlappingActors))
		{
			auto PlayerController = GetLocalPlayerController();
			if (PlayerController)
			{
				SimulatedBall->SlotsConfig = PlayerController->SlotsConfig;
			}

			SimulatedBall->SetActorLocation(LaunchLocation);
			SimulatedBall->SetVelocity(GetLaunchVelocity(ChargeTime));

			SimulatedBall->Sphere->TrajectoryComponent->SimulatedLocations.Empty();
			
			SimulatedBall->Sphere->UpdateBallPhysics(10.f);
			
			//SimulatedBall->DrawPhysicalSimulation();

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
	float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower);

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

	if (!UKismetSystemLibrary::SphereOverlapActors_NEW(GetWorld(), LaunchLocation, 100.f, ObjectTypes, nullptr, ActorsToIgnore, OverlappingActors))
	{
		ASteamrollBall* Ball = GetWorld()->SpawnActor<ASteamrollBall>(WhatToSpawn, LaunchLocation, AimTransform->GetComponentRotation(), SpawnParams);

		if (Ball)
		{
			SetLastDeployedActor(Ball);
			Ball->SetVelocity(GetLaunchVelocity(ChargeTime));
		}
	}

	ExplosionClient();
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

	ChargeTime = 0.f;
}


void APlayerBase::FireDebug1() { ChargeTime = 0.1f * FiringTimeout; }
void APlayerBase::FireDebug2() { ChargeTime = 0.2f * FiringTimeout; }
void APlayerBase::FireDebug3() { ChargeTime = 0.3f * FiringTimeout; }
void APlayerBase::FireDebug4() { ChargeTime = 0.4f * FiringTimeout; }
void APlayerBase::FireDebug5() { ChargeTime = 0.5f * FiringTimeout; }
void APlayerBase::FireDebug6() { ChargeTime = 0.6f * FiringTimeout; }
void APlayerBase::FireDebug7() { ChargeTime = 0.7f * FiringTimeout; }
void APlayerBase::FireDebug8() { ChargeTime = 0.8f * FiringTimeout; }
void APlayerBase::FireDebug9() { ChargeTime = 0.9f * FiringTimeout; }
void APlayerBase::FireDebug10() { ChargeTime = 1.0f * FiringTimeout; }

