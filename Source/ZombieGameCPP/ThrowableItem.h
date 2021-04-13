// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableItem.generated.h"

UCLASS()
class ZOMBIEGAMECPP_API AThrowableItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AThrowableItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	class UStaticMeshComponent* StartingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* ThrownMesh;

	FTimerHandle EffectBeginTimer;
	float EffectDelayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<class ASpecialEffect> Effect;

	void PinPulled();
	void SpawnEffect();

	class AMain* RefToMain;

	FORCEINLINE void SetRefToMain(AMain* Main) { RefToMain = Main; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
