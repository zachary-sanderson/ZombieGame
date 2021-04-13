// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "AK47.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAMECPP_API AAK47 : public AWeapon
{
	GENERATED_BODY()
	
public:
	AAK47();

	void Shoot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
