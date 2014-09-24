

#include "Steamroll.h"
#include "BaseBall.h"


ABaseBall::ABaseBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Sphere = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("Sphere"));
	SphereMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("SphereMesh"));
	GyroMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("GyroMesh"));	
	SpringArm = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("BaseSpringArm0"));

	RootComponent = Sphere;
	AimTransform->AttachTo(RootComponent);
	SphereMesh->AttachTo(RootComponent);
	GyroMesh->AttachTo(AimTransform);
	SpringArm->AttachTo(AimTransform);
	Camera->AttachTo(SpringArm);
}


void ABaseBall::Fire(float ChargeTime)
{
	// Firing direction
	FVector Direction = AimTransform->GetComponentToWorld().TransformVector(FVector(1.f, 0.f, 0.f));
	Direction.Z = 0.f;

	// Firing power
	float LaunchPower = ChargeTime / FiringTimeout;
	float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower);
	Sphere->SetPhysicsLinearVelocity(Direction * LaunchSpeed);

	// Fix camera and gyro rotation
	FRotator Rotator = SpringArm->GetComponentRotation();
	SpringArm->bAbsoluteRotation = true;
	SpringArm->SetWorldRotation(Rotator);

	GyroMesh->bAbsoluteRotation = true;

	ExplosionClient();
}


void ABaseBall::Explode()
{
	//FActorSpawnParameters SpawnParams;
	//SpawnParams.Owner = nullptr;
	//SpawnParams.Instigator = Instigator;

	//AEmitter* Emitter = GetWorld()->SpawnActor<AEmitter>(AEmitter::StaticClass(), AimTransform->GetComponentLocation(), AimTransform->GetComponentRotation(), SpawnParams);
	//Emitter->SetTemplate(Explosion->Template);
	//Emitter->AddActorLocalOffset(FVector(400.f, 0.f, 0.f));
	//Emitter->SetActorScale3D(FVector(4.f, 4.f, 4.f));
	//Emitter->SetLifeSpan(4.f);
}


