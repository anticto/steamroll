

#include "Steamroll.h"
#include "Portal.h"
#include "SteamrollBall.h"


APortal::APortal(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	BoxPortal = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Box"));
	RootComponent = BoxPortal;

	BoxPortal->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnBeginOverlap);
}


void APortal::OnBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ASteamrollBall* SteamrollBall = Cast<ASteamrollBall>(OtherActor);

	if (SteamrollBall && LinkedPortal)
	{
		FTransform Inverse = GetTransform().Inverse();

		FVector LocalLocation = Inverse.TransformPosition(SteamrollBall->GetActorLocation());
		FVector LocalVelocity = Inverse.TransformVector(SteamrollBall->Sphere->GetPhysicsLinearVelocity());
		FVector LocalAngularVelocity = Inverse.TransformVector(SteamrollBall->Sphere->GetPhysicsAngularVelocity());

		FVector WorldTransformedLocation = LinkedPortal->GetTransform().TransformPosition(LocalLocation);
		FVector WorldTransformedVelocity = LinkedPortal->GetTransform().TransformVector(LocalVelocity);
		FVector WorldTransformedAngularVelocity = LinkedPortal->GetTransform().TransformVector(LocalAngularVelocity);

		SteamrollBall->Sphere->SetPhysicsLinearVelocity(WorldTransformedVelocity);
		SteamrollBall->Sphere->SetPhysicsAngularVelocity(WorldTransformedAngularVelocity);
		SteamrollBall->TeleportTo(WorldTransformedLocation, SteamrollBall->GetActorRotation());
	}
}


