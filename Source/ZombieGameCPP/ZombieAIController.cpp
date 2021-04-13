// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAIController.h"
#include "NavigationSystem.h"
#include "Enemy.h"
#include "Main.h"
#include "Tasks/AITask_MoveTo.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

/* AI Specific includes */
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


AZombieAIController::AZombieAIController()
{
	// Initialise Pawn senses
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensingComponent->SetPeripheralVisionAngle(90.f);

	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	/* Match with the AI/ZombieBlackboard */
	PatrolLocationKeyName = "PatrolLocation";
	HeardLocationKeyName = "HeardLocation";
	EnemyStatusKeyName = "EnemyStatus";
	TargetEnemyKeyName = "TargetEnemy";
	IsInAnimationKeyName = "IsInAnimation";

	/* Initializes PlayerState so we can assign a team index to AI */
	bWantsPlayerState = true;
}


void AZombieAIController::OnPossess(class APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AEnemy* ZombieBot = Cast<AEnemy>(InPawn);
	if (ZombieBot)
	{
		if (ensure(ZombieBot->BehaviorTree->BlackboardAsset))
		{
			BlackboardComp->InitializeBlackboard(*ZombieBot->BehaviorTree->BlackboardAsset);
		}

		BehaviorComp->StartTree(*ZombieBot->BehaviorTree);

		/* Make sure the Blackboard has the type of bot we possessed */
		SetBlackboardEnemyStatus(ZombieBot->EnemyMovementStatus);
	}
}


void AZombieAIController::OnUnPossess()
{
	Super::OnUnPossess();

	/* Stop any behavior running as we no longer have a pawn to control */
	BehaviorComp->StopTree();
}

void AZombieAIController::SetTargetEnemy(APawn* NewTarget)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(TargetEnemyKeyName, NewTarget);
	}
}

AMain* AZombieAIController::GetTargetEnemy() const
{
	if (BlackboardComp)
	{
		return Cast<AMain>(BlackboardComp->GetValueAsObject(TargetEnemyKeyName));
	}

	return nullptr;
}

void AZombieAIController::SetHeardLocation(FVector Location)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(HeardLocationKeyName, Location);
		//UE_LOG(LogTemp, Warning, TEXT("bScreaming value is: %s"), bScreaming ? TEXT("true") : TEXT("false"))
		UE_LOG(LogTemp, Warning, TEXT("Heard Location : X = %f, Y = %f, Z = %f"), Location.X, Location.Y, Location.Z)
	}
}


void AZombieAIController::SetBlackboardEnemyStatus(EEnemyMovementStatus NewType)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(EnemyStatusKeyName, (uint8)NewType);
	}
}

void AZombieAIController::SetIsInAnimation(bool IsInAnimation)
{
	if (BlackboardComp)
	{

		BlackboardComp->SetValueAsBool(IsInAnimationKeyName, IsInAnimation);
	}
}






/**
 * Called every frame.
 *
 * @param DeltaTime The time between the previous Tick and the current one.
 */
void AZombieAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

/**
 * Returns a zero rotator if GetPawn is null and the Actor rotation yaw
 * otherwise.
 */
FRotator AZombieAIController::GetControlRotation() const
{
	if (GetPawn() == nullptr) return FRotator(0.0f, 0.0f, 0.0f);

	return FRotator(0.0f, GetPawn()->GetActorRotation().Yaw, 0.0f);
}
/**
 * When a `MoveToLocation` or `MoveToActor` action has completed this method is fired
 * and we either move to another Waypoint or we chase the PlayerCharacter.
 *
 * @param RequestID The Move Request ID for the move that was completed.
 * @param Result
 */
void AZombieAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.IsSuccess())
	{
		if (EnemyCharacter)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Move Complete has Enemy Character"))
				if (EnemyCharacter->EnemyMovementStatus == EEnemyMovementStatus::EMS_Normal || EnemyCharacter->EnemyMovementStatus == EEnemyMovementStatus::EMS_Investigating)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Attempting to Scream"))
					EnemyCharacter->bScreaming = true;
					EnemyCharacter->bRandomChoice = FMath::RandBool();
					// The PatrolCharacter is in Patrol mode so we wait at the current Waypoint
					// for 5 seconds and then we call `MoveToWaypoint` to move to the next Waypoint.
					//GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AZombieAIController::Patrol, PatrolDelay);
				}
		}
	}
}
