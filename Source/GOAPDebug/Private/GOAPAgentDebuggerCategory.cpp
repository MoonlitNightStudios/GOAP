// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "GOAPAgentDebuggerCategory.h"

#include "GOAPAction.h"
#include "GOAPAgentComponent.h"
#include "Engine/Font.h"

TSharedRef<FGameplayDebuggerCategory> FGOAPAgentDebuggerCategory::MakeInstance()
{
	return MakeShareable(new FGOAPAgentDebuggerCategory());
}

void FGOAPAgentDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	
}

void FGOAPAgentDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (AActor* SelectedActor = FindLocalDebugActor())
	{
		DrawOverheadInfo(SelectedActor, CanvasContext);

		if (const APawn* Pawn = Cast<APawn>(SelectedActor))
		{
			if (AController* Controller = Pawn->GetController())
			{
				if (UGOAPAgentComponent* Agent = Controller->GetComponentByClass<UGOAPAgentComponent>())
				{
					if (const FInstancedStruct* IntrGoal = Agent->GetInterruptGoal())
					{
						if (const FGOAPGoal* Goal = IntrGoal->GetPtr<FGOAPGoal>())
						{
							CanvasContext.Printf(TEXT("Interrupt goal: %s"), *Goal->GetDebugString(IntrGoal->GetScriptStruct(), Agent));
						}
					}
					if (const FInstancedStruct* IntrGoal = Agent->GetCurrentPlan().GetCurrentGoal())
					{
						if (const FGOAPGoal* Goal = IntrGoal->GetPtr<FGOAPGoal>())
						{
							CanvasContext.Printf(TEXT("Current goal: %s"), *Goal->GetDebugString(IntrGoal->GetScriptStruct(), Agent));
						}
					}
					CanvasContext.Printf(TEXT("Plan: \n%s"), *GetPlanString(Agent->GetCurrentPlan().GetRootNode()));

					CanvasContext.Printf(TEXT("\n____________________________________________________\n"));
					//Info on how we chose the goal
					CanvasContext.Printf(TEXT("GoalPicker data:\n"));
					for (const FGOAPGoalPickerDebugDataRow& Row : Agent->GetGoalPickerDebugData())
					{
						CanvasContext.Printf(TEXT("%s : %s%f/%f"),
							Row.Goal? *Row.Goal->GetDebugString(Row.GoalType, Agent) : *FString("Invalid Goal"),
							Row.bFailed? *FString("Failed ") : *FString(),
							Row.TotalPriority,
							Row.Goal? Row.Goal->GetWeight() : 0.f
						);
					}
				}
			}
		}
	}
}

void FGOAPAgentDebuggerCategory::DrawOverheadInfo(const AActor* DebugActor, const FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (const APawn* Pawn = Cast<APawn>(DebugActor))
	{
		if (AController* Controller = Pawn->GetController())
		{
			if (UGOAPAgentComponent* Agent = Controller->GetComponentByClass<UGOAPAgentComponent>())
			{
				const FVector OverheadLocation = DebugActor->GetActorLocation() + FVector(0, 0, DebugActor->GetSimpleCollisionHalfHeight());
				if (CanvasContext.IsLocationVisible(OverheadLocation))
				{
					FGameplayDebuggerCanvasContext OverheadContext(CanvasContext);
					OverheadContext.Font = GEngine->GetSmallFont();
					OverheadContext.FontRenderInfo.bEnableShadow = true;

					FString GoalName = "None", ActionGoalName = "None", ActionName = "None";
					if (const FInstancedStruct* Goal = Agent->GetCurrentPlan().GetCurrentGoal())
					{
						if (const FGOAPGoal* GoalPtr = Goal->GetPtr<FGOAPGoal>())
						{
							GoalName = GoalPtr->GetDebugString(Goal->GetScriptStruct(), Agent);
						}
					}
					if (const FInstancedStruct* Goal = Agent->GetCurrentPlan().GetCurrentActionGoal())
					{
						if (const FGOAPGoal* GoalPtr = Goal->GetPtr<FGOAPGoal>())
						{
							ActionGoalName = GoalPtr->GetDebugString(Goal->GetScriptStruct(), Agent);
						}
					}
					if (const UGOAPAction* Action = Agent->GetCurrentPlan().GetCurrentAction())
					{
						ActionName = Action->GetName();
					}
					
					const FVector2D ScreenLoc = OverheadContext.ProjectLocation(OverheadLocation);
					FString ActorDesc = FString::Printf(TEXT("{yellow}Main Goal: {white}%s \n {yellow}Current Goal: {white}%s \n {yellow}Action: {white}%s"),
						*GoalName,
						*ActionGoalName,
						*ActionName);

					float SizeX = 0.0f, SizeY = 0.0f;
					OverheadContext.MeasureString(ActorDesc, SizeX, SizeY);
					OverheadContext.PrintAt(static_cast<float>(ScreenLoc.X - (SizeX * 0.5f)), static_cast<float>(ScreenLoc.Y - (SizeY * 1.5)), ActorDesc);
				}
			}
		}
	}
}

FString FGOAPAgentDebuggerCategory::GetPlanString(const struct FGOAPActionNode& Node, int IndentationLevel) const
{
	FString RetVal;

	for (int i=0;i<IndentationLevel;i++)
	{
		RetVal += TEXT(" ");
	}
	
	if (Node.Goal.IsValid())
	{
		RetVal += Node.Goal.GetScriptStruct()->GetName();
	}
	else
	{
		RetVal += "None";
	}

	RetVal += " <- ";
	if (Node.Action)
	{
		RetVal += Node.Action->GetName();
	}
	else
	{
		RetVal += "None";
	}

	for (const FGOAPActionNode& Child : Node.Children)
	{
		RetVal += "\n";
		for (int i=0;i<IndentationLevel;i++)
		{
			RetVal += TEXT(" ");
		}
		RetVal.Append(GetPlanString(Child, IndentationLevel + 1));
	}
	
	return RetVal;
}
