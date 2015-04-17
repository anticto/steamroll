// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ItemInventory.generated.h"


USTRUCT()
struct FCounterStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	TArray<class UChildActorComponent*> CounterArray;

	//Constructor
	FCounterStruct()
	{

	}
};


UCLASS()
class STEAMROLL_API AItemInventory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemInventory();

	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Inventory)
	int32 NumCounters;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Inventory)
	UStaticMeshComponent* Frame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	TSubclassOf<class AInvCounter> CounterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UChildActorComponent* Counter6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	TArray<UChildActorComponent*> Counters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UMaterialInterface* BallMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UMaterialInterface* WallMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UMaterialInterface* ExplosionMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	UMaterialInterface* RampMaterial;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	class AInvCounter* GetCounter(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void UpdateInventory(const TArray<struct FSlotContentConfigStruct>& SlotContent);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void UpdateNumBalls(int32 NumBalls);
};
