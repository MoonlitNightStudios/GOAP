// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "Goals/GOAPSensorGoal.h"

float FGOAPTagContainerGoal::GetPriority(const UGOAPAgentComponent* Agent) const
{
	const FGameplayTagContainer& Tags = GetTagContainer(Agent);

	if (!Tags.MatchesQuery(FGameplayTagQuery::MakeQuery_MatchAllTags(MustHaveTags)))
	{
		return 1;
	}

	if (!Tags.MatchesQuery(FGameplayTagQuery::MakeQuery_MatchNoTags(MustNotHaveTags)))
	{
		return 1;
	}
	
	return 0;
}

bool FGOAPTagContainerGoal::CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal,
	float& ContributionPoints) const
{
	const FGameplayTagContainer& Tags = GetTagContainer(Agent);

	bool bCanContribute = false;
	if (GetScriptStruct() == Goal.GetScriptStruct())
	{
		if (const FGOAPTagContainerGoal* GoalPtr = Goal.GetPtr<FGOAPTagContainerGoal>())
		{
			for (const FGameplayTag& Tag : MustHaveTags)
			{
				if (!Tags.HasTag(Tag) && GoalPtr->MustHaveTags.HasTag(Tag))
				{
					bCanContribute = true;
					ContributionPoints++;
				}
			}

			for (const FGameplayTag& Tag : MustNotHaveTags)
			{
				if (Tags.HasTag(Tag) && GoalPtr->MustNotHaveTags.HasTag(Tag))
				{
					bCanContribute = true;
					ContributionPoints++;
				}
			}
		}
	}
	
	return bCanContribute;
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
FString FGOAPTagContainerGoal::GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const
{
	return "Wants = " + MustHaveTags.ToString() + "Hates + " + MustNotHaveTags.ToString();
}
#endif

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
FString FGOAPSensorGoal::GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const
{
	FString RetVal = "SensorGoal - ";

	RetVal.Append(FGOAPTagContainerGoal::GetDebugString(Type, Agent));
	
	return RetVal;
}
#endif
