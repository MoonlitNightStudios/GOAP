// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "GameplayDebuggerCategory.h"


class FGOAPAgentDebuggerCategory : public FGameplayDebuggerCategory
{
public:
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

protected:

	void DrawOverheadInfo(const AActor* DebugActor, const FGameplayDebuggerCanvasContext& CanvasContext);
	FString GetPlanString(const struct FGOAPActionNode& Node, int IndentationLevel = 0) const;
	
};
