// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};
UCLASS()
class ZOMBIEGAMECPP_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	// Follow Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class UPawnNoiseEmitterComponent* NoiseEmitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<AWeapon*> CurrentWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class AActor> Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class AActor> Shotgun;

	int32 WeaponIndex;

	virtual void Jump() override;

	// Base turn rate to scale turning function for camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnRate;

	// Base lookup rate to scale lookup function for camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/**
	/*
	/* Player Stats
	/*
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float SprintingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float CrouchingSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	bool bCrouching;

	void CrouchDown();
	void StopCrouching();

	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bSprinting;
	float SprintDrainRate;

	void SprintOn();
	void SprintOff();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bIsInBulletTime;
	float BulletTimeDrainRate;

	void EnterBulletTime();
	void LeaveBulletTime();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bShooting;

	void Shoot();
	void StopShooting();

	UFUNCTION(BlueprintCallable)
	void StillShooting();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Booleans")
	bool bZoomed;

	void ZoomIn();
	void ZoomOut();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bChangedWeapon;

	UFUNCTION(BlueprintCallable)
	void SwapWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bReloading;

	void Reload();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
		TSubclassOf<class AActor> PredictPathFX;

	TArray<AActor*> GrenadeTrail;

	bool bCanShoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bTakingDamage;

	float ForwardMovement;
	float RightMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AActor> Grenade;

	class AThrowableItem* CurrentGrenade;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Booleans")
	bool bPreppingGrenade;
	bool bPredictingGrenade;
	bool bThrowingGrenade;

	void PrepGrenade();
	UFUNCTION(BlueprintCallable)
	void SpawnGrenade();
	UFUNCTION(BlueprintCallable)
	void PredictGrenadePath();
	UFUNCTION(BlueprintCallable)
	void ThrowGrenade();
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	UFUNCTION(BlueprintCallable)
	void FinishedThrowingGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called for fwd and bwd
	void MoveForward(float Value);

	// Called for yaw rotation
	void MoveRight(float Value);

	void Turn(float Value);

	// Called for pitch rotation
	void LookUp(float Value);

	bool CanShoot();

	float MoveForwardSpeed;
	float MoveRightSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	float InterpSpeed;

	/** Called via input to turn at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to lookup at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired lookup rate
	*/
	void LookUpAtRate(float Rate);

	FRotator GetLookAtRotationYaw(FVector Target);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void RestoreHealth(float Amount);

	void ReceiveDamage(float Amount);

	bool bInvulnerable;

	void NotInvulnerable();

	FTimerHandle InvulnerabilityTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
	float InvulnerabilityDelay;

	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* RifleHipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* RifleIronsightsMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* ReloadRifleMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* ShotgunHipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* ShotgunIronsightsMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* ReloadShotgunMontage;
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* SwapWeaponMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		UAnimMontage* GrenadeMontage;

	void PlaySwapWeaponAnimation();

	UFUNCTION(BlueprintCallable)
	void FinishedSwappingWeapon();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* HitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* DeathMontage;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	UFUNCTION(BlueprintCallable)
	void FinishedReloading();

	void Equip();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equip")
	AWeapon* ActiveOverlappingWeapon;

	FORCEINLINE void SetActiveOverlappingItem(AWeapon* OverlappingWeapon) { ActiveOverlappingWeapon = OverlappingWeapon; }
	FORCEINLINE AWeapon* GetActiveOverlappingItem() { return ActiveOverlappingWeapon; }

	// AI
	void MakeNoiseAsInstigator(float Loudness, FVector Location);

	void MakeNoiseAsNullInstigator(float Loudness, FVector Location);
};
