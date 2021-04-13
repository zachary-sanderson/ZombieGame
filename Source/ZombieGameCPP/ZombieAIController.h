// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Enemy.h"
#include "ZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIEGAMECPP_API AZombieAIController : public AAIController
{
	GENERATED_BODY()
public:
	// Sets default values for this AI Controller's properties.
	AZombieAIController();

	// Called when the AIController is taken over.
	virtual void OnPossess(APawn* Pawn) override;

	virtual void OnUnPossess() override;

	class UBehaviorTreeComponent* BehaviorComp;

	UBlackboardComponent* BlackboardComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName TargetEnemyKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName PatrolLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName HeardLocationKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName EnemyStatusKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
		FName IsInAnimationKeyName;

	AMain* GetTargetEnemy() const;

	void SetTargetEnemy(APawn* NewTarget);

	void SetHeardLocation(FVector Location);

	void SetBlackboardEnemyStatus(EEnemyMovementStatus NewType);

	void SetIsInAnimation(bool IsInAnimation);

	/** Returns BehaviorComp subobject **/
	FORCEINLINE UBehaviorTreeComponent* GetBehaviorComp() const { return BehaviorComp; }

	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	// Called to get the control's rotation.
	virtual FRotator GetControlRotation() const override;

	// Called when a move request has been completed. This can be
	// a move request to a Waypoint or to the PlayerCharacter.
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/*
	UFUNCTION()
	void Patrol();
	*/

	// Gets called when the perception component updates. This is
	// where check to see if the PlayerCharacter was detected.
	//UFUNCTION()
		//void OnPawnDetected(const TArray<AActor*>& DetectedPawns);

	// The TimerHandle used to delay the setting of the waypoint giving the
	// PatrolCharacter time to perform a looking animation.
	UPROPERTY()
		FTimerHandle TimerHandle;

	// The position to move to. This can be different than the PlayerCharacter's
	// position if a distraction was used.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FVector PositionToMoveTo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PatrolRadius = 1000.f;

	// The amount of time before Patrol() is called again after a move has been completed
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float PatrolDelay = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class AEnemy* EnemyCharacter;

	// A reference to the hearing perception component.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		class UAISenseConfig_Hearing* HearingConfig;

	// A reference to the sight perception component.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		class UAISenseConfig_Sight* SightConfig;

	// A reference to the sight perception component.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
	class UPawnSensingComponent* PawnSensingComponent;

	// The range at which the PatrolCharacter can hear the PlayerCharacter.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float AIHearingRange = 1000.0f;

	// The amount of time after the PlayerCharacter has been heard that the
	// PatrolCharacter will forget they heard the PlayerCharacter.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float AIHearingMaxAge = 5.0f;

	// Indicates whether the PatrolCharacter will be able to listen at the
	// start of the game or whether it will be enabled later manually.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		bool bAIHearingStartsEnabled = true;

	// Indicates whether the PatrolCharacter has seen the PlayerCharacter or not.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		bool bIsPlayerDetected = false;

	// Indicates whether the PatrolCharacter has heard but not seen the PlayerCharacter.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		bool bNeedsToInvestigate = false;

	// If the PatrolCharacter has seen the PlayerCharacter then this is populated with how
	// far away the PlayerCharacter is from the PatrolCharacter.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
		float DistanceToPlayer = 0.0f;
};
