// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "BaseBall.h"
#include "SteamrollSphereComponent.h"
#include "PhysicsVirtualSphereComponent.h"
#include "SteamrollBall.h"


ABaseBall::ABaseBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Activated = true;

	Sphere = PCIP.CreateDefaultSubobject<USteamrollSphereComponent>(this, TEXT("Sphere"));
	SphereMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("SphereMesh"));
	GyroMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("GyroMesh"));	
	SpringArm = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("BaseSpringArm0"));

	VirtualSphere = PCIP.CreateDefaultSubobject<UPhysicsVirtualSphereComponent>(this, TEXT("VirtualSphere"));
	VirtualSphere->AttachTo(RootComponent);

	RootComponent = Sphere;
	AimTransform->AttachTo(RootComponent);
	SphereMesh->AttachTo(RootComponent);
	GyroMesh->AttachTo(AimTransform);
	SpringArm->AttachTo(AimTransform);
	Camera->AttachTo(SpringArm);

	PrimaryActorTick.bCanEverTick = true;

	Sphere->DragCoefficientSlow = 10.f;
}


void ABaseBall::Tick(float DeltaSeconds)
{
	static const float StoppingSpeed = 10.f;
	static const float MaxSpeed = 5000.f;

	Super::Tick(DeltaSeconds);
	Sphere->SteamrollTick(DeltaSeconds);
	VirtualSphere->SteamrollTick(DeltaSeconds, Sphere);

	if (bFiring)
	{
		// Firing direction
		FVector Direction = AimTransform->GetComponentToWorld().TransformVector(FVector(1.f, 0.f, 0.f));
		Direction.Z = 0.f;

		// Firing power
		float LaunchPower = DeltaSeconds / FiringTimeout;
		float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower);
		Sphere->Velocity += Direction * LaunchSpeed;

		float Speed = Sphere->Velocity.Size();

		if (Speed > MaxSpeed)
		{
			Sphere->Velocity = Sphere->Velocity / Speed * MaxSpeed;
		}
	}

	bool bTouchingFloor = Sphere->IsTouchingFloor();

	if (!Activated && bTouchingFloor && Sphere->Velocity.SizeSquared() < FMath::Square(StoppingSpeed))
	{
		Activated = true;
		Sphere->Velocity = FVector::ZeroVector;
	}
}


void ABaseBall::Fire(float ChargeTime)
{
	if (Activated)
	{
		Activated = false;
	}
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


void ABaseBall::ReceiveHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (MyComp == VirtualSphere && Other->IsRootComponentMovable() && !Cast<ASteamrollBall>(Other) && !(OtherComp->GetCollisionProfileName() == FName("OnlyDynamicPhysics")))
	{
		VirtualSphere->ReceiveHit(Sphere, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	}
}


void ABaseBall::UndeployInstantly_Implementation()
{

}

