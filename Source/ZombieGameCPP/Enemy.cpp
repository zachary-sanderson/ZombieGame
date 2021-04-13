// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "ZombieAIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalmeshComponent.h"
#include "Main.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimInstance.h"

#include "ZombieAIController.h"
/* AI Include */
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/NavMovementComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
AEnemy::AEnemy(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	/* Note: We assign the Controller class in the Blueprint extension of this class
		Because the zombie AIController is a blueprint in content and it's better to avoid content references in code.  */
		/*AIControllerClass = ASZombieAIController::StaticClass();*/

		/* Our sensing component to detect players by visibility and noise checks. */
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComp->SetPeripheralVisionAngle(60.0f);
	PawnSensingComp->SightRadius = 2000;
	PawnSensingComp->HearingThreshold = 600;
	PawnSensingComp->LOSHearingThreshold = 1200;

	/* These values are matched up to the CapsuleComponent above and are used to find navigation paths */
	GetMovementComponent()->NavAgentProps.AgentRadius = 42;
	GetMovementComponent()->NavAgentProps.AgentHeight = 192;

	AudioLoopComp = CreateDefaultSubobject<UAudioComponent>(TEXT("ZombieLoopedSoundComp"));
	AudioLoopComp->bAutoActivate = false;
	AudioLoopComp->bAutoDestroy = false;
	AudioLoopComp->SetupAttachment(RootComponent);

	SenseTimeOut = 5.f;

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("AttackSocket"));
	

	//Stats
	Health = 100.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	SetEnemySpeed(PatrolSpeed);

	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Normal);

	//Combat
	bOverlappingCombatSphere = false;
	bAttacking = false;

	AttackMinTime = 0.5f;
	AttackMaxTime = 1.f;

	DeathDelay = 3.f;

	bHasValidTarget = false;

	/* Note: Visual Setup is done in the AI/ZombieCharacter Blueprint file */
}


void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	/* This is the earliest moment we can bind our delegates to the component */
	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AEnemy::OnSeePlayer);
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AEnemy::OnHearNoise);
	}

	BroadcastUpdateAudioLoop(bSensedTarget);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatCollisionOnOverlapBegin);

	//FAttachmentTransformRules rules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
	//CombatCollision->AttachToComponent(GetMesh(), rules, FName("AttackSocket"));
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	AIController = Cast<AZombieAIController>(GetController());
	AIController->EnemyCharacter = this;
}


void AEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//UE_LOG(LogTemp, Warning, TEXT("bScreaming value is: %s"), bScreaming ? TEXT("true") : TEXT("false"))

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		bool bIsInAnimation = IsInAnimation();
		bool MontagePlaying = AnimInstance->IsAnyMontagePlaying();
		if (bIsInAnimation || MontagePlaying)
		{
			bUseControllerRotationYaw = false;
			AIController->StopMovement();
		}
		else
			bUseControllerRotationYaw = true;
	}

	if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking) return;

	/* Check if the last time we sensed a player is beyond the time out value to prevent bot from endlessly following a player. */
	if (bSensedTarget && (GetWorld()->TimeSeconds - LastSeenTime) > SenseTimeOut
		&& (GetWorld()->TimeSeconds - LastHeardTime) > SenseTimeOut)
	{
		//ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());
		if (AIController)
		{
			bSensedTarget = false;
			/* Reset */
			//AIController->SetTargetEnemy(nullptr);

			/* Stop playing the hunting sound */
			BroadcastUpdateAudioLoop(false);
		}
	}
}


void AEnemy::OnSeePlayer(APawn* Pawn)
{
	if (!IsAlive())
	{
		return;
	}

	if (!bSensedTarget)
	{
		BroadcastUpdateAudioLoop(true);
	}

	/* Keep track of the time the player was last sensed in order to clear the target */
	LastSeenTime = GetWorld()->GetTimeSeconds();
	bSensedTarget = true;
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);

	//ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());
	AMain* SensedPawn = Cast<AMain>(Pawn);
	if (AIController && SensedPawn)
	{
		AIController->SetTargetEnemy(SensedPawn);
		CombatTarget = SensedPawn;
	}
}


void AEnemy::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	UE_LOG(LogTemp, Warning, TEXT("Heard Noise"))

	if (!IsAlive() /*|| EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking*/ || EnemyMovementStatus == EEnemyMovementStatus::EMS_Downed)
	{
		return;
	}

	if (!bSensedTarget)
	{
		BroadcastUpdateAudioLoop(true);
	}

	bSensedTarget = true;
	LastHeardTime = GetWorld()->GetTimeSeconds();

	if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking)
	{
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Investigating);
		//UE_LOG(LogTemp, Warning, TEXT("Investigating"));
	}

	//ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());

	//AMain* SensedPawn = Cast<AMain>(PawnInstigator);'
	
	if (AIController)
	{
		/*TO IMPLEMENT: Set Target Noise location and move to there*/
		if (GetDistanceTo(PawnInstigator) < PawnSensingComp->HearingThreshold)
		{
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			AIController->SetTargetEnemy(PawnInstigator);
			UE_LOG(LogTemp, Warning, TEXT("Heard Player"))
		}
		else
		{
			AIController->SetHeardLocation(Location);
			UE_LOG(LogTemp, Warning, TEXT("Heard Location Set"))
		}
			
	}
}

void AEnemy::SetEnemyMovementStatus(EEnemyMovementStatus Status)
{
	EnemyMovementStatus = Status;

	if (Status == EEnemyMovementStatus::EMS_Attacking)
		GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;

	if (Status == EEnemyMovementStatus::EMS_Downed)
		GetCharacterMovement()->MaxWalkSpeed = DownedSpeed;

	//ASZombieAIController* AIController = Cast<ASZombieAIController>(GetController());
	if (AIController)
	{
		AIController->SetBlackboardEnemyStatus(Status);
	}

	BroadcastUpdateAudioLoop(bSensedTarget);
}


UAudioComponent* AEnemy::PlayCharacterSound(USoundCue* CueToPlay)
{
	if (CueToPlay)
	{
		return UGameplayStatics::SpawnSoundAttached(CueToPlay, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	}

	return nullptr;
}

void AEnemy::BroadcastUpdateAudioLoop_Implementation(bool bNewSensedTarget)
{
	/* Start playing the hunting sound and the "noticed player" sound if the state is about to change */
	if (bNewSensedTarget && EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
	{
		PlayCharacterSound(SoundPlayerNoticed);

		AudioLoopComp->SetSound(SoundHunting);
		AudioLoopComp->Play();
	}
	else
	{
		if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Normal)
		{
			AudioLoopComp->SetSound(SoundWandering);
			AudioLoopComp->Play();
		}
		else
		{
			AudioLoopComp->SetSound(SoundIdle);
			AudioLoopComp->Play();
		}
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//When in attacking range begin attacking, provided timer allows it
void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Alive())
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			bHasValidTarget = true;

			bOverlappingCombatSphere = true;

			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		}
	}
}

//Reset timer and begin moving towards target
void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			bHasValidTarget = false;
			bOverlappingCombatSphere = false;
			/*
			if (!IsInAnimation() && Alive())
			{
				MoveToTarget(Main);
			}
			*/
			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

/*
void AEnemy::MoveToTarget(class AMain* Target)
{
	if (AIController)
	{
		AIController->MoveToActor(Target, 50.f);
	}
}
*/

//When left weapon connects deals damage and displays a particle effect at the location
void AEnemy::CombatCollisionOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main)
		{
			if (Main->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (HitParticles)
			{
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("AttackSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SocketLocation, FRotator(0.f), true);
				}
			}
			if (DamageTypeClass)
			{
				Main->ReceiveDamage(Damage);
			}
		}
	}
}

void AEnemy::ActivateCombatCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCombatCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//Play weapon swing sound
void AEnemy::PlayAttackSound()
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySound2D(this, AttackSound);
	}
}

/*
	**Stop moving and attack!**
*/
void AEnemy::Attack()
{
	if (Alive() && bHasValidTarget && EnemyMovementStatus != EEnemyMovementStatus::EMS_Downed && !IsInAnimation())
	{
		if (CombatMontage && CombatMontage2)
		{
			if (AIController)
			{
				AIController->StopMovement();
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			}
			else
				return;

			if (!bAttacking)
			{
				bAttacking = true;
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					FMath::RandBool() ? AnimInstance->Montage_Play(CombatMontage, 1.f) : AnimInstance->Montage_Play(CombatMontage2, 1.f);
				}
			}
		}
	}
}


void AEnemy::ReceiveDamage(float DamageAmount, FName BoneName, FVector FwdVec, AMain* Attacker)
{
	if (Attacker)
	{
		if (AIController)
		{
			bSensedTarget = true;
			AIController->StopMovement();
			AIController->SetTargetEnemy(Attacker);
		}
	}
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
		AnimInstance->StopAllMontages(0.1f);

	bScreaming = false;
	bAttacking = false;

	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.0f;
		Die(BoneName, FwdVec);
		return;
	}

	bTakingDamage = true;
	if (Health < MaxHealth / 4 && LegBoneNames.Contains(BoneName))
	{
		bIsDowned = true;
		SetEnemySpeed(DownedSpeed);
	}
	else if (BoneName == FName("Spine2"))
	{
		bHeadshot = true;
	}
	else { Health -= DamageAmount; }
}

//Death and cleanup
void AEnemy::Die(FName BoneName, FVector FwdVec)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	AIController->StopMovement();

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetCollisionProfileName(FName("ZombieRagdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->AddForce(FwdVec*30000.f, BoneName);

	if (AudioLoopComp)
		AudioLoopComp->Stop();

	AIController->UnPossess();


	bAttacking = false;
}

bool AEnemy::Alive()
{
	return EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::FinishedAttacking()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
	/*
	else
		MoveToTarget(CombatTarget);
	*/
}

void AEnemy::FinishedScreaming()
{
	bScreaming = false;
	if (bOverlappingCombatSphere) Attack();
}

void AEnemy::Recover()
{
	bAttacking = false;
	bTakingDamage = false;
	bHeadshot = false;
	if (bOverlappingCombatSphere) Attack();

	//else if (CombatTarget) MoveToTarget(CombatTarget);
}

bool AEnemy::IsInAnimation()
{
	bool IsInAnimation =  bAttacking || bScreaming || bTakingDamage || bHeadshot;
	if (AIController)
	{
		AIController->SetIsInAnimation(IsInAnimation);
	}
	return IsInAnimation;
}