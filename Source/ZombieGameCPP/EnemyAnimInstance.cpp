// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn);
		}
	}
}

void UEnemyAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn);
		}
	}
	//UPDATE VELOCITY/MOVE SPEED
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();
	}
	//UPDATE STATUS EFFECTS
	if (Enemy)
	{
			bScreaming = Enemy->bScreaming;
			bRandomChoice = Enemy->bRandomChoice;
			bAttacking = Enemy->bAttacking;
			bDamaged = Enemy->bTakingDamage;
			bHeadshot = Enemy->bHeadshot;
			bDowned = Enemy->bIsDowned;
	}
}