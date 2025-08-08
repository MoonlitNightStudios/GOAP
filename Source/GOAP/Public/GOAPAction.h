// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GOAPGoal.h"
#include "StructUtils/StructView.h"
#include "GOAPAction.generated.h"


class UGOAPSubsystem;
class UGOAPAgentComponent;


UCLASS(Abstract)
class GOAP_API UGOAPAction : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GOAP|Action")
	virtual TArray<FInstancedStruct> GetRequirements(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GOAP|Action")
	virtual TArray<FInstancedStruct> GetEffects(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const;

	virtual void OpenKnowledgeBank(UGOAPSubsystem* SS) const {}
	virtual void CloseKnowledgeBank(UGOAPSubsystem* SS) const {}
	virtual void StartAction(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const {}
	virtual void UpdateAction(float DeltaTime, UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const {}
	virtual void StopAction(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal) const {}
	
};
