

#include "Steamroll.h"
#include "PlayerBase.h"
#include "SteamrollBall.h"
#include "SteamrollPlayerController.h"

#include "TimerManager.h"
#include "Engine.h"
#include "UnrealNetwork.h"


APlayerBase::APlayerBase(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Create mesh component for the base
	Base = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Base0"));
	Base->SetSimulatePhysics(false);
	Base->SetMobility(EComponentMobility::Movable);
	RootComponent = Base;

	AimTransform = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Tranform0"));
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
	Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("BaseCamera0"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUseControllerViewRotation = false; // We don't want the controller rotating the camera

	// Create explosion particle system
	Explosion = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Explosion0"));
	Explosion->bAutoActivate = false;

	FiringTimeout = 2.f;
	MinLaunchSpeed = 50.f;
	MaxLaunchSpeed = 20000.f;

	RaiseCamera = false;
	LowerCamera = false;
	CameraSpeed = 300.f;
	MaxHeight = 500.f;
	MinHeight = 0.f;
	CurrCameraTime = 0.f;

	PrimaryActorTick.bCanEverTick = true;
}


void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;

		if (CameraManager)
		{
			CameraAnimInst = CameraManager->PlayCameraAnim(CameraAnim, 0.f, 1.f, 0.f, 0.f, true);
		}
	}
}


void APlayerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (RaiseCamera)
	{
		CurrCameraTime += DeltaSeconds;
		CurrCameraTime = FMath::Min(CurrCameraTime, CameraAnim->AnimLength);
	}

	if (LowerCamera)
	{
		CurrCameraTime -= DeltaSeconds;
		CurrCameraTime = FMath::Max(CurrCameraTime, 0.001f); // 0.001 not 0 becase going back to 0 would sometimes make te camera anim loop around to the end due to precision issues
	}

	SetCameraAnim(CurrCameraTime, DeltaSeconds); // Multiplied by AnimLength to map the 0-1 interval to the anim duration

	//if (RaiseCamera)
	//{
	//	Camera->AddRelativeLocation(FVector(0.f, 0.f, CameraSpeed * DeltaSeconds));

	//	if (Camera->RelativeLocation.Z > MaxHeight)
	//	{
	//		Camera->SetRelativeLocation(FVector(Camera->RelativeLocation.X, Camera->RelativeLocation.Y, MaxHeight));
	//	}
	//}

	//if (LowerCamera)
	//{
	//	Camera->AddRelativeLocation(FVector(0.f, 0.f, -CameraSpeed * DeltaSeconds));

	//	if (Camera->RelativeLocation.Z < MinHeight)
	//	{
	//		Camera->SetRelativeLocation(FVector(Camera->RelativeLocation.X, Camera->RelativeLocation.Y, MinHeight));
	//	}
	//}
}


void APlayerBase::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	InputComponent->BindAxis("MoveRight", this, &APlayerBase::MoveRight);
	InputComponent->BindAxis("MoveForward", this, &APlayerBase::MoveForward);

	InputComponent->BindAction("Fire", IE_Pressed, this, &APlayerBase::FireCharge);
	InputComponent->BindAction("Fire", IE_Released, this, &APlayerBase::FireRelease);

	InputComponent->BindAction("FireDebug_1", IE_Released, this, &APlayerBase::FireDebug1);
	InputComponent->BindAction("FireDebug_2", IE_Released, this, &APlayerBase::FireDebug2);
	InputComponent->BindAction("FireDebug_3", IE_Released, this, &APlayerBase::FireDebug3);
	InputComponent->BindAction("FireDebug_4", IE_Released, this, &APlayerBase::FireDebug4);
	InputComponent->BindAction("FireDebug_5", IE_Released, this, &APlayerBase::FireDebug5);
	InputComponent->BindAction("FireDebug_6", IE_Released, this, &APlayerBase::FireDebug6);
	InputComponent->BindAction("FireDebug_7", IE_Released, this, &APlayerBase::FireDebug7);
	InputComponent->BindAction("FireDebug_8", IE_Released, this, &APlayerBase::FireDebug8);
	InputComponent->BindAction("FireDebug_9", IE_Released, this, &APlayerBase::FireDebug9);
	InputComponent->BindAction("FireDebug_10", IE_Released, this, &APlayerBase::FireDebug10);

	InputComponent->BindAction("Undo", IE_Pressed, this, &APlayerBase::Undo);
	InputComponent->BindAction("DestroyBalls", IE_Pressed, this, &APlayerBase::DestroyBalls);
	InputComponent->BindAction("RemoteTrigger", IE_Pressed, this, &APlayerBase::ActivateBall);
}


void APlayerBase::MoveRight(float Val)
{
	if (Val != 0)
	{
		AimTransform->AddRelativeRotation(FRotator(0.f, Val, 0.f));

		RotationServer(AimTransform->RelativeRotation);
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


void APlayerBase::Timeout()
{
	FireServer(FiringTimeout, AimTransform->RelativeRotation);
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
	}

	ExplosionClient();
}


void APlayerBase::FireCharge()
{
	GetWorldTimerManager().SetTimer(this, &APlayerBase::Timeout, FiringTimeout, false);
}


void APlayerBase::FireRelease()
{
	// To prevent multiple firing when releasing the fire button check that we actually loaded/charged before releasing
	if (GetWorldTimerManager().IsTimerActive(this, &APlayerBase::Timeout))
	{
		float TimeElapsed = GetWorldTimerManager().GetTimerElapsed(this, &APlayerBase::Timeout);
		GetWorldTimerManager().PauseTimer(this, &APlayerBase::Timeout);

		FireServer(TimeElapsed, AimTransform->RelativeRotation);

		GetWorldTimerManager().ClearTimer(this, &APlayerBase::Timeout);
	}
}


void APlayerBase::FireDebug1() { FireServer(0.1f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug2() { FireServer(0.2f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug3() { FireServer(0.3f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug4() { FireServer(0.4f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug5() { FireServer(0.5f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug6() { FireServer(0.6f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug7() { FireServer(0.7f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug8() { FireServer(0.8f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug9() { FireServer(0.9f * FiringTimeout, AimTransform->RelativeRotation); }
void APlayerBase::FireDebug10() { FireServer(1.0f * FiringTimeout, AimTransform->RelativeRotation); }


void APlayerBase::Debug(FString Msg)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, Msg);
	}
}


bool APlayerBase::FireServer_Validate(float ChargeTime, FRotator Rotation)
{
	return ChargeTime >= 0.f && ChargeTime <= FiringTimeout + 0.1f;
}


void APlayerBase::FireServer_Implementation(float ChargeTime, FRotator Rotation)
{
	AimTransform->SetRelativeRotation(Rotation);
	Fire(ChargeTime);
}


void APlayerBase::ExplosionClient_Implementation()
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


bool APlayerBase::RotationServer_Validate(FRotator Rotation)
{
	return true;
}


void APlayerBase::RotationServer_Implementation(FRotator Rotation)
{
	AimTransform->SetRelativeRotation(Rotation);
	RotationClient(Rotation);
}


void APlayerBase::RotationClient_Implementation(FRotator Rotation)
{
	AimTransform->SetRelativeRotation(Rotation);
}


float APlayerBase::GetCharge()
{
	if (GetWorldTimerManager().IsTimerActive(this, &APlayerBase::Timeout))
	{
		return GetWorldTimerManager().GetTimerElapsed(this, &APlayerBase::Timeout) / FiringTimeout;
	}

	return 0.f;
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


void APlayerBase::DestroyBalls()
{
	for (TActorIterator<ASteamrollBall> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ActorItr->Destroy();
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


void APlayerBase::SetCameraAnim(float Time, float DeltaSeconds)
{
	if (CameraAnim && CameraAnimInst)
	{
		float CurrentTime = CameraAnimInst->CurTime;
		float DeltaTime;

		if (Time >= CurrentTime)
		{
			DeltaTime = Time - CurrentTime;
		}
		else
		{
			float RemainingLength = CameraAnimInst->CamAnim->AnimLength - CurrentTime;
			DeltaTime = Time + RemainingLength;
		}

		CameraAnimInst->Update(1.f, 1.f, 0.f, 0.f, 0.f);
		CameraAnimInst->AdvanceAnim(DeltaTime, true);
		CameraAnimInst->Update(0.f, 1.f, 0.f, 0.f, 0.f);
	}
}

