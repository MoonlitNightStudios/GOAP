// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "GOAPGoal.h"
#include "StructUtils/PropertyBag.h"
#include "GOAPAgentComponent.generated.h"


class UPropertyBag;
class UGOAPSubsystem;
class UGOAPAction;
class UGOAPAgentComponent;
class UGOAPAgentBehaviour;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGoalCompletedDelegate, UGOAPAgentComponent*, Agent, bool, bSuccess, FInstancedStruct&, Goal, const UObject*, Interrupter);


enum EGOAPActionNodeStatus
{
	Untested,
	Completed,
};


#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
struct FGOAPGoalPickerDebugDataRow
{
	const UScriptStruct* GoalType = nullptr;
	const FGOAPGoal* Goal = nullptr;
	bool bFailed = false;
	float TotalPriority = 0.0f;
};
#endif


struct FGOAPActionNode
{
	EGOAPActionNodeStatus Status = EGOAPActionNodeStatus::Untested;
	const UGOAPAction* Action = nullptr;
	FInstancedStruct Goal;
	FGOAPActionNode* ParentNode = nullptr;
	TArray<FGOAPActionNode> Children;
	TArray<const UGOAPAction*> FailedActions;
};


struct FGOAPPlan
{
	FGOAPPlan();
	
	bool GeneratePlan(class UGOAPAgentComponent* AgentComponent, const UGOAPSubsystem* SS, const FInstancedStruct& ForGoal);

	void OnActionCompleted(const UGOAPAction* Action, const FInstancedStruct& ForGoal);
	void OnActionFailed(const UGOAPAction* Action, const FInstancedStruct& ForGoal);

	void StopPlan(UGOAPAgentComponent* Agent);
	void UpdatePlan(float DeltaTime, UGOAPAgentComponent* Agent);
	void ExecutePlan(UGOAPAgentComponent* Agent);

	const UGOAPAction* GetCurrentAction() const {return CurrentAction ? CurrentAction->Action : nullptr;}
	const FInstancedStruct* GetCurrentActionGoal() const {return CurrentAction ? &CurrentAction->Goal : nullptr;}
	const FInstancedStruct* GetCurrentGoal() const {return Goal;}
	const FGOAPActionNode& GetRootNode() const {return RootAction;}
	
private:

	bool FindBestActionForGoal(UGOAPAgentComponent* Agent, const FInstancedStruct& Goal, const TArray<const UGOAPAction*>& AvailableActions, FGOAPActionNode& Node);
	bool HasActionFailedForGoal(const UGOAPAction* Action, FGOAPActionNode& Node) const;
	FGOAPActionNode& GetLeafNode(FGOAPActionNode& Root) const;
	
private:

	const UGOAPSubsystem* Subsystem = nullptr;
	UGOAPAgentComponent* OwningAgent = nullptr;
	const FInstancedStruct* Goal = nullptr;
	FGOAPActionNode* CurrentAction = nullptr;
	FGOAPActionNode RootAction;
};


UCLASS( ClassGroup=(GOAP), meta=(BlueprintSpawnableComponent), DisplayName = "GOAP Agent" )
class GOAP_API UGOAPAgentComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UGOAPSubsystem;

public:	
	// Sets default values for this component's properties
	UGOAPAgentComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	APawn* GetPawn() const;

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	AController* GetController() const;
	
	//Temp
	void MakePlan(const UGOAPSubsystem* SS);

	//Called by the Subsystem when the world state has changed
	void OnWorldStateChanged(const UGOAPSubsystem* SS);
	void OnAgentStateChanged();
	
	void StopCurrentPlan();
	void ExecutePlan();
	void OnPlanCompleted(const FInstancedStruct* CompletedGoal);
	void OnPlanFailed(const FInstancedStruct* FailedGoal);

	const FGOAPPlan& GetCurrentPlan() const {return CurrentPlan;}

	void OnActionCompleted(const UGOAPAction* Action, const FInstancedStruct& ForGoal);
	void OnActionFailed(const UGOAPAction* Action, const FInstancedStruct& ForGoal);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void SetInterruptGoal(const FInstancedStruct& Goal, const UObject* InterruptRequester);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void AddStateData(FGameplayTag StateTag, const FInstancedStruct& Effect, bool bIsPermanent = false);

	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void RemoveStateData(FGameplayTag StateTag);
	
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	bool HasStateData(FGameplayTag StateTag) const;
	
	const FInstancedStruct* GetInterruptGoal() const {return &InterruptGoal;}

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	const TArray<FGOAPGoalPickerDebugDataRow>& GetGoalPickerDebugData() const {return GoalPickerDebugData;}
#endif
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	const FInstancedStruct* GetBestGoal() const;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP", meta = (BaseStruct = "/Script/GOAP.GOAPGoal"))
	TArray<FInstancedStruct> Goals;

	UPROPERTY(EditAnywhere, Category = "GOAP")
	FInstancedPropertyBag KnowledgeBase;

	UPROPERTY(EditAnywhere, Instanced, Category = "GOAP")
	TArray<UGOAPAgentBehaviour*> AgentBehaviours;
	
	TMap<FGameplayTag, FInstancedStruct> TransientStateData;
	TMap<FGameplayTag, FInstancedStruct> PermanentStateData;

	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category = "GOAP")
	FGoalCompletedDelegate OnInterruptGoalFinished;
	
protected:
	
	FGOAPPlan CurrentPlan;
	FInstancedStruct InterruptGoal;
	TWeakObjectPtr<const UObject> Interrupter;
	TArray<const FGOAPGoal*> FailedGoals;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	mutable TArray<FGOAPGoalPickerDebugDataRow> GoalPickerDebugData;
#endif
	
};
