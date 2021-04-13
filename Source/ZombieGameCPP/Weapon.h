// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"



UENUM(BlueprintType)
enum class EWeapon : uint8
{
	EW_Rifle UMETA(DisplayName = "Rifle"),
	EW_Shotgun UMETA(DisplayName = "Shotgun"),
	EW_Pistol UMETA(DisplayName = "Pistol"),

	EW_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Pickup UMETA(DisplayName = "Pickup"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class ZOMBIEGAMECPP_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	class AMain* RefToMain;

	// Base shape collision
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Collision")
		class USphereComponent* CollisionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
		bool bRotate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
		float RotationRate;

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	EWeapon WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int CurrentTotalAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	int MaxAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	int ClipSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int CurrentClipAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* ShootHipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* ShootIronsightsMontage;

	FVector AimPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
	TSubclassOf<class AActor> BloodSFX;

	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chaos")
	TSubclassOf<class AActor> WeaponImpactField;
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* ReloadHipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* ReloadIronsightsMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anims")
	TSubclassOf<UMatineeCameraShake> MyShake;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Reload();

	void PlayReloadAnimation(bool bZoomed);

	void PlayShootAnimation(bool bZoomed);

	bool CanShoot();

	UFUNCTION(BlueprintCallable)
	void Shoot();

	void ShootSingle();

	void ShootSpread();

	int NumProjectilesPerShot;

	void SetMainReference(AMain* Main);
};
