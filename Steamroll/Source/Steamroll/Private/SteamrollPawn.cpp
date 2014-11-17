

#include "Steamroll.h"
#include "SteamrollPawn.h"
#include "SteamrollBall.h"
#include "SteamrollPlayerController.h"

#include "TimerManager.h"
#include "Engine.h"
#include "UnrealNetwork.h"


ASteamrollPawn::ASteamrollPawn(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	AimTransform = PCIP.CreateDefaultSubobject<USceneComponent>(this, TEXT("Tranform0"));
	Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("BaseCamera0"));
	Camera2 = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FreeCamera0"));

	RaiseCamera = false;
	LowerCamera = false;
	CameraSpeed = 300.f;
	MaxHeight = 500.f;
	MinHeight = 0.f;
	CurrCameraTime = 0.f;

	FiringTimeout = 6.f;
	MinLaunchSpeed = 50.f;
	MaxLaunchSpeed = 8000.f;
	bFiring = false;
	bFireSimulation = false;

	PrimaryActorTick.bCanEverTick = true;
}


void ASteamrollPawn::BeginPlay()
{
	Super::BeginPlay();

	InitCameraAnim();
}


void ASteamrollPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (RaiseCamera)
	{
		CurrCameraTime += DeltaSeconds;
		CurrCameraTime = FMath::Min(CurrCameraTime, CameraAnim->AnimLength);
	}

	if (LowerCamera)
	{
		CurrCameraTime -= DeltaSeconds;
		CurrCameraTime = FMath::Max(CurrCameraTime, 0.001f); // 0.001 not 0 becase going back to 0 would sometimes make te camera anim loop around to the end due to precision issues
	}

	SetCameraAnim(CurrCameraTime, DeltaSeconds); // Multiplied by AnimLength to map the 0-1 interval to the anim duration
}


void ASteamrollPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// set up gameplay key bindings
	InputComponent->BindAxis("MoveRight", this, &ASteamrollPawn::MoveRight);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ASteamrollPawn::FireCharge);
	InputComponent->BindAction("Fire", IE_Released, this, &ASteamrollPawn::FireRelease);

	InputComponent->BindAction("FireDebug_1", IE_Released, this, &ASteamrollPawn::FireDebug1);
	InputComponent->BindAction("FireDebug_2", IE_Released, this, &ASteamrollPawn::FireDebug2);
	InputComponent->BindAction("FireDebug_3", IE_Released, this, &ASteamrollPawn::FireDebug3);
	InputComponent->BindAction("FireDebug_4", IE_Released, this, &ASteamrollPawn::FireDebug4);
	InputComponent->BindAction("FireDebug_5", IE_Released, this, &ASteamrollPawn::FireDebug5);
	InputComponent->BindAction("FireDebug_6", IE_Released, this, &ASteamrollPawn::FireDebug6);
	InputComponent->BindAction("FireDebug_7", IE_Released, this, &ASteamrollPawn::FireDebug7);
	InputComponent->BindAction("FireDebug_8", IE_Released, this, &ASteamrollPawn::FireDebug8);
	InputComponent->BindAction("FireDebug_9", IE_Released, this, &ASteamrollPawn::FireDebug9);
	InputComponent->BindAction("FireDebug_10", IE_Released, this, &ASteamrollPawn::FireDebug10);

	InputComponent->BindAction("DestroyBalls", IE_Pressed, this, &ASteamrollPawn::DestroyBalls);
	InputComponent->BindAction("FireSimulation", IE_Pressed, this, &ASteamrollPawn::ToggleFireSimulation);

	InputComponent->BindAction("RaiseCamera", IE_Pressed, this, &ASteamrollPawn::RaiseCameraPressed);
	InputComponent->BindAction("RaiseCamera", IE_Released, this, &ASteamrollPawn::RaiseCameraReleased);
	InputComponent->BindAction("LowerCamera", IE_Pressed, this, &ASteamrollPawn::LowerCameraPressed);
	InputComponent->BindAction("LowerCamera", IE_Released, this, &ASteamrollPawn::LowerCameraReleased);
}


void ASteamrollPawn::MoveRight(float Val)
{
	if (Val != 0)
	{
		AimTransform->AddRelativeRotation(FRotator(0.f, Val, 0.f));

		RotationServer(AimTransform->RelativeRotation);
	}
}


void ASteamrollPawn::Fire(float ChargeTime)
{
	
}


void ASteamrollPawn::Explode()
{

}


void ASteamrollPawn::Debug(FString Msg)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, Msg);
	}
}


bool ASteamrollPawn::FireServer_Validate(float ChargeTime, FRotator Rotation)
{
	return ChargeTime >= 0.f && ChargeTime <= FiringTimeout + 0.1f;
}


void ASteamrollPawn::FireServer_Implementation(float ChargeTime, FRotator Rotation)
{
	AimTransform->SetRelativeRotation(Rotation);
	Fire(ChargeTime);
}


void ASteamrollPawn::ExplosionClient_Implementation()
{
	Explode();
}


bool ASteamrollPawn::RotationServer_Validate(FRotator Rotation)
{
	return true;
}


void ASteamrollPawn::RotationServer_Implementation(FRotator Rotation)
{
	AimTransform->SetRelativeRotation(Rotation);
	RotationClient(Rotation);
}


void ASteamrollPawn::RotationClient_Implementation(FRotator Rotation)
{
	AimTransform->SetRelativeRotation(Rotation);
}


float ASteamrollPawn::GetCharge()
{
	if (GetWorldTimerManager().IsTimerActive(this, &ASteamrollPawn::Timeout))
	{
		return GetWorldTimerManager().GetTimerElapsed(this, &ASteamrollPawn::Timeout);
	}

	return bFiring ? FiringTimeout : 0.f;
}


void ASteamrollPawn::SetCameraAnim(float Time, float DeltaSeconds)
{
	if (CameraAnim && CameraAnimInst)
	{
		float CurrentTime = CameraAnimInst->CurTime;
		float DeltaTime;

		if (Time >= CurrentTime)
		{
			DeltaTime = Time - CurrentTime;
		}
		else
		{
			float RemainingLength = CameraAnimInst->CamAnim->AnimLength - CurrentTime;
			DeltaTime = Time + RemainingLength;
		}

		CameraAnimInst->Update(1.f, 1.f, 0.f, 0.f, 0.f);
		CameraAnimInst->AdvanceAnim(DeltaTime, true);
		CameraAnimInst->Update(0.f, 1.f, 0.f, 0.f, 0.f);
	}
}


ASteamrollPlayerController* ASteamrollPawn::GetLocalPlayerController()
{
	UWorld* World = GetWorld();

	APlayerController* PlayerController = nullptr;

	if (GEngine && World)
	{
		PlayerController = GEngine->GetFirstLocalPlayerController(World);
	}

	return PlayerController ? Cast<ASteamrollPlayerController>(GEngine->GetFirstLocalPlayerController(World)) : nullptr;
}


void ASteamrollPawn::InitCameraAnim()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;

		if (CameraManager)
		{
			CameraAnimInst = CameraManager->PlayCameraAnim(CameraAnim, 0.f, 1.f, 0.f, 0.f, true);
		}
	}
}


void ASteamrollPawn::FireCharge()
{
	bFiring = true;
	GetWorldTimerManager().SetTimer(this, &ASteamrollPawn::Timeout, FiringTimeout, false);
}


void ASteamrollPawn::FireRelease()
{	
	if (bFiring)
	{
		// To prevent multiple firing when releasing the fire button check that we actually loaded/charged before releasing
		if (GetWorldTimerManager().IsTimerActive(this, &ASteamrollPawn::Timeout))
		{
			float TimeElapsed = GetWorldTimerManager().GetTimerElapsed(this, &ASteamrollPawn::Timeout);
			GetWorldTimerManager().PauseTimer(this, &ASteamrollPawn::Timeout);

			FireServer(TimeElapsed, AimTransform->RelativeRotation);

			GetWorldTimerManager().ClearTimer(this, &ASteamrollPawn::Timeout);
		}
		else
		{
			FireServer(FiringTimeout, AimTransform->RelativeRotation);
		}

		bFiring = false;
	}
}


void ASteamrollPawn::Timeout()
{
	//FireServer(FiringTimeout, AimTransform->RelativeRotation);
}


void ASteamrollPawn::FireDebug1() { FireServer(0.1f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug2() { FireServer(0.2f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug3() { FireServer(0.3f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug4() { FireServer(0.4f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug5() { FireServer(0.5f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug6() { FireServer(0.6f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug7() { FireServer(0.7f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug8() { FireServer(0.8f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug9() { FireServer(0.9f * FiringTimeout, AimTransform->RelativeRotation); }
void ASteamrollPawn::FireDebug10() { FireServer(1.0f * FiringTimeout, AimTransform->RelativeRotation); }


void ASteamrollPawn::DestroyBalls()
{
	for (TActorIterator<ASteamrollBall> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ActorItr->Destroy();
	}
}


void ASteamrollPawn::ToggleFireSimulation()
{
	bFireSimulation = !bFireSimulation;
}


void ASteamrollPawn::RaiseCameraPressed()
{
	RaiseCamera = true;
}


void ASteamrollPawn::RaiseCameraReleased()
{
	RaiseCamera = false;
}


void ASteamrollPawn::LowerCameraPressed()
{
	LowerCamera = true;
}


void ASteamrollPawn::LowerCameraReleased()
{
	LowerCamera = false;
}

