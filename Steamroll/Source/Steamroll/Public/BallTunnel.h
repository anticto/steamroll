// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "BallTunnel.generated.h"

/**
 * Tunnel that lets steamballs travel nearly instantly from one point of the level to another
 */
UCLASS()
class STEAMROLL_API ABallTunnel : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tunnel)
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tunnel)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Tunnel)
	class USphereComponent* TriggerVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tunnel)
	ABallTunnel* ConnectedTunnel;

	/** True if the player has already sent a steamball through this tunnel, which means trajectory prediction will go through the tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tunnel)
	bool bDiscovered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tunnel)
	float TransportDelaySeconds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tunnel)
	float SpeedMultiplier;

	UPROPERTY(BlueprintReadWrite, Category = Tunnel)
	class ASteamrollBall* IncomingBall;

	UFUNCTION(BlueprintCallable, Category = Tunnel)
	void TransportToOtherTunnelEnd(AActor* OtherActor);

	/** Timer to control the tunnel time. */
	FTimerHandle TunnelTimer;

};
