// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITypes.h"
#include "GOAPAction.h"
#include "GOAPAction_MoveToLocation.generated.h"


struct FMobileAgentData
{
	FAIRequestID RequestID;
	TArray<FInstancedStruct> FailedToReachLocations;
};


UCLASS()
class GOAP_API UGOAPAction_MoveToLocation : public UGOAPAction
{
	GENERATED_BODY()
public:

	virtual TArray<FInstancedStruct> GetRequirements(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const override;
	virtual TArray<FInstancedStruct> GetEffects(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const override;

	virtual void CloseKnowledgeBank(UGOAPSubsystem* SS) const override;
	virtual void StartAction(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const override;
	virtual void StopAction(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const override;

protected:

	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	mutable TMap<UGOAPAgentComponent*, FMobileAgentData> MobileAgents;
	
};
