// Fill out your copyright notice in the Description page of Project Settings.

#include "Steamroll.h"
#include "TrajectoryComponent.h"
#include "SceneManagement.h"


UTrajectoryComponent::UTrajectoryComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UMaterial> Object0(TEXT("Material'/Game/Ball/TrajectoryMat.TrajectoryMat'"));
	//static ConstructorHelpers::FObjectFinder<UMaterial> Object0(TEXT("Material'/Game/Base/M_StatueGlass.M_StatueGlass'"));
	TrajectoryMat = Object0.Object;
}


FPrimitiveSceneProxy* UTrajectoryComponent::CreateSceneProxy()
{
	/** Represents a UCapsuleComponent to the scene manager. */
	class FDrawTrajectorySceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FDrawTrajectorySceneProxy(const UTrajectoryComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
		{
			Radius = 50.f;
			SimulatedLocations = &InComponent->SimulatedLocations;
			Material = InComponent->TrajectoryMat;
		}

		/**
		* Draw the scene proxy as a dynamic element
		*
		* @param	PDI - draw interface to render to
		* @param	View - current view
		*/
		virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View) override
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

			uint32 Step = 4;

			for (int32 Itr = Step; Itr < SimulatedLocations->Num() - 1; Itr += Step)
			{				
				//DrawDebugCylinder(GetWorld(), SimulatedLocations->operator[](Itr - Step), SimulatedLocations->operator[](Itr), Sphere->GetScaledSphereRadius(), 10, FColor::Blue, false, -1.f);
				FVector ZAxis = SimulatedLocations->operator[](Itr) - SimulatedLocations->operator[](Itr - Step);
				float Len = ZAxis.Size();
				ZAxis /= Len;
				FVector XAxis = FMath::Abs(ZAxis | FVector(0.f, 0.f, 1.f)) < 0.9f ? FVector::CrossProduct(ZAxis, FVector(0.f, 0.f, 1.f)) : FVector::CrossProduct(ZAxis, FVector(1.f, 0.f, 0.f));
				FVector YAxis = FVector::CrossProduct(ZAxis, XAxis);
				FVector Origin = (SimulatedLocations->operator[](Itr - Step) + SimulatedLocations->operator[](Itr)) / 2.f;

				//DrawSphere(PDI, SimulatedLocations->operator[](Itr), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);
				DrawCylinder(PDI, Origin, XAxis, YAxis, ZAxis, Radius, Len / 2.f, 5, Material->GetRenderProxy(false), 0);
				//DrawDebugLine(GetWorld(), SimulatedLocations->operator[](Itr - Step), SimulatedLocations->operator[](Itr), FColor::Blue, false, -1.f, 0, 2.f * Sphere->GetScaledSphereRadius());
			}

			DrawSphere(PDI, SimulatedLocations->operator[](SimulatedLocations->Num() - 1), FVector(Radius), 10, 10, Material->GetRenderProxy(false), 0, false);
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) override
		{
			const bool bVisible = true;
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = true;// IsShown(View) && bVisible;
			Result.bStaticRelevance = true;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bNormalTranslucencyRelevance = true;
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const TArray<FVector>* SimulatedLocations;
		float Radius;
		UMaterial* Material;
	};

	return new FDrawTrajectorySceneProxy(this);
}


FBoxSphereBounds UTrajectoryComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	FBoxSphereBounds BoxSphereBounds;
	float Extent = BIG_NUMBER;

	BoxSphereBounds.Origin = FVector(0.f, 0.f, 0.f);
	BoxSphereBounds.BoxExtent = FVector(Extent, Extent, Extent);
	BoxSphereBounds.BoxExtent = FVector(Extent, Extent, Extent);
	BoxSphereBounds.SphereRadius = Extent;

	return BoxSphereBounds;
}


bool UTrajectoryComponent::IsZeroExtent() const
{
	return false;
}


FCollisionShape UTrajectoryComponent::GetCollisionShape(float Inflation) const
{
	return FCollisionShape::MakeSphere(1000000.f);
}


bool UTrajectoryComponent::AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const
{
	return false;
}

