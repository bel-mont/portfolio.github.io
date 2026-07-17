#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "NG2InputConfig.generated.h"

enum class ETriggerEvent : uint8;

USTRUCT(BlueprintType)
struct FNG2InputActionMapping
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<ETriggerEvent> TriggerEvents;
};

UCLASS()
class NG2_API UNG2InputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Allows multiple tags to map to a single action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FNG2InputActionMapping> InputActions;
};
