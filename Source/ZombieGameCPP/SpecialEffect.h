// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpecialEffect.generated.h"

UCLASS()
class ZOMBIEGAMECPP_API ASpecialEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpecialEffect();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	class AMain* RefToMain;

	FORCEINLINE void SetMainReference(AMain* Main) { RefToMain = Main; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
