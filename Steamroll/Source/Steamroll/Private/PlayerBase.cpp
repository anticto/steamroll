

#include "Steamroll.h"
#include "PlayerBase.h"
#include "SteamrollPlayerController.h"
#include "SteamrollBall.h"

#include "EngineUtils.h"


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
	Camera->bUseControllerViewRotation = false; // We don't want the controller rotating the camera

	// Create explosion particle system
	Explosion = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Explosion0"));
	Explosion->bAutoActivate = false;
}


void APlayerBase::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// set up gameplay key bindings
	InputComponent->BindAxis("MoveForward", this, &APlayerBase::MoveForward);

	InputComponent->BindAction("Undo", IE_Pressed, this, &APlayerBase::Undo);
	InputComponent->BindAction("RemoteTrigger", IE_Pressed, this, &APlayerBase::ActivateBall);
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


void APlayerBase::Fire(float ChargeTime)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.Instigator = Instigator;
	SpawnParams.bNoCollisionFail = true;

	ASteamrollBall* Ball = GetWorld()->SpawnActor<ASteamrollBall>(WhatToSpawn, AimTransform->GetComponentLocation(), AimTransform->GetComponentRotation(), SpawnParams);
	SetLastDeployedActor(Ball);
	
	if (Ball)
	{
		Ball->AddActorLocalOffset(FVector(500.f, 0.f, 0.f));
		FVector Direction = AimTransform->GetComponentToWorld().TransformVector(FVector(1.f, 0.f, 0.f));
		float LaunchPower = ChargeTime / FiringTimeout;
		float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower);
		Ball->Sphere->SetPhysicsLinearVelocity(Direction * LaunchSpeed);
		//Ball->Sphere->SetPhysicsAngularVelocity(Direction * 50000000);
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
			ActorItr->ActivateBall();
		}
	}
}

