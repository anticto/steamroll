

#include "Steamroll.h"
#include "LaserSight.h"
#include "Engine.h"


ALaserSight::ALaserSight(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	FirstHit = false;
	SecondHit = false;

	PrimaryActorTick.bCanEverTick = true;
}


void ALaserSight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float RayLength = 50000.f;
	float Epsilon = 0.001f;

	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bTraceAsyncScene = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;
	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);
	FVector DirectionVector = GetActorForwardVector() * RayLength;

	if (GetWorld()->LineTraceSingle(
		RV_Hit, //result
		GetActorLocation(), //start
		GetActorLocation() + DirectionVector, //end
		ECC_PhysicsBody, //collision channel
		RV_TraceParams
		))
	{
		FirstHit = true;
		FirstHitLocation = RV_Hit.ImpactPoint;
		
		FHitResult RV_Hit2(ForceInit);
		ReflectedVector = DirectionVector.MirrorByVector(RV_Hit.ImpactNormal);

		if (GetWorld()->LineTraceSingle(
			RV_Hit2, //result
			FirstHitLocation + RV_Hit.ImpactNormal * Epsilon, //start
			FirstHitLocation + ReflectedVector, //end
			ECC_PhysicsBody, //collision channel
			RV_TraceParams
			))
		{
			SecondHit = true;
			SecondHitLocation = RV_Hit2.ImpactPoint;		
		}
		else
		{
			SecondHit = false;
		}
	}
	else
	{
		FirstHit = false;
		SecondHit = false;
	}
}

