// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "GOAPAgentComponent.h"
#include "GOAPGoal.h"
#include "GOAPSensorGoal.generated.h"

USTRUCT(BlueprintType)
struct GOAP_API FGOAPTagContainerGoal : public FGOAPGoal
{
	GENERATED_BODY()
	
	virtual float GetPriority(const UGOAPAgentComponent* Agent) const override;
	virtual bool CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal, float& ContributionPoints) const override;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	virtual FString GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const override;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Goal")
	FGameplayTagContainer MustHaveTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Goal")
	FGameplayTagContainer MustNotHaveTags;
	
protected:

	virtual FGameplayTagContainer GetTagContainer(const UGOAPAgentComponent* Agent) const {return FGameplayTagContainer();}
	virtual UScriptStruct* GetScriptStruct() const {return StaticStruct();}
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPSensorGoal : public FGOAPTagContainerGoal
{
	GENERATED_BODY()

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	virtual FString GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const override;
#endif
	
	virtual UScriptStruct* GetScriptStruct() const override {return StaticStruct();}
};
