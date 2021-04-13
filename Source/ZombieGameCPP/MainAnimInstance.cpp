// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Main.h"
#include "Weapon.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Main = Cast<AMain>(Pawn);
		}
	}
}

void UMainAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		//Set the MovementSpeed of the player when not locked on
		bIsInAir = Pawn->GetMovementComponent()->IsFalling();

		if (Main == nullptr)
		{
			Main = Cast<AMain>(Pawn);
		}

		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();


		//Set the movement speed of the player when locked on as well as animation related booleans for locomotion

		if (Main)
		{
			TurnValue = Main->TurnRate;
			MovementSpeedForward = Main->MoveForwardSpeed;
			MovementSpeedRight = Main->MoveRightSpeed;
			bZoomed = Main->bZoomed;
			bCrouching = Main->bCrouching;
			bSprinting = Main->bSprinting;

			if (Main->bChangedWeapon || Main->bReloading || Main->bPreppingGrenade)
				bUseIK = false;
			else if (Main->EquippedWeapon)
			{
				HandLocation = Main->EquippedWeapon->SkeletalMesh->GetSocketLocation(FName("Hand_IK"));
				bUseIK = true;
			}
			bIsInAction = !(Main->CanShoot());
		}
	}
}
