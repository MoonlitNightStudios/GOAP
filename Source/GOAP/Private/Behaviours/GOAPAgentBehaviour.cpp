// Copyright Moonlit Night Studios Ltd. All Rights Reserved.


#include "Behaviours/GOAPAgentBehaviour.h"

void UGOAPAgentBehaviour::ActivateBehaviour(UGOAPAgentComponent* OwningAgent)
{
	Agent = OwningAgent;
	bIsActive = true;
	OnBehaviourActivated();
	K2_OnBehaviourActivated();
}

void UGOAPAgentBehaviour::DeactivateBehaviour()
{
	bIsActive = false;
	OnBehaviourDeactivated();
	K2_OnBehaviourDeactivated();
}

void UGOAPAgentBehaviour::UpdateBehaviour(float DeltaTime)
{
	if (bIsActive)
	{
		OnBehaviourUpdated(DeltaTime);
		K2_OnBehaviourUpdated(DeltaTime);
	}
}
