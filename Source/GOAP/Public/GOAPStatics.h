// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GOAPGoal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "GOAPStatics.generated.h"


UCLASS()
class GOAP_API UGOAPStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	template<typename T>
	static TArray<const T*> GetContextsOfType(const TArray<FGOAPContext>& Contexts, FGameplayTagContainer RequiredTags = FGameplayTagContainer());
	
};

template <typename T>
TArray<const T*> UGOAPStatics::GetContextsOfType(const TArray<FGOAPContext>& Contexts,
	FGameplayTagContainer RequiredTags)
{
	TArray<const T*> Result;
	
	for (const FGOAPContext& ContextInstance : Contexts)
	{
		if (ContextInstance.Tags.HasAll((RequiredTags)))
		{
			if (const T* Context = ContextInstance.Value.GetPtr<T>())
			{
				Result.Add(Context);
			}
		}
	}

	return Result;
}
