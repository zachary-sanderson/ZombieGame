// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EEnemyMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Investigating UMETA(DisplayName = "Investigating"),
	EMS_Attacking UMETA(DisplayName = "Attacking"),
	EMS_Downed UMETA(DisplayName = "Downed"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMax")
};
UCLASS()
class ZOMBIEGAMECPP_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus = EEnemyMovementStatus::EMS_Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
		class AZombieAIController* AIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float Damage = 10.f;

		FVector EnemyDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bTakingDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bIsDowned;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bScreaming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bRandomChoice;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bHeadshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		USoundCue* AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		TArray<FName> LegBoneNames;

	void SetEnemyMovementStatus(EEnemyMovementStatus Status);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DownedSpeed = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float PatrolSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float ChaseSpeed = 250.f;

	FORCEINLINE void SetEnemySpeed(float Speed) { GetCharacterMovement()->MaxWalkSpeed = Speed; }

	bool bHasValidTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		class USphereComponent* CombatCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		FName CombatCollisionSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UAnimMontage* CombatMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		class UAnimMontage* CombatMontage2;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float AttackMinTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float AttackMaxTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		TSubclassOf<UDamageType> DamageTypeClass;

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		float DeathDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		class AMain* CombatTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void CombatCollisionOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//UFUNCTION(BlueprintCallable)
	//void MoveToTarget(class AMain* Target);

	bool IsInAnimation();

	UFUNCTION(BlueprintCallable)
		void ActivateCombatCollision();

	UFUNCTION(BlueprintCallable)
		void DeactivateCombatCollision();

	UFUNCTION(BlueprintCallable)
		void PlayAttackSound();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
		void FinishedAttacking();

	UFUNCTION(BlueprintCallable)
		void FinishedScreaming();

	UFUNCTION(BlueprintCallable)
	void ReceiveDamage(float DamageAmount, FName BoneName, FVector FwdVec, AMain* Attacker = nullptr);

	UFUNCTION(BlueprintCallable)
	void Die(FName BoneName, FVector FwdVec);

	UFUNCTION(BlueprintCallable)
		bool Alive();

	UFUNCTION(BlueprintCallable)
		void Recover();


	FORCEINLINE bool IsAlive() {return EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead; }


	/* Last time the player was spotted */
	float LastSeenTime;

	/* Last time the player was heard */
	float LastHeardTime;

	/* Last time we attacked something */
	float LastMeleeAttackTime;

	/* Time-out value to clear the sensed position of the player. Should be higher than Sense interval in the PawnSense component not never miss sense ticks. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
		float SenseTimeOut;

	/* Resets after sense time-out to avoid unnecessary clearing of target each tick */
	bool bSensedTarget;

	UPROPERTY(VisibleAnywhere, Category = "AI")
		class UPawnSensingComponent* PawnSensingComp;

	/* The thinking part of the brain, steers our zombie and makes decisions based on the data we feed it from the Blackboard */
	/* Assigned at the Character level (instead of Controller) so we may use different zombie behaviors while re-using one controller. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
		class UBehaviorTree* BehaviorTree;

protected:

	/* Triggered by pawn sensing component when a pawn is spotted */
	/* When using functions as delegates they need to be marked with UFUNCTION(). We assign this function to FSeePawnDelegate */
	UFUNCTION()
		void OnSeePlayer(APawn* Pawn);

	UFUNCTION()
		void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);

	/* Update the vocal loop of the zombie (idle, wandering, hunting) */
	UFUNCTION(Reliable, NetMulticast)
		void BroadcastUpdateAudioLoop(bool bNewSensedTarget);

	void BroadcastUpdateAudioLoop_Implementation(bool bNewSensedTarget);

	UAudioComponent* PlayCharacterSound(USoundCue* CueToPlay);

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SoundPlayerNoticed;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SoundHunting;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SoundIdle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
		USoundCue* SoundWandering;

	/* Plays the idle, wandering or hunting sound */
	UPROPERTY(VisibleAnywhere, Category = "Sound")
		UAudioComponent* AudioLoopComp;
};
