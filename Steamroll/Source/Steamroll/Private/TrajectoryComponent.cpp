// Fill out your copyright notice in the Description page of Project Settings.

#include "Steamroll.h"
#include "TrajectoryComponent.h"
#include "SceneManagement.h"
#include "DrawDebugHelpers.h"


UTrajectoryComponent::UTrajectoryComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UMaterial> Object0(TEXT("Material'/Game/Ball/TrajectoryMat.TrajectoryMat'"));
	//static ConstructorHelpers::FObjectFinder<UMaterial> Object0(TEXT("Material'/Game/Base/M_StatueGlass.M_StatueGlass'"));
	TrajectoryMat = Object0.Object;

	SetMaterial(0, TrajectoryMat);

	bAddedFirstLocation = false;
	bLastPValid = false;
	Radius = 50.f;
	CumulativeV = 0.f;
	LastXAxis = FVector::ZeroVector;

	this->SetAbsolute(true, true);
}


void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View, TArray<FVector> *SimulatedLocations, float Radius, UMaterial* Material)
{
	//QUICK_SCOPE_CYCLE_COUNTER(STAT_DrawCylinderSceneProxy_DrawDynamicElements);
	//DrawSphere(PDI, FVector(0, 0, 500), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);

	if (!SimulatedLocations || SimulatedLocations->Num() == 0)
	{
		return;
	}
	else if (SimulatedLocations->Num() == 1)
	{
		DrawSphere(PDI, SimulatedLocations->operator[](0), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);
		return;
	}

	DrawSphere(PDI, SimulatedLocations->operator[](0), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);

	uint32 Step = 1;
	FVector LastLocation = SimulatedLocations->operator[](0);

	for (int32 Itr = Step; Itr < SimulatedLocations->Num() - 1; Itr += Step)
	{				
		//DrawDebugCylinder(GetWorld(), SimulatedLocations->operator[](Itr - Step), SimulatedLocations->operator[](Itr), Sphere->GetScaledSphereRadius(), 10, FColor::Blue, false, -1.f);
		FVector ZAxis = SimulatedLocations->operator[](Itr) - LastLocation;//SimulatedLocations->operator[](Itr - Step);
		float Len = ZAxis.SizeSquared();

		if (Len < FMath::Square(50.f))
		{
			continue;
		}

		Len = FMath::Sqrt(Len);
		ZAxis /= Len;
		FVector XAxis = FMath::Abs(ZAxis | FVector(0.f, 0.f, 1.f)) < 0.9f ? FVector::CrossProduct(ZAxis, FVector(0.f, 0.f, 1.f)) : FVector::CrossProduct(ZAxis, FVector(1.f, 0.f, 0.f));
		FVector YAxis = FVector::CrossProduct(ZAxis, XAxis);
		FVector Origin = (SimulatedLocations->operator[](Itr - Step) + SimulatedLocations->operator[](Itr)) / 2.f;

		//DrawSphere(PDI, SimulatedLocations->operator[](Itr), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);
		DrawCylinder(PDI, Origin, XAxis, YAxis, ZAxis, Radius, Len / 2.f, 5, Material->GetRenderProxy(false), 0);
		//DrawDebugLine(GetWorld(), SimulatedLocations->operator[](Itr - Step), SimulatedLocations->operator[](Itr), FColor::Blue, false, -1.f, 0, 2.f * Sphere->GetScaledSphereRadius());

		LastLocation = SimulatedLocations->operator[](Itr);
	}

	DrawSphere(PDI, SimulatedLocations->operator[](SimulatedLocations->Num() - 1), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);
}


void UTrajectoryComponent::AddLocation(const FVector& Location, bool bOverrideMinDistBetweenLocations)
{
	const static float MinDistBetweenLocations = 50.f;
	const static float VStretchFactor = 0.01f;

	if (!bAddedFirstLocation)
	{
		LastLocation = Location;
		bAddedFirstLocation = true;
	}
	else
	{
		FVector Displacement = Location - LastLocation;
		float Len = Displacement.Size();
		FVector XAxis = Displacement / Len;

		if (Len < MinDistBetweenLocations && !bOverrideMinDistBetweenLocations)
		{
			return;		
		}

		//if (!bOverrideMinDistBetweenLocations && LastXAxis != FVector::ZeroVector && (XAxis | LastXAxis) < 0.5f)
		//{
		//	FVector InterpolatedLocation = (LastLocation + Location) / 2.f + LastXAxis * (MinDistBetweenLocations * 0.5f);
		//	//Swap(LastP0, LastP1);

		//	AddLocation(InterpolatedLocation, true);

		//	LastLocation = InterpolatedLocation;
		//	Displacement = Location - LastLocation;
		//	Len = Displacement.Size();
		//	XAxis = Displacement / Len;
		//}

		FVector YAxis = FMath::Abs(XAxis | FVector::UpVector) < 0.9f ? FVector::CrossProduct(XAxis, FVector::UpVector) : FVector::CrossProduct(XAxis, FVector(1.f, 0.f, 0.f));
		FVector ZAxis = FVector::CrossProduct(XAxis, YAxis);

		// The 4 vertices of the quad
		FVector p0 = Location - YAxis * Radius + ZAxis * Radius;
		FVector p1 = Location + YAxis * Radius + ZAxis * Radius;
		FVector p2;
		FVector p3;

		if (bLastPValid)
		{
			p2 = LastP1;
			p3 = LastP0;
		}
		else
		{
			p2 = p1 - Displacement;
			p3 = p0 - Displacement;
			bLastPValid = true;
		}

		FProceduralMeshVertex v0;
		FProceduralMeshVertex v1;
		FProceduralMeshVertex v2;
		FProceduralMeshVertex v3;
		
		v2.U = 1.f; v2.V = CumulativeV;
		v3.U = 0.f; v3.V = CumulativeV;

		CumulativeV += Len * VStretchFactor;

		v0.U = 0.f; v0.V = CumulativeV;
		v1.U = 1.f; v1.V = CumulativeV;

		FProceduralMeshTriangle t1;
		FProceduralMeshTriangle t2;
		v0.Position = p0;
		v1.Position = p1;
		v2.Position = p2;
		v3.Position = p3;
		t1.Vertex0 = v3;
		t1.Vertex1 = v0;
		t1.Vertex2 = v1;
		t2.Vertex0 = v3;
		t2.Vertex1 = v1;
		t2.Vertex2 = v2;
		ProceduralMeshTris.Add(t1);
		ProceduralMeshTris.Add(t2);

		//DrawDebugLine(GetWorld(), FVector(0, 0, 0), FVector(1000, 1000, 1000), FColor::Red);
		//DrawDebugLine(GetWorld(), p0, p1, FColor::Red);
		//DrawDebugLine(GetWorld(), p1, p2, FColor::Red);
		//DrawDebugLine(GetWorld(), p2, p3, FColor::Red);
		//DrawDebugLine(GetWorld(), p3, p0, FColor::Red);

		LastLocation = Location;
		LastP0 = p0;
		LastP1 = p1;
		LastXAxis = XAxis;
	}
}


void UTrajectoryComponent::DeleteLocations()
{
	ProceduralMeshTris.Empty();
	bAddedFirstLocation = false;
	bLastPValid = false;
	CumulativeV = 0.f;
	LastXAxis = FVector::ZeroVector;
}


void UTrajectoryComponent::SendData()
{
	//UpdateCollision();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
}

