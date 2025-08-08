// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "GOAPSubsystem.h"

#include "GOAPAction.h"
#include "GOAPAgentComponent.h"
#include "GOAPSensor.h"

#if WITH_EDITOR
#include "Kismet2/KismetEditorUtilities.h"
#endif

void UGOAPSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if(It->IsChildOf(UGOAPAction::StaticClass())
		&& It->IsNative()
		&& !It->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)
#if WITH_EDITOR
		&& !FKismetEditorUtilities::IsClassABlueprintSkeleton(*It)
#endif
		)
		{
			UGOAPAction* Action = Cast<UGOAPAction>(It->GetDefaultObject());
			Action->OpenKnowledgeBank(this);
			GlobalActions.Add(Action);
		}
	}

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if(It->IsChildOf(UGOAPSensor::StaticClass())
		&& It->IsNative()
		&& !It->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)
#if WITH_EDITOR
		&& !FKismetEditorUtilities::IsClassABlueprintSkeleton(*It)
#endif
		)
		{
			GlobalSensors.Add(Cast<UGOAPSensor>(It->GetDefaultObject()));
		}
	}
	
	//GetWorld()->GetTimerManager().SetTimer(GeneratePlansTimer, this, &UGOAPSubsystem::GeneratePlans, PlanGenerationRate, true, PlanGenerationRate);
}

void UGOAPSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ActiveAgents.Empty();
	InactiveAgents.Empty();

	for (const UGOAPAction* Action : GlobalActions)
	{
		Action->CloseKnowledgeBank(this);
	}
	GlobalActions.Empty();

	GetWorld()->GetTimerManager().ClearTimer(GeneratePlansTimer);
}

void UGOAPSubsystem::RegisterAgent(UGOAPAgentComponent* Agent)
{
	ActiveAgents.AddUnique(Agent);
}

void UGOAPSubsystem::UnregisterAgent(UGOAPAgentComponent* Agent)
{
	ActiveAgents.Remove(Agent);
	InactiveAgents.Remove(Agent);
}

void UGOAPSubsystem::ActivateAgent(UGOAPAgentComponent* Agent)
{
	ActiveAgents.AddUnique(Agent);
	InactiveAgents.Remove(Agent);
	Agent->Activate(true);
}

void UGOAPSubsystem::DeactivateAgent(UGOAPAgentComponent* Agent)
{
	InactiveAgents.AddUnique(Agent);
	ActiveAgents.Remove(Agent);
	Agent->Deactivate();
}

void UGOAPSubsystem::GeneratePlans()
{
	for (UGOAPAgentComponent* Agent : ActiveAgents)
	{
		Agent->MakePlan(this);
	}
}
