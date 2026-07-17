#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NG2AbilitySystemComponent.generated.h"

/**
 * Note: Debug in-editor with the showstats AbilitySystem command
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NG2_API UNG2AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AddAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);
	
	void AbilityInputTagTriggered(const FGameplayTag& InputTag);
};
