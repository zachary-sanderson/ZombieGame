// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Main.h"
#include "Enemy.h"


// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SetRootComponent(SkeletalMesh);

	NumProjectilesPerShot = 10;

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionVolume"));
	CollisionVolume->SetupAttachment(GetRootComponent());

	bRotate = false;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnOverlapBegin);
	CollisionVolume->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnOverlapEnd);

	CurrentClipAmmo = ClipSize;
	CurrentTotalAmmo = MaxAmmo - ClipSize;
}

void AWeapon::PlayReloadAnimation(bool bZoomed)
{
	UAnimInstance* AnimInstance = RefToMain->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (bZoomed)
		{
			if (ReloadIronsightsMontage)
				AnimInstance->Montage_Play(ReloadIronsightsMontage, 1.f);
			return;
		}
		
		if (ReloadHipMontage)
				AnimInstance->Montage_Play(ReloadHipMontage, 1.f);
	}
}

void AWeapon::Reload()
{
	if (CurrentTotalAmmo > ClipSize) {
		CurrentTotalAmmo -= (ClipSize - CurrentClipAmmo);
		CurrentClipAmmo = ClipSize;
	}
	else
	{
		CurrentClipAmmo = CurrentTotalAmmo;
		CurrentTotalAmmo = 0;
	}
}


void AWeapon::PlayShootAnimation(bool bZoomed)
{
	if (RefToMain && CanShoot())
	{
		UAnimInstance* AnimInstance = RefToMain->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			if (!AnimInstance->IsAnyMontagePlaying())
			{
				if (bZoomed)
				{
					if (ShootIronsightsMontage && WeaponType == EWeapon::EW_Pistol)
						AnimInstance->Montage_Play(ShootIronsightsMontage, 0.2f);
					else if (ShootHipMontage)
						AnimInstance->Montage_Play(ShootIronsightsMontage, 1.f);
					return;
				}

				if (ShootHipMontage && WeaponType == EWeapon::EW_Pistol)
					AnimInstance->Montage_Play(ShootHipMontage, 0.2f);
				else if (ShootHipMontage)
					AnimInstance->Montage_Play(ShootHipMontage, 1.f);
			}
		}
	}
}

bool AWeapon::CanShoot()
{
	return CurrentClipAmmo > 0;
}

void AWeapon::Shoot()
{
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(MyShake, 1.0f);
	switch (WeaponType)
	{
	case EWeapon::EW_Rifle:
		ShootSingle();
		break;
	case EWeapon::EW_Shotgun:
		ShootSpread();
		break;
	case EWeapon::EW_Pistol:
		ShootSingle();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Shoot() function doesn't recognise weapon type"));
	}
}

/* Implement After I create attenutation and concurrency classes */
	//UGameplayStatics::PlaySoundAtLocation(FireSound, GetWorld(), 1.f, 1.f, 0.f, MuzzleLocation, FRotator(0.f), 1f, 1f);
void AWeapon::ShootSingle()
{
	FVector StartPoint = RefToMain->CameraBoom->GetComponentLocation();
	FVector FwdVec = RefToMain->FollowCamera->GetForwardVector();
	FVector EndPoint = StartPoint + (FwdVec * 10000);
	FVector MuzzleLocation = SkeletalMesh->GetSocketLocation(FName("Muzzle"));
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleLocation, RefToMain->GetActorRotation(), FVector(0.1f));

	//UGameplayStatics::PlaySound2D(this, FireSound);
	UGameplayStatics::SpawnSoundAtLocation(this, FireSound, MuzzleLocation, FRotator::ZeroRotator, 1.f, 1.f, 0.0f, nullptr, nullptr, true);
	//RefToMain->MakeNoiseAsInstigator(1.f, MuzzleLocation);

	FCollisionQueryParams CollisionParams;
	//CollisionParams.bDebugQuery = true;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.AddIgnoredActor(RefToMain);

	//DrawDebugLine(GetWorld(), MuzzleLocation, EndPoint, FColor::Green, false, 1, 0, 1);

	FHitResult OutHit;

	if (GetWorld()->LineTraceSingleByChannel(OutHit, MuzzleLocation, EndPoint, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor.IsValid())
			{
				AEnemy* Enemy = Cast<AEnemy>(OutHit.Actor);

				UWorld* World = GetWorld();
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				if (Enemy)
				{
					FTransform Transform;
					Transform.SetLocation(OutHit.Location);
					Transform.SetRotation(FQuat(0.f, 0.f, 0.f, 0.f));
					Transform.SetScale3D(FVector(10.f));
					if (BloodSFX)
						AActor* Actor = World->SpawnActor<AActor>(BloodSFX, Transform, SpawnParams);
					Enemy->ReceiveDamage(Damage, OutHit.BoneName, FwdVec, RefToMain);
				}
				else
				{
					/*
						if (WeaponImpactField)
							AActor* Actor = World->SpawnActor<AActor>(WeaponImpactField, OutHit.Location, FRotator(0.f), SpawnParams);
						*/
					if (ImpactParticles)
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, OutHit.Location, FRotator(0.f), true);
					if (ImpactSound)
					{
						UGameplayStatics::SpawnSoundAtLocation(this, ImpactSound, OutHit.Location, FRotator::ZeroRotator, 1.f, 1.f, 0.0f, nullptr, nullptr, true);
						RefToMain->MakeNoiseAsNullInstigator(1.f, OutHit.Location);
					}
				}
			}
		}
	}
	CurrentClipAmmo--;
}

/* Implement After I create attenutation and concurrency classes */
//UGameplayStatics::PlaySoundAtLocation(FireSound, GetWorld(), 1.f, 1.f, 0.f, MuzzleLocation, FRotator(0.f), 1f, 1f);
void AWeapon::ShootSpread()
{
	FVector StartPoint = RefToMain->CameraBoom->GetComponentLocation();
	FRotator CameraRotation = RefToMain->GetBaseAimRotation();
	FVector FwdVec = RefToMain->FollowCamera->GetForwardVector();
	FVector RightVec = RefToMain->CameraBoom->GetRightVector();
	FVector UpVec = RefToMain->CameraBoom->GetUpVector();
	FVector MuzzleLocation = SkeletalMesh->GetSocketLocation(FName("Muzzle"));
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleLocation, RefToMain->GetActorRotation(), FVector(0.25f));

	UGameplayStatics::SpawnSoundAtLocation(this, FireSound, MuzzleLocation, FRotator::ZeroRotator, 1.f, 1.f, 0.0f, nullptr, nullptr, true);
	//RefToMain->MakeNoiseAsInstigator(1.f, MuzzleLocation);

	for (int i = 0; i < NumProjectilesPerShot; i++)
	{
		FVector EndPoint = MuzzleLocation + (FwdVec * 10000);

		EndPoint += (RightVec * FMath::RandRange(-250.f, 250.f)) + (UpVec * FMath::RandRange(-250.f, 250.f));

		FHitResult OutHit;

		FCollisionQueryParams CollisionParams;

		//DrawDebugLine(GetWorld(), MuzzleLocation, EndPoint, FColor::Green, false, 1, 0, 1);

		if (GetWorld()->LineTraceSingleByChannel(OutHit, MuzzleLocation, EndPoint, ECC_Visibility, CollisionParams))
		{
			if (OutHit.bBlockingHit)
			{
				if (OutHit.Actor.IsValid())
				{
					UWorld* World = GetWorld();
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
					AEnemy* Enemy = Cast<AEnemy>(OutHit.Actor);
					if (Enemy)
					{
						FTransform Transform;
						Transform.SetLocation(OutHit.Location);
						Transform.SetRotation(FQuat(0.f, 0.f, 0.f, 0.f));
						Transform.SetScale3D(FVector(5.f));
						AActor* Actor = World->SpawnActor<AActor>(BloodSFX, Transform, SpawnParams);
						//AActor* Actor = World->SpawnActor<AActor>(BloodSFX, OutHit.Location, FRotator(0.f), SpawnParams);
						Enemy->ReceiveDamage(Damage, OutHit.BoneName, FwdVec, RefToMain);
					}
					else
					{
						/*
						if (WeaponImpactField)
							AActor* Actor = World->SpawnActor<AActor>(WeaponImpactField, OutHit.Location, FRotator(0.f), SpawnParams);
						*/
						if (ImpactParticles)
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, OutHit.Location, FRotator(0.f), true);
						if (ImpactSound)
						{
							UGameplayStatics::SpawnSoundAtLocation(this, ImpactSound, OutHit.Location, FRotator::ZeroRotator, 1.f, 1.f, 0.0f, nullptr, nullptr, true);
							RefToMain->MakeNoiseAsNullInstigator(1.f, OutHit.Location);
						}
					}
				}
			}
		}
	}
	CurrentClipAmmo--;
}

void AWeapon::SetMainReference(AMain* Main)
{
	RefToMain = Main;
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRotate)
	{
		FRotator Rotation = GetActorRotation();
		Rotation.Yaw += DeltaTime * RotationRate;
		SetActorRotation(Rotation);
	}
}


void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (WeaponState == EWeaponState::EWS_Pickup && OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SetActiveOverlappingItem(this);
		}
	}
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			Main->SetActiveOverlappingItem(nullptr);
		}
	}
}
