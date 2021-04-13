// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAMECPP_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
		void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
		float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
		class APawn* Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
		class AEnemy* Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bScreaming;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bRandomChoice;

	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bDamaged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bHeadshot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Status)
		bool bDowned;
};
