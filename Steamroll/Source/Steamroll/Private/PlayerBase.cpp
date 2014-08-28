

#include "Steamroll.h"
#include "PlayerBase.h"
#include "SteamRollTestBall.h"

#include "TimerManager.h"
#include "Engine.h"


APlayerBase::APlayerBase(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseMesh(TEXT("/Game/Meshes/TemplateCube_Rounded.TemplateCube_Rounded"));

	// Create mesh component for the base
	Base = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Base0"));
	Base->SetStaticMesh(BaseMesh.Object);
	Base->SetSimulatePhysics(false);
	Base->SetMobility(EComponentMobility::Static);
	RootComponent = Base;

	AimTransform = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Tranform0"));
	AimTransform->AttachTo(Base);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CannonMesh(TEXT("/Game/Shapes/Shape_Cylinder.Shape_Cylinder"));

	// Create mesh component for the cannon
	Cannon = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Cannon0"));
	Cannon->SetStaticMesh(CannonMesh.Object);
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

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionParticleSystem(TEXT("/Game/Particles/P_Explosion.P_Explosion"));

	// Create explosion particle system
	Explosion = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("Explosion0"));
	Explosion->bAutoActivate = false;
	Explosion->SetTemplate(ExplosionParticleSystem.Object);

	FiringTimeout = 2.f;
	MinLaunchSpeed = 1500.f;
	MaxLaunchSpeed = 20000.f;
}


void APlayerBase::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	InputComponent->BindAxis("MoveRight", this, &APlayerBase::MoveRight);
	InputComponent->BindAxis("MoveForward", this, &APlayerBase::MoveForward);

	InputComponent->BindAction("Fire", IE_Pressed, this, &APlayerBase::FireCharge);
	InputComponent->BindAction("Fire", IE_Released, this, &APlayerBase::FireRelease);
}


void APlayerBase::MoveRight(float Val)
{
	AimTransform->AddRelativeRotation(FRotator(0.f, Val, 0.f));
}


void APlayerBase::MoveForward(float Val)
{
	AimTransform->AddRelativeRotation(FRotator(-Val, 0.f, 0.f));

	FRotator Rotation = AimTransform->GetComponentRotation();

	if (Rotation.Pitch < 0.f)
	{
		Rotation.Pitch = 0.f;
		AimTransform->SetRelativeRotation(Rotation);
	}
	else if (Rotation.Pitch > 65.f)
	{
		Rotation.Pitch = 65.f;
		AimTransform->SetRelativeRotation(Rotation);
	}
}


void APlayerBase::Timeout()
{
	Fire(FiringTimeout);
}


void APlayerBase::Fire(float ChargeTime)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.Instigator = Instigator;

	ASteamRollTestBall* Ball = GetWorld()->SpawnActor<ASteamRollTestBall>(ASteamRollTestBall::StaticClass(), AimTransform->GetComponentLocation(), AimTransform->GetComponentRotation(), SpawnParams);
	Ball->AddActorLocalOffset(FVector(400.f, 0.f, 0.f));
	FVector Direction = AimTransform->GetComponentToWorld().TransformVector(FVector(1.f, 0.f, 0.f));
	float LaunchPower = ChargeTime / FiringTimeout;
	float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower);
	Ball->Ball->SetPhysicsLinearVelocity(Direction * LaunchSpeed);

	//Explosion->Activate(true);

	AEmitter* Emitter = GetWorld()->SpawnActor<AEmitter>(AEmitter::StaticClass(), AimTransform->GetComponentLocation(), AimTransform->GetComponentRotation(), SpawnParams);
	Emitter->SetTemplate(Explosion->Template);
	Emitter->AddActorLocalOffset(FVector(400.f, 0.f, 0.f));
	Emitter->SetActorScale3D(FVector(4.f, 4.f, 4.f));
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
		Debug(FString::Printf(TEXT("ChargeTime=%f"), TimeElapsed));

		Fire(TimeElapsed);

		GetWorldTimerManager().ClearTimer(this, &APlayerBase::Timeout);
	}
}


void APlayerBase::Debug(FString Msg)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, Msg);
	}
}

