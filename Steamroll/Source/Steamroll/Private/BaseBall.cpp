

#include "Steamroll.h"
#include "BaseBall.h"


ABaseBall::ABaseBall(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	Activated = true;

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

	DraggingBallSetSphere(Sphere);

	PrimaryActorTick.bCanEverTick = true;
}


void ABaseBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	bool bTouchingFloor = IsTouchingFloor();
	float SpeedSquared = GetVelocity().SizeSquared();

	DraggingBallTick(DeltaSeconds, bTouchingFloor, SpeedSquared);
}


void ABaseBall::Fire(float ChargeTime)
{
	if (Activated)
	{
		DraggingBallReset();
		Activated = false;

		// Firing direction
		FVector Direction = AimTransform->GetComponentToWorld().TransformVector(FVector(1.f, 0.f, 0.f));
		Direction.Z = 0.f;

		// Firing power
		float LaunchPower = ChargeTime / FiringTimeout;
		float LaunchSpeed = FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, LaunchPower);
		Sphere->SetPhysicsLinearVelocity(Direction * LaunchSpeed);

		ExplosionClient();
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


bool ABaseBall::IsActivated()
{
	return Activated;
}


void ABaseBall::DraggingBallActivate()
{
	Activated = true;
}


void ABaseBall::DraggingBallStop()
{
	Sphere->PutRigidBodyToSleep();
	Sphere->SetAllPhysicsLinearVelocity(FVector(0.f, 0.f, 0.f));
	Sphere->SetAllPhysicsAngularVelocity(FVector(0.f, 0.f, 0.f));
}


bool ABaseBall::IsTouchingFloor() const
{
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;
	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	return GetWorld()->LineTraceSingle(
		RV_Hit, //result
		GetActorLocation(), //start
		GetActorLocation() + FVector(0.f, 0.f, -Sphere->GetScaledSphereRadius() - 1.f), //end
		ECC_PhysicsBody, //collision channel
		RV_TraceParams
		);
}

