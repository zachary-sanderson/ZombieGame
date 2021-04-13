// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAMECPP_API UMainAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
	void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeedForward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementSpeedRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float TurnValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bTurningLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bTurningRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bZoomed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bUseIK;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bIsInAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	FVector HandLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class APawn* Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class AMain* Main;
};
