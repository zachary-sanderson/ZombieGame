// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAMECPP_API AShotgun : public AWeapon
{
	GENERATED_BODY()
	
public:
	AShotgun();

	void Shoot();

	int NumProjectilesPerShot;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
