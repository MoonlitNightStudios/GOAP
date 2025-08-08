// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "GOAPGoal.h"

#include "GOAPAgentComponent.h"

AActor* FGOAPSelfAgentPawnValue::GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const
{
	return Agent ? Agent->GetPawn() : nullptr;
}

FGOAPGoal& FGOAPGoal::SetValue(const FInstancedStruct& Value)
{
	TargetStateValue = Value;
	return *this;
}

void FGOAPGoal::AddContext(const FGOAPContext& Context)
{
	Contexts.Add(Context);
}

float FGOAPLocationGoal::GetPriority(const UGOAPAgentComponent* Agent) const
{
	if (FVector::Distance(Agent->GetPawn()->GetActorLocation(), GetTargetLocation(Agent)) > AcceptanceRadius)
	{
		return 1.0f;
	}
	return 0.f;
}

bool FGOAPLocationGoal::CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal, float& ContributionPoints) const
{
	if (const FGOAPLocationGoal* LocationGoal = Goal.GetPtr<FGOAPLocationGoal>())
	{
		//Other goal is of same type

		//Negative because we want larger distances to mean less contribution
		ContributionPoints = -FVector::Distance(GetTargetLocation(Agent), LocationGoal->GetTargetLocation(Agent));
		return true;
	}
	
	return false;
}

FVector FGOAPLocationGoal::GetTargetLocation(const UGOAPAgentComponent* Agent) const
{
	if (TargetStateValue.IsValid())
	{
		if (const FGOAPVectorValue* Value = TargetStateValue.GetPtr<FGOAPVectorValue>())
		{
			return Value->GetValue(Agent, Contexts);
		}
		else if (const FGOAPActorValue* ActorValue = TargetStateValue.GetPtr<FGOAPActorValue>())
		{
			AActor* Actor = ActorValue->GetValue(Agent, Contexts);
			return Actor? Actor->GetActorLocation() : FVector::ZeroVector;
		}
	}
	
	return FVector::ZeroVector;
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
FString FGOAPLocationGoal::GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const
{
	if (TargetStateValue.IsValid())
	{
		if (const FGOAPVectorValue* Value = TargetStateValue.GetPtr<FGOAPVectorValue>())
		{
			return "LocationGoal - " + Value->GetValue(Agent, Contexts).ToString();
		}
		else if (const FGOAPActorValue* ActorValue = TargetStateValue.GetPtr<FGOAPActorValue>())
		{
			AActor* Actor = ActorValue->GetValue(Agent, Contexts);
			return "LocationGoal - " + (Actor? Actor->GetName() : "{red}Invalid Actor");
		}
	}

	return "LocationGoal - {red}Invalid Target Type";
}
#endif

float FGOAPFindPathGoal::GetPriority(const UGOAPAgentComponent* Agent) const
{
	return 0.01f;
}

bool FGOAPFindPathGoal::CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal,
	float& ContributionPoints) const
{
	if (const FGOAPFindPathGoal* PathGoal = Goal.GetPtr<FGOAPFindPathGoal>())
	{
		ContributionPoints = 1;
		return true;
	}
	
	return false;
}

FVector FGOAPFindPathGoal::GetTargetLocation(const UGOAPAgentComponent* Agent) const
{
	if (TargetStateValue.IsValid())
	{
		if (const FGOAPVectorValue* Value = TargetStateValue.GetPtr<FGOAPVectorValue>())
		{
			return Value->GetValue(Agent, Contexts);
		}
		else if (const FGOAPActorValue* ActorValue = TargetStateValue.GetPtr<FGOAPActorValue>())
		{
			AActor* Actor = ActorValue->GetValue(Agent, Contexts);
			return Actor? Actor->GetActorLocation() : FVector::ZeroVector;
		}
	}
	
	return FVector::ZeroVector;
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
FString FGOAPFindPathGoal::GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const
{
	if (TargetStateValue.IsValid())
	{
		if (const FGOAPVectorValue* Value = TargetStateValue.GetPtr<FGOAPVectorValue>())
		{
			return "FindPathGoal - " + Value->GetValue(Agent, Contexts).ToString();
		}
		else if (const FGOAPActorValue* ActorValue = TargetStateValue.GetPtr<FGOAPActorValue>())
		{
			AActor* Actor = ActorValue->GetValue(Agent, Contexts);
			return "FindPathGoal - " + (Actor? Actor->GetName() : "{red}Invalid Actor");
		}
	}

	return "FindPathGoal - {red}Invalid Target Type";
}
#endif
