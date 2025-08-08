// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GOAPSensor.generated.h"


class UGOAPAgentComponent;

UCLASS(Abstract, Blueprintable, BlueprintType)
class GOAP_API UGOAPSensor : public UObject
{
	GENERATED_BODY()

public:

	virtual void Sense(UGOAPAgentComponent* Agent) const {}
	
};
