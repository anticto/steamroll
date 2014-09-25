

#include "Steamroll.h"
#include "DraggingBall.h"


DraggingBall::DraggingBall()
{
	StoppingSpeed = 10.f;

	DraggingBallReset();
	StartDraggingTime = 2.f;

	Sphere = nullptr;
}


void DraggingBall::DraggingBallSetSphere(USphereComponent* Sphere)
{
	this->Sphere = Sphere;

	DraggingBallReset();
}


void DraggingBall::DraggingBallReset()
{
	Age = 0.f;
	Dragging = false;

	if (Sphere)
	{
		Sphere->SetLinearDamping(0.01f);
		Sphere->SetAngularDamping(0.f);
	}
}


void DraggingBall::DraggingBallTick(float DeltaSeconds, bool bTouchingFloor, float SpeedSquared)
{
	if (!IsActivated() && SpeedSquared < StoppingSpeed * StoppingSpeed && bTouchingFloor)
	{
		DraggingBallActivate();
	}
	else if (SpeedSquared < StoppingSpeed * StoppingSpeed)
	{
		DraggingBallStop();
	}

	Age += DeltaSeconds;

	if (!Dragging && Age > StartDraggingTime && bTouchingFloor)
	{
		Dragging = true;

		if (Sphere)
		{
			Sphere->SetLinearDamping(3.f);
			Sphere->SetAngularDamping(3.f);
		}
	}
	else if (!bTouchingFloor)
	{
		DraggingBallReset();
	}
}

