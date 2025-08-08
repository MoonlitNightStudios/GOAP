// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GOAPAgentBehaviour.generated.h"


class UGOAPAgentComponent;


UCLASS(Abstract, Blueprintable, EditInlineNew, BlueprintType)
class GOAP_API UGOAPAgentBehaviour : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void ActivateBehaviour(UGOAPAgentComponent* OwningAgent);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void DeactivateBehaviour();
	
	void UpdateBehaviour(float DeltaTime);

protected:
	
	virtual void OnBehaviourActivated(){}
	virtual void OnBehaviourDeactivated(){}
	virtual void OnBehaviourUpdated(float DeltaTime){}
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GOAP", DisplayName = "OnBehaviourActivated")
	void K2_OnBehaviourActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "GOAP", DisplayName = "OnBehaviourActivated")
	void K2_OnBehaviourDeactivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "GOAP", DisplayName = "OnBehaviourActivated")
	void K2_OnBehaviourUpdated(float DeltaTime);

protected:

	UPROPERTY(BlueprintReadOnly, Category = "GOAP")
	UGOAPAgentComponent* Agent = nullptr;
	
	bool bIsActive = false;
	
};
