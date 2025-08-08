// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "Actions/GOAPAction_MoveToLocation.h"

#include "GOAPAgentComponent.h"
#include "AIController.h"
#include "GOAPTags.h"
#include "LogGOAP.h"
#include "NavigationSystem.h"
#include "Chaos/Deformable/MuscleActivationConstraints.h"
#include "Goals/GOAPSensorGoal.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "Navigation/PathFollowingComponent.h"

TArray<FInstancedStruct> UGOAPAction_MoveToLocation::GetRequirements(const UGOAPAgentComponent* Agent,
	const FInstancedStruct& Goal) const
{
	TArray<FInstancedStruct> RetVal;

	if (const FGOAPLocationGoal* GoalPtr = Goal.GetPtr<FGOAPLocationGoal>())
	{
		bool bCanReachLocation = true;
		if (FMobileAgentData* Data = MobileAgents.Find(Agent))
		{
			for (FInstancedStruct& FailedLoc : Data->FailedToReachLocations)
			{
				if (const FGOAPVectorValue* Value = FailedLoc.GetPtr<FGOAPVectorValue>())
				{
					if (Value->GetValue(Agent, GoalPtr->GetContexts()) == GoalPtr->GetTargetLocation(Agent))
					{
						bCanReachLocation = false;
						break;
					}
				}
				if (const FGOAPActorValue* Value = FailedLoc.GetPtr<FGOAPActorValue>())
				{
					if (AActor* Actor = Value->GetValue(Agent, GoalPtr->GetContexts()))
					{
						if (Actor->GetActorLocation() == GoalPtr->GetTargetLocation(Agent))
						{
							bCanReachLocation = false;
							break;
						}
					}
				}
			}
		}

		if (!bCanReachLocation)
		{
			if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
			{
				APawn* Pawn = Agent->GetPawn();
				ANavigationData* NavData = NavSys->GetNavDataForProps(Pawn->GetNavAgentPropertiesRef(), Pawn->GetNavAgentLocation());
				FPathFindingQuery Query = FPathFindingQuery(Agent->GetOwner(), *NavData,
					Pawn->GetActorLocation(), GoalPtr->GetTargetLocation(Agent),
					UNavigationQueryFilter::GetQueryFilter(*NavData, Agent->GetOwner(), nullptr));
				Query.SetAllowPartialPaths(false);
				Query.SetRequireNavigableEndLocation(true);
				if (!NavSys->TestPathSync(Query))
				{
					//Can't reach location. Add a goal to find path
					FGOAPFindPathGoal Req;
					Req.SetValue(GoalPtr->GetTargetStateValue());
					RetVal.Add(FInstancedStruct::Make(Req));
				}
			}
			else
			{
				//Assume we can't reach location. Add a goal to find path
				FGOAPFindPathGoal Req;
				Req.SetValue(GoalPtr->GetTargetStateValue());
				RetVal.Add(FInstancedStruct::Make(Req));
			}
		}
	}
	
	return RetVal;
}

TArray<FInstancedStruct> UGOAPAction_MoveToLocation::GetEffects(const UGOAPAgentComponent* Agent,
                                                                const FInstancedStruct& Goal) const
{
	TArray<FInstancedStruct> RetVal;
	
	if (const FGOAPLocationGoal* GoalPtr = Goal.GetPtr<FGOAPLocationGoal>())
	{
		//Actor goes to location
		FGOAPLocationGoal Effect;
		Effect.SetValue(GoalPtr->GetTargetStateValue());	//Promise we can get to the location
		RetVal.Add(FInstancedStruct::Make(Effect));
	}
	
	return RetVal;
}

void UGOAPAction_MoveToLocation::CloseKnowledgeBank(UGOAPSubsystem* SS) const
{
	MobileAgents.Empty();
}

void UGOAPAction_MoveToLocation::StartAction(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const
{
	if (const FGOAPLocationGoal* LocationGoal = Goal.GetPtr<FGOAPLocationGoal>())
	{
		AAIController* AIController = Cast<AAIController>(Agent->GetPawn()->GetController());
		AIController->ReceiveMoveCompleted.AddUniqueDynamic(this, &UGOAPAction_MoveToLocation::OnMoveCompleted);     
		FMobileAgentData& Data = MobileAgents.FindOrAdd(Agent);
		EPathFollowingRequestResult::Type Result = EPathFollowingRequestResult::Failed;
		const FInstancedStruct& TargetStateValue = LocationGoal->GetTargetStateValue();
		if (TargetStateValue.IsValid())
		{
			if (const FGOAPVectorValue* Value = TargetStateValue.GetPtr<FGOAPVectorValue>())
			{
				Result = AIController->MoveToLocation(Value->GetValue(Agent, LocationGoal->GetContexts()), 10.f/*, FMath::Max(0,LocationGoal->AcceptanceRadius)*/);
			}
			else if (const FGOAPActorValue* ActorValue = TargetStateValue.GetPtr<FGOAPActorValue>())
			{
				if (AActor* GoalActor = ActorValue->GetValue(Agent, LocationGoal->GetContexts()))
				{
					Result = AIController->MoveToActor(GoalActor, 10.f/*, FMath::Max(0,LocationGoal->AcceptanceRadius)*/);
				}
			}
		}
		switch (Result)
		{
		case EPathFollowingRequestResult::AlreadyAtGoal:
			UE_LOG(LogGOAP, Display, TEXT("Agent %s: Move started. Already at goal. Completing immediately."), *Agent->GetName());
			Agent->OnActionCompleted(this, Goal);
			return;
		case EPathFollowingRequestResult::RequestSuccessful:
			UE_LOG(LogGOAP, Display, TEXT("Agent %s: Move request successful."), *Agent->GetName());
			Data.RequestID = AIController->GetCurrentMoveRequestID();
			return;
		case EPathFollowingRequestResult::Failed:
			//Register location as unreachable by agent. We couldn't find a way to reach the goal
			UE_LOG(LogGOAP, Display, TEXT("Agent %s: Move request Failed."), *Agent->GetName());
			MobileAgents[Agent].FailedToReachLocations.Add(LocationGoal->GetTargetStateValue());
		default:break;
		}
	}

	//Let agent know we can't move there
	UE_LOG(LogGOAP, Display, TEXT("Agent %s: Failing action."), *Agent->GetName());
	Agent->OnActionFailed(this, Goal);
}

void UGOAPAction_MoveToLocation::StopAction(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const
{
	if (const FGOAPLocationGoal* LocationGoal = Goal.GetPtr<FGOAPLocationGoal>())
	{
		if (APawn* Pawn = Agent->GetPawn())
		{
			if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
			{
				AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UGOAPAction_MoveToLocation::OnMoveCompleted);
				AIController->StopMovement();
			}
		}
	}
}

void UGOAPAction_MoveToLocation::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	UGOAPAgentComponent* Agent = nullptr;
	for (TPair<UGOAPAgentComponent*, FMobileAgentData>& AgentData : MobileAgents)
	{
		if (AgentData.Value.RequestID ==  RequestID)
		{
			Agent = AgentData.Key;
			break;
		}
	}
	
	if (Agent)
	{
		UEnum* Enum = StaticEnum<EPathFollowingResult::Type>();
		UE_LOG(LogGOAP, Display, TEXT("Agent %s: Completed move. Result : %s"), *Agent->GetName(), *Enum->GetNameByValue(Result).ToString());
		switch (Result)
		{
			case EPathFollowingResult::Success:
				{
					if (const FInstancedStruct* GoalPtr = Agent->GetCurrentPlan().GetCurrentActionGoal())
					{
						if (const FGOAPLocationGoal* Goal = GoalPtr->GetPtr<FGOAPLocationGoal>())
						{
							if (FVector::Distance(Agent->GetPawn()->GetActorLocation(), Goal->GetTargetLocation(Agent)) <= Goal->AcceptanceRadius)
							{
								UE_LOG(LogGOAP, Display, TEXT("Agent %s: Move was successful. Completing action."), *Agent->GetName());
								Agent->OnActionCompleted(this, *GoalPtr);
								break;
							}
						}
					}
				}
			case EPathFollowingResult::Invalid:
			case EPathFollowingResult::OffPath:
				{
					UE_LOG(LogGOAP, Display, TEXT("Agent %s: Move Failed. Failing action."), *Agent->GetName());
					if (const FInstancedStruct* GoalPtr = Agent->GetCurrentPlan().GetCurrentActionGoal())
					{
						if (const FGOAPLocationGoal* Goal = GoalPtr->GetPtr<FGOAPLocationGoal>())
						{
							//Register location as unreachable by agent. We couldn't find a way to reach the goal
							MobileAgents[Agent].FailedToReachLocations.Add(Goal->GetTargetStateValue());
						}
					}
					Agent->OnActionFailed(this, FInstancedStruct());
					break;
				}
			default:															//handles Aborted and Blocked
				break;
		}
	}
}
