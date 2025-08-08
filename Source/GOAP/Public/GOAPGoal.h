// Copyright Moonlit Night Studios Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/NoExportTypes.h"
#include "GOAPGoal.generated.h"


class UGOAPAgentComponent;

USTRUCT(BlueprintType)
struct GOAP_API FGOAPContext
{
	GENERATED_BODY()

	virtual ~FGOAPContext() {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GOAP", meta = (Categories = "GOAP"))
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GOAP", meta = (BaseStruct = "/Script/GOAP.GOAPValue"))
	FInstancedStruct Value;
	
};


USTRUCT(meta = (HiddenByDefault), NotBlueprintType)
struct GOAP_API FGOAPValue
{
	GENERATED_BODY()

	virtual ~FGOAPValue() {}
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPBoolValue : public FGOAPValue
{
	GENERATED_BODY()

	FGOAPBoolValue() = default;
	FGOAPBoolValue(bool InValue) : Value(InValue) {}
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	bool Value = false;
	
};


USTRUCT(NotBlueprintType)
struct GOAP_API FGOAPFloatValueBase : public FGOAPValue
{
	GENERATED_BODY()

	FGOAPFloatValueBase() = default;

	virtual float GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const {return 0.f;}
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPFloatValue : public FGOAPFloatValueBase
{
	GENERATED_BODY()

	FGOAPFloatValue() = default;
	FGOAPFloatValue(float InValue) : Value(InValue) {}

	virtual float GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const {return Value;}
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	float Value = 0.f;
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPVector2DValue : public FGOAPValue
{
	GENERATED_BODY()

	FGOAPVector2DValue() = default;
	FGOAPVector2DValue(float x, float y) : Value(FVector2D(x,y)) {}
	FGOAPVector2DValue(float InValue) : Value(FVector2D(InValue)) {}
	FGOAPVector2DValue(FVector2D InValue) : Value(InValue) {}

	FVector2D GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const {return Value;}
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	FVector2D Value = FVector2D::ZeroVector;
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPVectorValue : public FGOAPValue
{
	GENERATED_BODY()

	FGOAPVectorValue() = default;
	FGOAPVectorValue(float InValue) : Value(FVector(InValue)) {}
	FGOAPVectorValue(float X, float Y, float Z) : Value(FVector(X,Y,Z)) {}
	FGOAPVectorValue(FVector InValue) : Value(InValue) {}

	FVector GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const {return Value;}

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	FVector Value = FVector::ZeroVector;
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPStringValue : public FGOAPValue
{
	GENERATED_BODY()

	FGOAPStringValue() = default;
	FGOAPStringValue(FString& InValue) : Value(InValue) {}

	FString GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const {return Value;}

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	FString Value;
	
};


USTRUCT(NotBlueprintType, meta = (HiddenByDefault))
struct GOAP_API FGOAPActorValueBase : public FGOAPValue
{
	GENERATED_BODY()

	FGOAPActorValueBase() = default;

	virtual AActor* GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const {return nullptr;}
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPActorValue : public FGOAPActorValueBase
{
	GENERATED_BODY()

	FGOAPActorValue() = default;
	FGOAPActorValue(const TSoftObjectPtr<AActor>& InValue) : Value(InValue) {}

	virtual AActor* GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const override {return Value.LoadSynchronous();}

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GOAP")
	TSoftObjectPtr<AActor> Value;
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPSelfAgentPawnValue : public FGOAPActorValueBase
{
	GENERATED_BODY()

	FGOAPSelfAgentPawnValue() = default;

	virtual AActor* GetValue(const UGOAPAgentComponent* Agent, const TArray<FGOAPContext>& Contexts) const override;
	
};


USTRUCT(BlueprintType, meta = (HiddenByDefault))
struct GOAP_API FGOAPGoal
{
	GENERATED_BODY()
	
	FGOAPGoal& SetValue(const FInstancedStruct& Value);

	template<typename T>
	FGOAPGoal& SetValue(const T& Value)
	{
		TargetStateValue = FInstancedStruct::Make(Value);
		return *this;
	}

	void AddContext(const FGOAPContext& Context);

	virtual ~FGOAPGoal() = default;

	virtual UScriptStruct* GetType() const {return StaticStruct();}

	const TArray<FGOAPContext>& GetContexts() const {return Contexts;}
	const FInstancedStruct& GetTargetStateValue() const {return TargetStateValue;}
	float GetWeight() const {return Weight;}

	virtual float GetPriority(const UGOAPAgentComponent* Agent) const {return 0.0f;}

	//Called on effects to see if it can contribute to goals
	virtual bool CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal, float& ContributionPoints) const {return false;}

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	virtual FString GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const
	{return FString::Printf(TEXT("%s: Debug not implemented"), *(Type ? Type->GetName() : "InvalidType"));}
#endif	
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GOAP")
	TArray<FGOAPContext> Contexts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GOAP", meta = (BaseStruct = "/Script/GOAP.GOAPValue"))
	FInstancedStruct TargetStateValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float Weight = 1.f;
	
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPLocationGoal : public FGOAPGoal
{
	GENERATED_BODY()

	virtual float GetPriority(const UGOAPAgentComponent* Agent) const override;
	virtual bool CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal, float& ContributionPoints) const override;
	
	FVector GetTargetLocation(const UGOAPAgentComponent* Agent) const;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	virtual FString GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const override;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float AcceptanceRadius = 10.f;
};


USTRUCT(BlueprintType)
struct GOAP_API FGOAPFindPathGoal : public FGOAPGoal
{
	GENERATED_BODY()

	virtual float GetPriority(const UGOAPAgentComponent* Agent) const override;
	virtual bool CanContributeToGoal(const UGOAPAgentComponent* Agent, const FInstancedStruct& Goal, float& ContributionPoints) const override;
	
	FVector GetTargetLocation(const UGOAPAgentComponent* Agent) const;

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	virtual FString GetDebugString(const UScriptStruct* Type, const UGOAPAgentComponent* Agent) const override;
#endif
	
};
