// Fill out your copyright notice in the Description page of Project Settings.

#include "AK47.h"
#include "Kismet/GameplayStatics.h"
#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"


AAK47::AAK47()
{
}

// Called when the game starts or when spawned
void AAK47::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AAK47::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/* Implement After I create attenutation and concurrency classes */
	//UGameplayStatics::PlaySoundAtLocation(FireSound, GetWorld(), 1.f, 1.f, 0.f, MuzzleLocation, FRotator(0.f), 1f, 1f);
void AAK47::Shoot()
{
	FVector StartPoint = RefToMain->CameraBoom->GetComponentLocation();
	FRotator CameraRotation = RefToMain->GetBaseAimRotation();
	FVector FwdVec = RefToMain->CameraBoom->GetForwardVector();
	FVector EndPoint = StartPoint + (FwdVec * 10000);
	FVector MuzzleLocation = SkeletalMesh->GetSocketLocation(FName("Muzzle"));
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleLocation, FRotator(0.f), true);

	UGameplayStatics::PlaySound2D(this, FireSound);

	FHitResult OutHit;

	FCollisionQueryParams CollisionParams;

	//DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor.IsValid())
			{
				//Cast to Enemy or whatever
			}
			//if (Enemy)
			UWorld* World = GetWorld();
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AActor* Actor = World->SpawnActor<AActor>(BloodSFX, OutHit.Location, FRotator(0.f), SpawnParams);
			//Enemy.TakeDamage(Damage, OutHit.BoneName, FwdVec)
			//else
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, OutHit.Location, FRotator(0.f), true);
			UGameplayStatics::PlaySound2D(this, ImpactSound);
		}
	}
	CurrentClipAmmo--;
}