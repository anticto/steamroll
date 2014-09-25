

#pragma once

/**
 * 
 */
class DraggingBall
{
public:

	DraggingBall();

	virtual void DraggingBallTick(float DeltaSeconds, bool bTouchingFloor, float SpeedSquared);
	void DraggingBallSetSphere(class USphereComponent* Sphere);

	virtual bool IsActivated() = 0;
	virtual void DraggingBallActivate() = 0;
	virtual void DraggingBallStop() = 0;

protected:

	/** Speed considered to be stopped and deactivate sphysics simulation */
	float StoppingSpeed;
	float Age;
	bool Dragging;
	float StartDraggingTime;

	class USphereComponent* Sphere;

	void DraggingBallReset();
	
};

