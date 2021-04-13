// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Weapon.h"
#include "ThrowableItem.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "MainPlayerController.h"


// Sets default values
AMain::AMain()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//AI
	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Noise Emitter"));

	// Create Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), FName("Camera"));
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->bUsePawnControlRotation = true;

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(34.f, 88.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = true;

	// Set turn/lookup rate
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't Rotate when the controller rotates
	// Let that just effect the camera
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Stats
	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 150.f;
	Stamina = 150.f;

	RunningSpeed = 250.f;
	SprintingSpeed = 550.f;
	CrouchingSpeed = 125.f;
	GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;

	MoveForwardSpeed = 0.f;
	MoveRightSpeed = 0.f;

	ForwardMovement = 0.f;
	RightMovement = 0.f;

	bPreppingGrenade = false;
	bPredictingGrenade = false;
	bThrowingGrenade = false;

	//Input booleans
	bCrouching = false;
	bSprinting = false;
	bShooting = false;
	bZoomed = false;
	bChangedWeapon = false;
	bReloading = false;
	bCanShoot = false;
	bTakingDamage = false;
	bIsInBulletTime = false;

	SprintDrainRate = 10.f;
	BulletTimeDrainRate = 20.f;

	//Combat
	InterpSpeed = 15.f;
	WeaponIndex = 0;

	//Damage Invulnerability Variables
	bInvulnerable = false;
	InvulnerabilityDelay = 0.2f;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	MovementStatus = EMovementStatus::EMS_Normal;

	FAttachmentTransformRules camerarules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
	CameraBoom->AttachToComponent(GetMesh(), camerarules, FName("Camera"));

	if (Rifle)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* RifleActor = GetWorld()->SpawnActor<AActor>(Rifle, FVector(0.f), FRotator(0.f), SpawnParams);
		AWeapon* RifleWeapon = Cast<AWeapon>(RifleActor);
		RifleWeapon->SetMainReference(this);
		FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
		RifleWeapon->AttachToComponent(GetMesh(), rules, FName("Weapon"));
		RifleWeapon->WeaponState = EWeaponState::EWS_Equipped;
		EquippedWeapon = RifleWeapon;
		CurrentWeapons.Empty();
		CurrentWeapons.Add(RifleWeapon);
	}
	/*
	if (CurrentWeapons.Num() > 0)
		EquippedWeapon = CurrentWeapons[0];
		*/
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Stamina Drain/Recovery On Tick, Cancel Spritning and Bullet Time if out of stamina
	float DeltaStamina = 0.f;
	if (bSprinting && bIsInBulletTime)
		DeltaStamina -= (SprintDrainRate + BulletTimeDrainRate);
	else if (bIsInBulletTime)
		DeltaStamina -= BulletTimeDrainRate;
	else if (bSprinting)
		DeltaStamina -= SprintDrainRate;
	else
		DeltaStamina = 10.f;
	DeltaStamina *= DeltaTime;

	if ((Stamina + DeltaStamina) > MaxStamina)
		Stamina = MaxStamina;
	else if ((Stamina + DeltaStamina) < 0.f)
	{
		Stamina = 0.f;
		SprintOff();
		LeaveBulletTime();
	}
	else
		Stamina += DeltaStamina;



	FRotator LookAtYaw = GetLookAtRotationYaw(FollowCamera->GetForwardVector()*10000.f);
	FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
	//SetActorRotation(InterpRotation, ETeleportType::TeleportPhysics);

	

	if (bPredictingGrenade && CurrentGrenade)
	{
		GrenadeTrail.Empty();
		UE_LOG(LogTemp, Warning, TEXT("Predicting Grendade Path"))
		FPredictProjectilePathParams Params;
		Params.SimFrequency = 10.f;
		Params.MaxSimTime = 1.f;
		Params.DrawDebugType = EDrawDebugTrace::None;
		Params.ActorsToIgnore = TArray<AActor*>{ this, CurrentGrenade };
		Params.StartLocation = CurrentGrenade->GetActorLocation();
		FVector FwdVec = FollowCamera->GetForwardVector();
		Params.LaunchVelocity = FwdVec * 1000.f;
		FPredictProjectilePathResult PathResult;
		UGameplayStatics::PredictProjectilePath(this, Params, PathResult);
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		for (int i = 0; i < PathResult.PathData.Num() - 1; i++)
		{
			if (i > 10) continue;
			FVector CurrentPointLocation = PathResult.PathData[i].Location;
			FVector NextPointLocation = PathResult.PathData[i + 1].Location;
			FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentPointLocation, NextPointLocation);
			if (PredictPathFX)
			{
				UE_LOG(LogTemp, Warning, TEXT("Spawning Grenade Trail"))
				AActor* Actor = World->SpawnActor<AActor>(PredictPathFX, CurrentPointLocation, Rotation, SpawnParams);
				GrenadeTrail.Add(Actor);
				//UNiagaraComponent* effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PredictPathFX, CurrentPointLocation, Rotation);
			}
		}
	}
}



FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::SprintOn);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::SprintOff);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMain::Shoot);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AMain::StopShooting);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AMain::ZoomIn);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &AMain::ZoomOut);

	PlayerInputComponent->BindAction("BulletTime", IE_Pressed, this, &AMain::EnterBulletTime);
	PlayerInputComponent->BindAction("BulletTime", IE_Released, this, &AMain::LeaveBulletTime);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMain::CrouchDown);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMain::Reload);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMain::Equip);

	PlayerInputComponent->BindAction("Grenade", IE_Pressed, this, &AMain::PrepGrenade);
	PlayerInputComponent->BindAction("Grenade", IE_Released, this, &AMain::ThrowGrenade);

	PlayerInputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AMain::PlaySwapWeaponAnimation);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
}

//Check booleans to see if a player is Dead or locked in animation and should not be able to move.
bool AMain::CanShoot()
{
	if (EquippedWeapon) {
		return !bSprinting && !bChangedWeapon && !bPreppingGrenade &&
			!bReloading && !GetMovementComponent()->IsFalling() && !bTakingDamage &&
			MovementStatus != EMovementStatus::EMS_Dead;
	}
	else
		return false;
}

void AMain::Turn(float Value)
{
	if (Value > 1.f)
		TurnRate = 1.f;
	else if (Value < -1.f)
		TurnRate = -1.f;
	else if (0.1f > Value && Value > -0.1f)
		TurnRate = 0;
	else
		TurnRate = Value;
	AddControllerYawInput(Value);
}

void AMain::LookUp(float Value)
{
	if (0.1f > Value && Value > -0.1f)
		Value = 0;
	AddControllerPitchInput(Value);
}

// Called for forwards movement
void AMain::MoveForward(float Value)
{
	MoveForwardSpeed = Value;
	if (bSprinting)
		MoveForwardSpeed = 1.f;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, MoveForwardSpeed);
}

// Called for sideways movement
void AMain::MoveRight(float Value)
{
	MoveRightSpeed = Value;
	if (bSprinting)
	{
		MoveRightSpeed = 0.f;
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, MoveRightSpeed);
}

void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::CrouchDown()
{
	if (bCrouching)
		StopCrouching();
	else
	{
		bCrouching = true;
		Super::Crouch();
		GetCharacterMovement()->MaxWalkSpeed = CrouchingSpeed;
	}
}

void AMain::StopCrouching()
{
	bCrouching = false;
	Super::UnCrouch();
	if (bSprinting)
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	else
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
}

void AMain::EnterBulletTime()
{
	bIsInBulletTime = true;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.5f);
}

void AMain::LeaveBulletTime()
{
	bIsInBulletTime = false;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
}

//Calls Attack() provided the player can attack
void AMain::Shoot()
{
	if (EquippedWeapon && CanShoot() && !bShooting)
	{
		bShooting = true;
		EquippedWeapon->PlayShootAnimation(bZoomed);
	}
}

void AMain::StopShooting()
{
	bShooting = false;
}

void AMain::StillShooting()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	if (!(EquippedWeapon->CanShoot() && CanShoot() && bShooting))
	{
		bShooting = false;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
			AnimInstance->Montage_JumpToSection(FName("End"));
	}
}

void AMain::MakeNoiseAsInstigator(float Loudness, FVector Location)
{
	UE_LOG(LogTemp, Warning, TEXT("Making noise Main"))
		//UE_LOG(LogTemp, Warning, TEXT("Noise Location : X = %f, Y = %f, Z = %f"), Location.X, Location.Y, Location.Z)
	NoiseEmitter->MakeNoise(this, Loudness, Location);
}


void AMain::MakeNoiseAsNullInstigator(float Loudness, FVector Location)
{
	UE_LOG(LogTemp, Warning, TEXT("Making noise nullptr"))
		//UE_LOG(LogTemp, Warning, TEXT("Noise Location : X = %f, Y = %f, Z = %f"), Location.X, Location.Y, Location.Z)
	NoiseEmitter->MakeNoise(this, Loudness, Location);
}

void AMain::Reload()
{
	if (EquippedWeapon && CanShoot() && !bShooting)
	{
		bReloading = true;
		EquippedWeapon->PlayReloadAnimation(bZoomed);
	}
}

void AMain::FinishedReloading()
{
	bReloading = false;
}

void AMain::Equip()
{
	if (ActiveOverlappingWeapon)
	{
		AWeapon* NewWeapon = ActiveOverlappingWeapon;
		NewWeapon->bRotate = false;
		NewWeapon->SetActorLocationAndRotation(FVector(0.f), FRotator(0.f));
		NewWeapon->SetMainReference(this);
		FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
		NewWeapon->AttachToComponent(GetMesh(), rules, FName("Weapon"));
		NewWeapon->SetActorHiddenInGame(true);
		NewWeapon->WeaponState = EWeaponState::EWS_Equipped;
		CurrentWeapons.Add(NewWeapon);
	}
}

void AMain::ZoomIn()
{
	//bZoomed = true;
}

void AMain::ZoomOut()
{
	//bZoomed = false;
}

void AMain::RestoreHealth(float Amount)
{
	Health = (Health + Amount >= MaxHealth) ? MaxHealth : Health + Amount;
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}

	MovementStatus = EMovementStatus::EMS_Dead;
}

void AMain::Jump()
{
	if (GetMovementComponent()->IsFalling()) return;
	//if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		UnCrouch();
		Super::Jump();
	}
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}


void AMain::SprintOn()
{
	if (CanShoot() && !bCrouching)
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	bSprinting = true;
}
void AMain::SprintOff()
{
	bSprinting = false;
	if (bCrouching)
		GetCharacterMovement()->MaxWalkSpeed = CrouchingSpeed;
	else
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
}

void AMain::PlaySwapWeaponAnimation()
{
	if (!CanShoot() || bShooting) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponMontage)
	{
		bChangedWeapon = true;
		AnimInstance->Montage_Play(SwapWeaponMontage, 1.f);
	}
}

//Cycles through different weapon types
void AMain::SwapWeapon()
{
	if (EquippedWeapon)
	{
		WeaponIndex = WeaponIndex >= CurrentWeapons.Num() - 1 ? 0 : WeaponIndex + 1;
		
		if (CurrentWeapons[WeaponIndex])
		{ 
			EquippedWeapon->SetActorHiddenInGame(true);
			EquippedWeapon = CurrentWeapons[WeaponIndex];
			EquippedWeapon->SetActorHiddenInGame(false);
		}
		else
			WeaponIndex -= 1;
	}
}

void AMain::FinishedSwappingWeapon()
{
	bChangedWeapon = false;
	if (bSprinting)
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
}

void AMain::ReceiveDamage(float Amount)
{
	if (!bInvulnerable)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && HitMontage)
		{
			AnimInstance->Montage_Play(HitMontage, 1.0f);
		}
		if (Health - Amount <= 0.f)
		{
			Health -= Amount;
			Die();
		}
		else
		{
			Health -= Amount;
			bInvulnerable = true;
			GetWorldTimerManager().SetTimer(InvulnerabilityTimer, this, &AMain::NotInvulnerable, InvulnerabilityDelay);
		}
	}
}

void AMain::NotInvulnerable()
{
	bInvulnerable = false;
	GetWorldTimerManager().ClearTimer(InvulnerabilityTimer);
}


void AMain::PrepGrenade()
{
	if (CanShoot() && !bShooting)
	{
		bPreppingGrenade = true;
		FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
		EquippedWeapon->AttachToComponent(GetMesh(), rules, FName("WeaponRight"));
		
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && GrenadeMontage)
			AnimInstance->Montage_Play(GrenadeMontage, 1.0f);
	}
}

void AMain::PredictGrenadePath()
{
	if (CurrentGrenade)
	{
		UE_LOG(LogTemp, Warning, TEXT("Predicting path, pulling pin"))
		CurrentGrenade->PinPulled();
		bPredictingGrenade = true;
	}
}

void AMain::ThrowGrenade()
{
	if (bPreppingGrenade)
	{
		bThrowingGrenade = true;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && GrenadeMontage)
			AnimInstance->Montage_SetNextSection(FName("Hold"), FName("Throw"));
	}
}

void AMain::SpawnGrenade()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AActor* Actor = GetWorld()->SpawnActor<AActor>(Grenade, FVector(0.f), FRotator(0.f), SpawnParams);

	CurrentGrenade = Cast<AThrowableItem>(Actor);
	if (CurrentGrenade)
	{
		UE_LOG(LogTemp, Warning, TEXT("Has current grenade"))
		CurrentGrenade->SetRefToMain(this);
		FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
		CurrentGrenade->AttachToComponent(GetMesh(), rules, FName("Grenade"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("No current grenade"))
}

void AMain::LaunchGrenade()
{
	if (CurrentGrenade)
	{
		CurrentGrenade->ThrownMesh->SetCollisionProfileName(FName("PhysicsActor"));
		CurrentGrenade->ThrownMesh->SetSimulatePhysics(true);
		FVector FwdVec = FollowCamera->GetForwardVector();
		CurrentGrenade->ThrownMesh->SetPhysicsLinearVelocity(FwdVec*1000.f);
	}
	bPredictingGrenade = false;
}

void AMain::FinishedThrowingGrenade()
{
	CurrentGrenade = nullptr;
	FAttachmentTransformRules rules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
	EquippedWeapon->AttachToComponent(GetMesh(), rules, FName("Weapon"));
	bPreppingGrenade = false;
	bThrowingGrenade = false;
	if (bSprinting)
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
}
