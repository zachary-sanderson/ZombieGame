// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FIndPatrolLocation.h"
#include "ZombieAIController.h"

/* AI Module includes */
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
/* This contains includes all key types like UBlackboardKeyType_Vector used below. */
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "NavigationSystem.h"



EBTNodeResult::Type UBTTask_FIndPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AZombieAIController* MyController = Cast<AZombieAIController>(OwnerComp.GetAIOwner());
	if (MyController == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	//AActor* MyWaypoint = MyController->GetWaypoint();
	//if (MyWaypoint)
	/* Find a position that is close to the waypoint. We add a small random to this position to give build predictable patrol patterns  */
	const float SearchRadius = 1000.0f;
	//const FVector SearchOrigin = MyWaypoint->GetActorLocation();
	const FVector SearchOrigin = MyController->GetPawn()->GetActorLocation();
	FNavLocation ResultLocation;
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(MyController);
	if (NavSystem)
	{
		if (NavSystem->GetRandomPointInNavigableRadius(SearchOrigin, SearchRadius, ResultLocation))
		{
			/* The selected key should be "PatrolLocation" in the BehaviorTree setup */
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), ResultLocation.Location);
			return EBTNodeResult::Succeeded;
		}
	}
	

	return EBTNodeResult::Failed;
}

