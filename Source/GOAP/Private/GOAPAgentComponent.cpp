// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "GOAPAgentComponent.h"

#include "GOAPAction.h"
#include "GOAPSensor.h"
#include "GOAPSubsystem.h"
#include "LogGOAP.h"
#include "Behaviours/GOAPAgentBehaviour.h"
#include "StructUtils/InstancedStruct.h"

FGOAPPlan::FGOAPPlan()
{
}

bool FGOAPPlan::GeneratePlan(UGOAPAgentComponent* AgentComponent, const UGOAPSubsystem* SS,
                             const FInstancedStruct& ForGoal)
{
	OwningAgent = AgentComponent;
	Subsystem = SS;
	Goal = &ForGoal;
	RootAction = FGOAPActionNode();
	CurrentAction = nullptr;
	
	if (FindBestActionForGoal(AgentComponent, ForGoal, SS->GetGlobalActions(), RootAction))
	{
		return true;
	}
	else
	{
		StopPlan(AgentComponent);
		return false;
	}
}

void FGOAPPlan::OnActionCompleted(const UGOAPAction* Action, const FInstancedStruct& ForGoal)
{
	if (!CurrentAction)
	{
		UE_LOG(LogGOAP, Error, TEXT("OnActionCompleted called but Current action in NULL"));
		return;
	}
	
	if (Action == CurrentAction->Action)
	{
		if (CurrentAction->Action)
		{
			CurrentAction->Action->StopAction(OwningAgent, CurrentAction->Goal);
			CurrentAction->Status = Completed;
		}

		ExecutePlan(OwningAgent);
	}
	else
	{
		UE_LOG(LogGOAP, Error, TEXT("OnActionCompleted called on GOAPAgent. Completed action is %s but the current action is %s."), *(Action? Action->GetName():FString("Invalid")), *(CurrentAction->Action? CurrentAction->Action->GetName():FString("Invalid")));
	}
}

void FGOAPPlan::OnActionFailed(const UGOAPAction* Action, const FInstancedStruct& ForGoal)
{
	if (!CurrentAction)
	{
		UE_LOG(LogGOAP, Error, TEXT("OnActionFailed called but Current action in NULL"));
		return;
	}
	
	if (CurrentAction->Action == Action)
	{
		CurrentAction->FailedActions.Add(Action);
		CurrentAction->Action->StopAction(OwningAgent, CurrentAction->Goal);

		UGOAPSubsystem* SS = OwningAgent->GetWorld()->GetSubsystem<UGOAPSubsystem>();
		if (FindBestActionForGoal(OwningAgent, CurrentAction->Goal, SS->GetGlobalActions(), *CurrentAction))
		{
			ExecutePlan(OwningAgent);
		}
		else
		{
			//All requirements failed. Cannot proceed with current action.
			if (CurrentAction->ParentNode)
			{
				//Let parent know it has failed so we can choose a different action.
				CurrentAction->ParentNode->FailedActions.Add(CurrentAction->ParentNode->Action);
				CurrentAction = CurrentAction->ParentNode;
				OnActionFailed(CurrentAction->Action, CurrentAction->Goal);
			}
			else
			{
				//No parent. This is the root. Come up with a fresh plan.
				//TODO:: there is no guarantee we would find a new plan. Should we still generate a new plan blindly?
				StopPlan(OwningAgent);
			}
		}
	}
	else
	{
		UE_LOG(LogGOAP, Error, TEXT("OnActionFailed called on GOAPAgent. Failed action is %s but the current action is %s."), *(Action? Action->GetName():FString("Invalid")), *(CurrentAction->Action? CurrentAction->Action->GetName():FString("Invalid")));
	}
}

void FGOAPPlan::StopPlan(UGOAPAgentComponent* Agent)
{
	if (CurrentAction && CurrentAction->Action)
	{
		CurrentAction->Action->StopAction(Agent, CurrentAction->Goal);
	}

	Goal = nullptr;
	CurrentAction = nullptr;
	RootAction = FGOAPActionNode();
}

void FGOAPPlan::UpdatePlan(float DeltaTime, UGOAPAgentComponent* Agent)
{
	if (CurrentAction && CurrentAction->Action)
	{
		CurrentAction->Action->UpdateAction(DeltaTime, Agent, CurrentAction->Goal);
	}
}

void FGOAPPlan::ExecutePlan(UGOAPAgentComponent* Agent)
{
	//Find next current action
	if (!CurrentAction)
	{
		//if no current action is set, find a leaf node and make it the new current action
		CurrentAction = &GetLeafNode(RootAction);
	}

	//We have a current action. Get its parent as the next current action. If the parent has other children though, their leaf node should be the next action
	//If it is not completed, it is still the current action
	if (CurrentAction->Status != Completed)
	{
		CurrentAction = &GetLeafNode(*CurrentAction);
	}
	else if (CurrentAction->ParentNode)
	{
		CurrentAction = &GetLeafNode(*CurrentAction->ParentNode);
	}
	else
	{
		//No parent. This is the root node. The plan is completed.
		const FInstancedStruct* CompletedGoal = Goal;
		StopPlan(Agent);
		Agent->OnPlanCompleted(CompletedGoal);		
		return;
	}

	if (CurrentAction && CurrentAction->Action)
	{		
		CurrentAction->Action->StartAction(Agent, CurrentAction->Goal);
	}
	else
	{
		StopPlan(Agent);
	}
}

bool FGOAPPlan::FindBestActionForGoal(UGOAPAgentComponent* Agent, const FInstancedStruct& InGoal, const TArray<const UGOAPAction*>& AvailableActions,
                                      FGOAPActionNode& Node)
{
	Node.Children.Empty();

	if (!InGoal.IsValid())
	{
		UE_LOG(LogGOAP, Display, TEXT("Invalid goal given for agent %s."), *(Agent ? Agent->GetPawn()->GetName() : FString("Invalid Pawn")));
		return false;
	}
	
	float HighestContribution = -INFINITY;
	const UGOAPAction* HighestContributionAction = nullptr;
	for (const UGOAPAction* Action : AvailableActions)
	{
		if (HasActionFailedForGoal(Action, Node))
		{
			continue;
		}
		
		TArray<FInstancedStruct> Effects = Action->GetEffects(Agent, InGoal);
		float Contribution = 0.f;
		bool bSatisfiesGoal = false;
		for (const FInstancedStruct& EffectStruct : Effects)
		{
			if (const FGOAPGoal* EffectGoal = EffectStruct.GetPtr<FGOAPGoal>())
			{
				if (EffectGoal->CanContributeToGoal(Agent, InGoal, Contribution))
				{
					bSatisfiesGoal = true;
					break;
				}
			}
		}

		if (!bSatisfiesGoal)
		{
			continue;
		}

		if (Contribution > HighestContribution)
		{
			HighestContribution = Contribution;
			HighestContributionAction = Action;
		}
	}

	if (HighestContributionAction)
	{
		Node.Action = HighestContributionAction;
		Node.Goal = InGoal;
		bool bAllActionsSatisfied = true;
		for (const FInstancedStruct& GoalStruct : HighestContributionAction->GetRequirements(Agent, InGoal))
		{
			if (const FGOAPGoal* GoalReq = GoalStruct.GetPtr<FGOAPGoal>())
			{
				//if priority is 0, this requirement is already satisfied. No need to plan for this
				if (GoalReq->GetPriority(Agent))
				{
					FGOAPActionNode& NewNode = Node.Children.AddDefaulted_GetRef();
					NewNode.ParentNode = &Node;
					bAllActionsSatisfied &= FindBestActionForGoal(Agent, GoalStruct, AvailableActions, NewNode);
				}
			}
		}
		return bAllActionsSatisfied;
	}
	else
	{
		//If not possible actions are available, create a new plan from scratch
		UE_LOG(LogGOAP, Display, TEXT("No available actions for goal %s for agent %s."), *InGoal.GetScriptStruct()->GetName(), *Agent->GetPawn()->GetName());
		return false;
	}
}

bool FGOAPPlan::HasActionFailedForGoal(const UGOAPAction* Action, FGOAPActionNode& Node) const
{
	for (const UGOAPAction* FailedAction : Node.FailedActions)
	{
		if (Action == FailedAction)
		{
			return true;
		}
	}
	return false;
}

FGOAPActionNode& FGOAPPlan::GetLeafNode(FGOAPActionNode& Root) const
{
	for (FGOAPActionNode& Child : Root.Children)
	{
		if (Child.Status < EGOAPActionNodeStatus::Completed)
		{
			return GetLeafNode(Child);
		}
	}

	return Root;
}

// Sets default values for this component's properties
UGOAPAgentComponent::UGOAPAgentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UGOAPAgentComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	MakePlan(GetWorld()->GetSubsystem<UGOAPSubsystem>());
	CurrentPlan.UpdatePlan(DeltaTime, this);
	
	TransientStateData.Empty();
}

APawn* UGOAPAgentComponent::GetPawn() const
{
	if (APawn* Owner = Cast<APawn>(GetOwner()))
	{
		return Owner;
	}

	if (AController* Controller = Cast<AController>(GetOwner()))
	{
		return Controller->GetPawn();
	}

	return nullptr;
}

AController* UGOAPAgentComponent::GetController() const
{
	if (AController* Controller = Cast<AController>(GetOwner()))
	{
		return Controller;
	}

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return Pawn->GetController();
	}

	return nullptr;
}


void UGOAPAgentComponent::MakePlan(const UGOAPSubsystem* SS)
{
	for (const UGOAPSensor* Sensor : SS->GetGlobalSensors())
	{
		Sensor->Sense(this);
	}
	
	if (const FInstancedStruct* Goal = GetBestGoal())
	{
		//Nothing to change here. Just pursue current goal
		if (Goal == CurrentPlan.GetCurrentGoal()) return;

		StopCurrentPlan();
		if (CurrentPlan.GeneratePlan(this, SS, *Goal))
		{
			ExecutePlan();
		}
		else
		{
			OnPlanFailed(Goal);
		}		
	}
}

void UGOAPAgentComponent::OnWorldStateChanged(const UGOAPSubsystem* SS)
{
	FailedGoals.Empty();
}

void UGOAPAgentComponent::OnAgentStateChanged()
{
	FailedGoals.Empty();
}

const FInstancedStruct* UGOAPAgentComponent::GetBestGoal() const
{
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	GoalPickerDebugData.Empty();
#endif
	
	if (InterruptGoal.IsValid())
	{
		return &InterruptGoal;
	}
	
	float HighestPriority = -1.f;
	const FInstancedStruct* HighestPriorityGoal = nullptr; 
	for (const FInstancedStruct& GoalStruct : Goals)
	{
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
		FGOAPGoalPickerDebugDataRow& Row = GoalPickerDebugData.AddDefaulted_GetRef();
		Row.GoalType = GoalStruct.GetScriptStruct();
#endif
		if (const FGOAPGoal* GoalPtr = GoalStruct.GetPtr<FGOAPGoal>())
		{
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
			Row.Goal = GoalPtr;
#endif
			if (FailedGoals.Contains(GoalPtr))
			{
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
				Row.bFailed = true;
				Row.TotalPriority = GoalPtr->GetPriority(this) * GoalPtr->GetWeight();
#endif
				continue;
			}
			
			float Priority = GoalPtr->GetPriority(this) * GoalPtr->GetWeight();
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
			Row.TotalPriority = Priority;
#endif
			if (Priority > HighestPriority)
			{
				HighestPriority = Priority;
				HighestPriorityGoal = &GoalStruct;
			}
		}
	}

	return HighestPriorityGoal;
}

void UGOAPAgentComponent::StopCurrentPlan()
{
	CurrentPlan.StopPlan(this);
}

void UGOAPAgentComponent::ExecutePlan()
{
	CurrentPlan.ExecutePlan(this);
}

void UGOAPAgentComponent::OnPlanCompleted(const FInstancedStruct* CompletedGoal)
{
	if (CompletedGoal == &InterruptGoal)
	{
		OnInterruptGoalFinished.Broadcast(this, true, InterruptGoal, Interrupter.Get());
		InterruptGoal = FInstancedStruct();
		Interrupter = nullptr;
	}
	//MakePlan(GetWorld()->GetSubsystem<UGOAPSubsystem>());
}

void UGOAPAgentComponent::OnPlanFailed(const FInstancedStruct* FailedGoal)
{
	if (FailedGoal == &InterruptGoal)
	{
		OnInterruptGoalFinished.Broadcast(this, false, InterruptGoal, Interrupter.Get());
		InterruptGoal = FInstancedStruct();
		Interrupter = nullptr;
	}
	else
	{
		FailedGoals.Add(FailedGoal->GetPtr<FGOAPGoal>());
	}
}

void UGOAPAgentComponent::OnActionCompleted(const UGOAPAction* Action, const FInstancedStruct& ForGoal)
{
	CurrentPlan.OnActionCompleted(Action, ForGoal);
}

void UGOAPAgentComponent::OnActionFailed(const UGOAPAction* Action, const FInstancedStruct& ForGoal)
{
	CurrentPlan.OnActionFailed(Action, ForGoal);
}

void UGOAPAgentComponent::SetInterruptGoal(const FInstancedStruct& Goal, const UObject* InterruptRequester)
{
	InterruptGoal = Goal;
	Interrupter = InterruptRequester;
	StopCurrentPlan();
	UGOAPSubsystem* SS = GetWorld()->GetSubsystem<UGOAPSubsystem>();
	MakePlan(SS);
}

void UGOAPAgentComponent::AddStateData(FGameplayTag StateTag, const FInstancedStruct& Effect, bool bIsPermanent)
{
	TMap<FGameplayTag, FInstancedStruct>& StateData = bIsPermanent ? PermanentStateData : TransientStateData;

	StateData.Add(StateTag, Effect);

	OnAgentStateChanged();
}

void UGOAPAgentComponent::RemoveStateData(FGameplayTag StateTag)
{
	PermanentStateData.Remove(StateTag);
}

bool UGOAPAgentComponent::HasStateData(FGameplayTag StateTag) const
{
	return TransientStateData.Contains(StateTag) || PermanentStateData.Contains(StateTag);
}

void UGOAPAgentComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	for (UGOAPAgentBehaviour* Behaviour : AgentBehaviours)
	{
		Behaviour->ActivateBehaviour(this);
	}
}

void UGOAPAgentComponent::Deactivate()
{
	Super::Deactivate();

	for (UGOAPAgentBehaviour* Behaviour : AgentBehaviours)
	{
		Behaviour->DeactivateBehaviour();
	}
	
	StopCurrentPlan();
}

// Called when the game starts
void UGOAPAgentComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetOwner()->GetWorld())
	{
		if (UGOAPSubsystem* Subsystem = World->GetSubsystem<UGOAPSubsystem>())
		{
			Subsystem->RegisterAgent(this);
		}
	}
	
}

void UGOAPAgentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	StopCurrentPlan();
	if (UWorld* World = GetOwner()->GetWorld())
	{
		if (UGOAPSubsystem* Subsystem = World->GetSubsystem<UGOAPSubsystem>())
		{
			Subsystem->UnregisterAgent(this);
		}
	}
}
