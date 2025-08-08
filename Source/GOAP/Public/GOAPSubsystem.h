// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GOAPSubsystem.generated.h"


class UGOAPAgentComponent;
class UGOAPAction;
class UGOAPSensor;


UCLASS(config = GOAP)
class GOAP_API UGOAPSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void RegisterAgent(UGOAPAgentComponent* Agent);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void UnregisterAgent(UGOAPAgentComponent* Agent);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void ActivateAgent(UGOAPAgentComponent* Agent);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void DeactivateAgent(UGOAPAgentComponent* Agent);
	
	void GeneratePlans();

	const TArray<const UGOAPAction*>& GetGlobalActions() const {return GlobalActions;}
	const TArray<const UGOAPSensor*>& GetGlobalSensors() const {return GlobalSensors;}

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP", Config)
	float PlanGenerationRate = 1.f;

	UPROPERTY()
	TArray<const UGOAPAction*> GlobalActions;

	UPROPERTY()
	TArray<const UGOAPSensor*> GlobalSensors;

	UPROPERTY()
	TArray<UGOAPAgentComponent*> ActiveAgents;

	UPROPERTY()
	TArray<UGOAPAgentComponent*> InactiveAgents;

	FTimerHandle GeneratePlansTimer;
	
};
